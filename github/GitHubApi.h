#pragma once
#include "pch.h"
#include "network/HttpClient.h"
#include "github/RepoParser.h"

class GitHubApi {
public:
    GitHubApi();

    std::shared_ptr<FileNode> getRepoTree(const std::wstring& owner, const std::wstring& name,
                                          const std::wstring& branch, std::wstring& defaultBranch);
    std::wstring getDefaultBranch(const std::wstring& owner, const std::wstring& name);
    std::vector<std::wstring> getBranches(const std::wstring& owner, const std::wstring& name);
    bool testConnection();

    void cancel() { m_http.cancel(); }
    bool isCancelled() const { return m_http.isCancelled(); }

private:
    HttpClient m_http;

    std::shared_ptr<FileNode> buildTree(const std::string& json, const std::wstring& branch);
    std::wstring extractString(const std::string& json, const std::string& key);
    std::vector<std::string> splitJsonArray(const std::string& json);
};
