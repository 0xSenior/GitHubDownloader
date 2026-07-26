#pragma once
#include "pch.h"

struct ParsedRepo {
    bool valid = false;
    std::wstring owner;
    std::wstring name;
    std::wstring branch;
    std::wstring rawUrl;
    std::wstring error;
};

class RepoParser {
public:
    static ParsedRepo parse(const std::wstring& url);
    static bool isValidGitHubUrl(const std::wstring& url);
};
