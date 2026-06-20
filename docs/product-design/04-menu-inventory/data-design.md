# Data Design - Menu Inventory

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `menus` | Menu chi nhánh | `id`, `branchId`, `name`, `status` |
| `menu_categories` | Nhóm món | `id`, `menuId`, `name`, `displayOrder` |
| `menu_items` | Món ăn | `id`, `categoryId`, `name`, `description`, `basePrice`, `catalogStatus` |
| `menu_item_variants` | Size/variant | `id`, `itemId`, `name`, `priceDelta` |
| `modifier_groups` | Nhóm tùy chọn | `id`, `itemId`, `name`, `minSelect`, `maxSelect` |
| `modifier_options` | Option cụ thể | `id`, `groupId`, `name`, `priceDelta` |
| `item_availability` | Trạng thái bán | `branchId`, `itemId`, `availabilityStatus`, `isVisible`, `isOrderable`, `reason` |
| `availability_history` | Lịch sử đổi trạng thái | `itemId`, `fromStatus`, `toStatus`, `actorId`, `reason` |

## Indexes

| Table | Index |
| --- | --- |
| `menu_items` | `categoryId`, `catalogStatus` |
| `item_availability` | unique `branchId`, `itemId` |
| `menu_categories` | `menuId`, `displayOrder` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `menu_items.catalogStatus` | `MenuCatalogStatusPolicy` | Không xóa item đã từng bán |
| `item_availability.status` | `MenuAvailabilityPolicy` | Branch-specific sold-out |
| `menu_item_modifiers.status` | `ModifierAvailabilityPolicy` | Validate lại khi submit |
| `priceVersion` / `priceSnapshot` | `PriceSnapshotPolicy` | Order/bill dùng snapshot để không sai tiền |
