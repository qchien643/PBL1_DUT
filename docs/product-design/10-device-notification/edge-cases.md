# Edge Cases - Device Notification

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Notification trùng | Idempotency by `eventId` | No duplicate |
| Notification stale | Load resource status when opened | Correct action |
| CMD offline | Notification stays unread | User sees later |
| Role changed after notification | Permission checked before action | Security |
| Customer CMD stale after transfer table | Reload context before command | Table/session |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Notification mất do CMD offline | `NotificationRecoveryPolicy` | Client gọi state API và đọc unread sau `lastSeenId` | No | Replay |
| Notification trùng | `NotificationDedupPolicy` | Bỏ qua ID đã đọc | No | No duplicate toast |
| Route thiếu recipient | `NotificationRoutingPolicy` | Ghi warning, không rollback state chính nếu không critical | Warning | Manager if repeated |
| Event gửi trước commit | `NotificationAfterStateChangePolicy` | Không cho gửi; chỉ emit sau persist thành công | Required if violated | No |
