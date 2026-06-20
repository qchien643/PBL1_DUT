# Commands And Interfaces - Menu Inventory

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `ViewMenu` | `MenuService.getCustomerMenu` | Customer | `MenuAvailabilityPolicy`, `InventoryPolicy` |
| `CreateMenuItem` | `MenuService.createItem` | Manager | `PermissionPolicy` |
| `UpdateMenuItem` | `MenuService.updateItem` | Manager | `PermissionPolicy`, `AuditPolicy` |
| `SetItemSoldOut` | `InventoryService.setSoldOut` | Manager/Cashier | `InventoryPolicy`, `AuditPolicy` |
| `SetItemAvailable` | `InventoryService.setAvailable` | Manager/Cashier | `InventoryPolicy`, `AuditPolicy` |
| `GetMenuItemDetail` | `MenuService.getItemDetail` | Customer/Staff | `MenuAvailabilityPolicy` |

## Output DTO

| Field | Meaning |
| --- | --- |
| `itemId` | Menu item id |
| `displayName` | Current item name |
| `price` | Current display price |
| `orderable` | From availability policy |
| `soldOutReason` | Reason if not orderable |

## Policy-aware interfaces

```text
getMenuForTable
→ MenuCatalogStatusPolicy
→ MenuAvailabilityPolicy
→ RecommendationFilterPolicy if needed
→ return display model with orderable=false when denied
```

Submit/accept vẫn phải re-check policy; UI menu chỉ là snapshot hiển thị.
