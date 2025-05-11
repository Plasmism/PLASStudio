#include "plas_syntax_check.h"
#include <unordered_set>
#include <stack>
#include <algorithm>
#include <cctype>

static std::string Trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string GetFirstWord(const std::string& s) {
    auto pos = s.find_first_of(" \t");
    return pos == std::string::npos ? s : s.substr(0, pos);
}

bool CheckPLASSyntax(const std::vector<std::string>& lines,
    std::vector<PlasError>& outErrors) {
    outErrors.clear();
    std::stack<int> blockStack;
    const std::unordered_set<std::string> reserved = {
        "include","create","main","function","fn","call","ask",
        "read","print","output","repeat","loop","if","else",
        "then","end","list","add","remove","get","into","delete",
        "memory","alloc","free","declare","let","return","write",
        "wait","delay","label","goto","square","max","min","abs","mod"
    };

    for (size_t i = 0; i < lines.size(); ++i) {
        auto trimmed = Trim(lines[i]);
        if (trimmed.empty() || trimmed.rfind("//", 0) == 0)
            continue;
        // skip braces
        if (trimmed.find('{') != std::string::npos || trimmed == "}")
            continue;

        // handle end keywords
        if (trimmed == "end program") {
            continue; 
        }
        if (trimmed == "end") {
            if (blockStack.empty()) {
                outErrors.push_back({ int(i + 1), "Unexpected 'end'" });
            }
            else {
                blockStack.pop();
            }
            continue;
        }


        if ((trimmed.rfind("if ", 0) == 0) || (trimmed.rfind("if(", 0) == 0) ||
            (trimmed.rfind("repeat ", 0) == 0) || (trimmed.rfind("loop ", 0) == 0)) {
            blockStack.push(int(i + 1));
            continue;
        }

        // treat any line with '=' as assignment
        if (trimmed.find('=') != std::string::npos) {
            continue;
        }

        auto first = GetFirstWord(trimmed);
        if (reserved.count(first) > 0) {
            continue;
        }

        outErrors.push_back({ int(i + 1), "Unknown keyword: '" + first + "'" });
    }

    // check for any unclosed blocks
    while (!blockStack.empty()) {
        outErrors.push_back({ blockStack.top(), "Missing 'end'" });
        blockStack.pop();
    }

    return outErrors.empty();
}
