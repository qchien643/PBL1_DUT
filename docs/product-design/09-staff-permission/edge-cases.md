# Edge Cases - Staff Permission

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Kitchen gọi `ConfirmPayment` | Forbidden | Payment |
| Customer dùng session bàn khác | Validate table/session binding | Security |
| Staff bị đổi role khi CMD đang mở | Service đọc role mới từ DB | All commands |
| Cashier hủy món preparing | Chặn hoặc yêu cầu manager override | Order |
| Manager tự xóa role manager cuối cùng | Chặn | Admin safety |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Customer dùng session bàn khác | `ActorScopePolicy` | Deny, yêu cầu reload context | Warning | Customer |
| Staff mất role khi CMD còn mở | `PermissionPolicy` | Deny command theo role mới | Required nếu command nhạy cảm | Staff |
| Cashier void món preparing | `ManagerOverridePolicy` | Deny nếu không có manager approval | Required | Cashier/manager |
| Manager cuối cùng tự xóa quyền | `RoleSafetyPolicy` | Deny để tránh mất quyền quản trị | Critical | Manager |
