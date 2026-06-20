#pragma once

#include <string>

namespace app::console {

int readInt(const std::string &prompt);
std::string readText(const std::string &prompt);
void printHelp();

}
