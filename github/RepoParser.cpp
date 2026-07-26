#include "pch.h"
#include "github/RepoParser.h"

ParsedRepo RepoParser::parse(const std::wstring& url) {
    ParsedRepo result;
    result.rawUrl = Str::trim(url);

    std::wstring work = result.rawUrl;

    if (Str::startsWith(work, L"https://github.com/")) {
        work = work.substr(19);
    } else if (Str::startsWith(work, L"http://github.com/")) {
        work = work.substr(18);
    } else if (Str::startsWith(work, L"github.com/")) {
        work = work.substr(11);
    } else {
        result.error = L"Invalid URL: must be a GitHub URL";
        return result;
    }

    if (work.back() == L'/') work.pop_back();

    size_t firstSlash = work.find(L'/');
    if (firstSlash == std::wstring::npos) {
        result.error = L"Missing repository name";
        return result;
    }

    result.owner = work.substr(0, firstSlash);
    work = work.substr(firstSlash + 1);

    size_t secondSlash = work.find(L'/');
    if (secondSlash != std::wstring::npos) {
        result.name = work.substr(0, secondSlash);
        std::wstring rest = work.substr(secondSlash + 1);
        if (Str::startsWith(rest, L"tree/") || Str::startsWith(rest, L"blob/")) {
            rest = rest.substr(5);
            size_t nextSlash = rest.find(L'/');
            result.branch = (nextSlash != std::wstring::npos) ? rest.substr(0, nextSlash) : rest;
        } else if (!rest.empty()) {
            result.branch = rest;
        }
    } else {
        result.name = work;
    }

    if (result.owner.empty() || result.name.empty()) {
        result.error = L"Could not parse owner and repository name";
        return result;
    }

    result.valid = true;
    if (result.branch.empty()) result.branch = L"main";
    return result;
}

bool RepoParser::isValidGitHubUrl(const std::wstring& url) {
    return parse(url).valid;
}
