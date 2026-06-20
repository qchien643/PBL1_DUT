#pragma once

#include "../infrastructure/file_database.hpp"

#include <string>

namespace app::console {

void customerLoop(FileDatabase &database, const std::string &tableCode);

}
