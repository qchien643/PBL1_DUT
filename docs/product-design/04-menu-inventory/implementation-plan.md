# Implementation Plan - Menu Inventory

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Create menu schema | Menu tables exist |
| 2 | Seed categories/items | Customer CMD sees menu |
| 3 | Implement availability schema | Sold out state stored |
| 4 | Implement `MenuAvailabilityPolicy` | Hidden/archived filtered |
| 5 | Implement `InventoryPolicy` | Sold out blocked |
| 6 | Implement manager commands | CRUD item, availability update |
| 7 | Add snapshot support | Order can save item data |

## Acceptance criteria

- Customer sees only orderable items.
- Manager can mark sold out.
- Sold out item cannot be ordered or recommended.
