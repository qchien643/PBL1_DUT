#pragma once

#include <string>
#include <vector>

namespace app {

std::string sanitize(std::string value);
std::string restore(const std::string &value);
std::vector<std::string> split(const std::string &line, char delimiter);
std::string joinIds(const std::vector<int> &ids);
std::vector<int> parseIds(const std::string &value);
std::string nowStamp();
std::string toLower(std::string value);

}
