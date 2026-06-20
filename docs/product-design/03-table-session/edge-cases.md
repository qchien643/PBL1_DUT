# Edge Cases - Table Session

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Mở bàn đang occupied | `TablePolicy` từ chối | Không tạo session |
| Ghép bàn đang có khách | `TablePolicy` từ chối | Không đổi bill |
| Chuyển bàn khi Customer CMD đang submit | Order validate theo `sessionId`, không theo bàn cũ | Order |
| Bàn chuyển sang cleaning nhưng Customer CMD cũ còn mở | `DeviceContext` yêu cầu reload | Console |
| Request bill khi còn task preparing | `PaymentPolicy` chặn | Payment |
| Mark cleaned một bàn trong nhóm chưa paid | `TablePolicy` chặn | Session integrity |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Mở bàn đang occupied | `CanOpenTablePolicy` | Deny, không tạo session mới | Warning | Cashier |
| Ghép bàn khi một session đã bill open | `CanMergeTablePolicy`, `BillLockPolicy` | Deny, yêu cầu xử lý bill trước | Required | Cashier |
| Chuyển sang bàn occupied | `CanTransferTablePolicy` | Deny, gợi ý merge table nếu phù hợp | Required | Cashier/waiter |
| Mở lại bàn cleaning | `TableCleanlinessPolicy` | Deny đến khi mark cleaned | Optional | Cashier |
