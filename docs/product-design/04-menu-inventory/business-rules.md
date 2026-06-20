# Business Rules - Menu Inventory

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| MENU_001 | Chỉ món `active` được hiển thị cho customer | `MenuAvailabilityPolicy` | Required |
| MENU_002 | Món `sold_out` không được order | `InventoryPolicy` | Required |
| MENU_003 | Món `sold_out` không được recommend | `InventoryPolicy` | Required |
| MENU_004 | `catalogStatus` và `availabilityStatus` phải tách riêng | `MenuAvailabilityPolicy` | Required |
| MENU_005 | Manager mới được sửa menu/giá | `PermissionPolicy` | Required |
| MENU_006 | Tạo món mới phải tạo availability mặc định | `InventoryPolicy` | Required |
| MENU_007 | Submit order phải snapshot giá/tên/modifier | `PricingPolicy` | Required |

## Status values

| Field | Values |
| --- | --- |
| `catalogStatus` | `draft`, `active`, `hidden`, `archived` |
| `availabilityStatus` | `available`, `sold_out`, `temporarily_unavailable` |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `MenuCatalogStatusPolicy` | Item có được hiển thị/order không? | item catalog status, menu status | `ITEM_NOT_ORDERABLE` | Audit khi manager đổi status |
| `MenuAvailabilityPolicy` | Item có còn phục vụ tại branch không? | branch, item availability, sold-out reason | `ITEM_UNAVAILABLE` | Notify customer/cashier khi sold-out |
| `ModifierAvailabilityPolicy` | Modifier có hợp lệ tại submit không? | item, modifier, current availability | `MODIFIER_UNAVAILABLE` | Optional audit |
| `PriceSnapshotPolicy` | Giá nào được dùng khi submit/accept? | current price, cart/order snapshot | `PRICE_CHANGED_REQUIRES_CONFIRMATION` | Audit price change |

Policy menu không tự tạo/hủy order; nó chỉ trả decision cho order module.
