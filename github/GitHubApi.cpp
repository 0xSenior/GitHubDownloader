#include "pch.h"
#include "github/GitHubApi.h"
#include "thirdparty/json/json.hpp"

using json = nlohmann::json;

GitHubApi::GitHubApi() {}

std::wstring GitHubApi::getDefaultBranch(const std::wstring& owner, const std::wstring& name) {
    std::wstring url = L"https://api.github.com/repos/" + owner + L"/" + name;
    auto resp = m_http.get(url, L"Accept: application/vnd.github+json\r\nUser-Agent: GitHubDownloader/2.0");
    if (!resp.success) return L"main";

    try {
        auto j = json::parse(resp.body);
        if (j.contains("default_branch")) return Str::toWide(j["default_branch"].get<std::string>());
    } catch (...) {}
    return L"main";
}

std::vector<std::wstring> GitHubApi::getBranches(const std::wstring& owner, const std::wstring& name) {
    std::vector<std::wstring> branches;
    std::wstring url = L"https://api.github.com/repos/" + owner + L"/" + name + L"/branches?per_page=100";
    auto resp = m_http.get(url, L"Accept: application/vnd.github+json\r\nUser-Agent: GitHubDownloader/2.0");
    if (!resp.success) return branches;

    try {
        auto j = json::parse(resp.body);
        if (j.is_array()) {
            for (auto& b : j) {
                if (b.contains("name")) branches.push_back(Str::toWide(b["name"].get<std::string>()));
            }
        }
    } catch (...) {}
    return branches;
}

std::shared_ptr<FileNode> GitHubApi::getRepoTree(const std::wstring& owner, const std::wstring& name,
    const std::wstring& branch, std::wstring& defaultBranch) {
    
    if (defaultBranch.empty()) {
        defaultBranch = getDefaultBranch(owner, name);
    }

    std::wstring effectiveBranch = branch.empty() ? defaultBranch : branch;
    std::wstring url = L"https://api.github.com/repos/" + owner + L"/" + name +
                       L"/git/trees/" + effectiveBranch + L"?recursive=1";

    auto resp = m_http.get(url, L"Accept: application/vnd.github+json\r\nUser-Agent: GitHubDownloader/2.0");
    if (!resp.success) return nullptr;

    return buildTree(resp.body, effectiveBranch);
}

std::shared_ptr<FileNode> GitHubApi::buildTree(const std::string& jsonStr, const std::wstring& branch) {
    auto root = std::make_shared<FileNode>();
    root->name = branch;
    root->isDir = true;
    root->path = L"";

    try {
        auto j = json::parse(jsonStr);
        if (!j.contains("tree") || !j["tree"].is_array()) return root;

        std::vector<std::shared_ptr<FileNode>> allNodes;

        for (auto& item : j["tree"]) {
            auto node = std::make_shared<FileNode>();
            std::string pathStr = item.value("path", "");
            std::string typeStr = item.value("type", "");

            node->path = Str::toWide(pathStr);
            node->isDir = (typeStr == "tree");

            size_t lastSlash = pathStr.find_last_of('/');
            node->name = (lastSlash != std::string::npos) ?
                Str::toWide(pathStr.substr(lastSlash + 1)) : Str::toWide(pathStr);

            if (item.contains("size")) node->size = item["size"].get<uint64_t>();

            allNodes.push_back(node);
        }

        std::unordered_map<std::wstring, std::shared_ptr<FileNode>> dirMap;
        dirMap[L""] = root;

        for (auto& node : allNodes) {
            std::wstring dirPath;
            size_t lastSlash = node->path.find_last_of(L'/');
            if (lastSlash != std::wstring::npos) {
                dirPath = node->path.substr(0, lastSlash);
            }

            if (dirMap.count(dirPath)) {
                dirMap[dirPath]->children.push_back(node);
            }

            if (node->isDir) {
                dirMap[node->path] = node;
            }
        }

        std::function<void(std::shared_ptr<FileNode>)> sortTree = [&](std::shared_ptr<FileNode> n) {
            if (n->children.empty()) return;
            std::sort(n->children.begin(), n->children.end(),
                [](const auto& a, const auto& b) {
                    if (a->isDir != b->isDir) return a->isDir > b->isDir;
                    return Str::toLower(a->name) < Str::toLower(b->name);
                });
            for (auto& c : n->children) sortTree(c);
        };
        sortTree(root);

    } catch (...) {}
    return root;
}

bool GitHubApi::testConnection() {
    auto resp = m_http.get(L"https://api.github.com/", L"User-Agent: GitHubDownloader/2.0");
    return resp.success;
}
