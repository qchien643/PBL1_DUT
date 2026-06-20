# Edge Cases - Console Runtime

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Hai CMD cùng xử lý một order | Service validate trạng thái DB trong transaction | Order |
| Customer CMD stale sau chuyển bàn | Reload `DeviceContext` trước submit | Table/session |
| Kitchen CMD start task đã cancelled | Service trả lỗi `TASK_NOT_ACTIVE` | Kitchen |
| Cashier confirm bill stale | Re-read bill version trước confirm | Payment |
| CMD bị tắt giữa command | Transaction rollback hoặc command idempotent | Data integrity |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Customer CMD dùng table cũ sau chuyển bàn | `ConsoleActorContextPolicy` | Reload session binding trước submit | Optional | Table screen |
| Cashier CMD giữ role cũ sau khi bị đổi quyền | `PermissionPolicy` | Service đọc role mới và deny nếu mất quyền | Required nếu command nhạy cảm | Staff screen |
| CMD nhận notification trễ | `NotificationRecoveryPolicy` | Reload source state, không dựa notification làm source of truth | No | Replay unread |
