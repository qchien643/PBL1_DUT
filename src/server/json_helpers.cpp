#include "json_helpers.hpp"

#include <cctype>
#include <regex>

namespace app::server {

std::string jsonEscape(const std::string &value) {
    std::string escaped;
    for (char character : value) {
        switch (character) {
            case '"':
                escaped += "\\\"";
                break;
            case '\\':
                escaped += "\\\\";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += character;
                break;
        }
    }
    return escaped;
}

std::string jsonString(const std::string &value) {
    return "\"" + jsonEscape(value) + "\"";
}

std::string jsonSuccess(const std::string &dataJson) {
    return "{\"ok\":true,\"success\":true,\"data\":" + dataJson + "}";
}

std::string jsonError(const std::string &code, const std::string &message) {
    return jsonError(code, message, "", "{}", "");
}

std::string jsonError(const std::string &code, const std::string &message, const std::string &requiredAction, const std::string &contextJson, const std::string &correlationId) {
    return "{\"ok\":false,\"success\":false,\"error\":{\"code\":" + jsonString(code) +
           ",\"message\":" + jsonString(message) +
           ",\"requiredAction\":" + jsonString(requiredAction) +
           ",\"context\":" + (contextJson.empty() ? "{}" : contextJson) +
           "},\"correlationId\":" + jsonString(correlationId) + "}";
}

std::string jsonBool(bool value) {
    return value ? "true" : "false";
}

std::string extractJsonString(const std::string &body, const std::string &key, const std::string &fallback) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(body, match, pattern)) {
        return match[1].str();
    }
    return fallback;
}

int extractJsonInt(const std::string &body, const std::string &key, int fallback) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?\\d+)");
    std::smatch match;
    if (std::regex_search(body, match, pattern)) {
        try {
            return std::stoi(match[1].str());
        } catch (...) {
            return fallback;
        }
    }
    return fallback;
}

std::vector<std::pair<int, int>> extractCartLines(const std::string &body) {
    std::vector<std::pair<int, int>> lines;
    const std::regex pattern("\\{[^\\}]*\"menuItemId\"\\s*:\\s*(\\d+)[^\\}]*\"quantity\"\\s*:\\s*(\\d+)[^\\}]*\\}");
    auto begin = std::sregex_iterator(body.begin(), body.end(), pattern);
    auto end = std::sregex_iterator();
    for (auto iterator = begin; iterator != end; ++iterator) {
        lines.push_back({std::stoi((*iterator)[1].str()), std::stoi((*iterator)[2].str())});
    }
    return lines;
}

}
