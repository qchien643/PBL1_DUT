#pragma once

#include "../../domain/models.hpp"
#include "../../infrastructure/file_database.hpp"

#include <string>
#include <vector>

namespace app::menu_inventory {

std::vector<MenuItemRecord> visibleMenuItems(const FileDatabase &database, bool includeHidden);
OperationResult setAvailability(FileDatabase &database, int menuItemId, const std::string &availabilityStatus, const std::string &actor);

}
