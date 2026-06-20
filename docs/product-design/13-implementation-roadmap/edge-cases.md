# Edge Cases - Implementation Roadmap

## Must-demo edge cases

| Edge case | Expected result |
| --- | --- |
| Submit order khi bàn chưa mở | Blocked by `OrderingPolicy` |
| Gọi món sold out | Blocked by `InventoryPolicy` |
| Hủy món trước khi bếp làm | Item cancelled, not billed |
| Hủy món khi bếp preparing | Requires manager approval or blocked |
| Request bill khi còn task preparing | Blocked by `PaymentPolicy` |
| Kitchen cố confirm payment | Blocked by `PermissionPolicy` |
| Không có recommendation model | Fallback best seller |

## Regression list

- No duplicated order on repeated submit.
- No billing cancelled item.
- No cross-table customer action.
- No task update after cancelled.
