#pragma once
#pragma once
#include <string>
#include <vector>

// simple error info
struct PlasError {
    int line;
    std::string message;
};

bool CheckPLASSyntax(const std::vector<std::string>& lines,
    std::vector<PlasError>& outErrors);
