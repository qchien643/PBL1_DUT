#pragma once

#include "../infrastructure/file_database.hpp"

#include <string>

namespace app::console {

void kitchenLoop(FileDatabase &database, std::string station);

}
