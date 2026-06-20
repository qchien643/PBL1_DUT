#pragma once

#include <string>
#include <utility>
#include <vector>

namespace app::server {

std::string jsonEscape(const std::string &value);
std::string jsonString(const std::string &value);
std::string jsonSuccess(const std::string &dataJson);
std::string jsonError(const std::string &code, const std::string &message);
std::string jsonError(const std::string &code, const std::string &message, const std::string &requiredAction, const std::string &contextJson, const std::string &correlationId);
std::string jsonBool(bool value);
std::string extractJsonString(const std::string &body, const std::string &key, const std::string &fallback = "");
int extractJsonInt(const std::string &body, const std::string &key, int fallback = 0);
std::vector<std::pair<int, int>> extractCartLines(const std::string &body);

}
