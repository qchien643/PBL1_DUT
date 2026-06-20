#include "menu_inventory_service.hpp"

#include "../../policies/business_policies.hpp"

namespace app::menu_inventory {

std::vector<MenuItemRecord> visibleMenuItems(const FileDatabase &database, bool includeHidden) {
    std::vector<MenuItemRecord> items;
    for (const MenuItemRecord &item : database.menuItems) {
        if (includeHidden || item.catalogStatus == "ACTIVE") {
            items.push_back(item);
        }
    }
    return items;
}

OperationResult setAvailability(FileDatabase &database, int menuItemId, const std::string &availabilityStatus, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "menu.availability");
    if (!permission.ok) {
        return permission;
    }

    MenuItemRecord *item = database.findMenuItemById(menuItemId);
    if (item == nullptr) {
        return OperationResult::failure("Menu item not found.");
    }
    if (availabilityStatus != "AVAILABLE" && availabilityStatus != "SOLD_OUT") {
        return OperationResult::failure("Invalid availability status.");
    }

    item->availabilityStatus = availabilityStatus;
    database.addAudit(actor, "Changed availability of item #" + std::to_string(menuItemId) + " to " + availabilityStatus);
    database.save();
    return OperationResult::success("Menu availability updated.");
}

}
