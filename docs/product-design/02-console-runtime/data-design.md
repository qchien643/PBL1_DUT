# Data Design - Console Runtime

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `console_sessions` | Phiên chạy CMD | `id`, `role`, `contextType`, `contextId`, `startedAt`, `endedAt` |
| `device_assignments` | Mapping bàn/station nếu mô phỏng device | `deviceId`, `tableId`, `stationId` |
| `notification_recipients` | Cursor notification theo actor/CMD | `notificationId`, `recipientId`, `readAt` |

## Indexes

| Table | Index |
| --- | --- |
| `console_sessions` | `role`, `contextId`, `endedAt` |
| `notification_recipients` | `recipientId`, `readAt` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `console_sessions.actorContext` | `ConsoleActorContextPolicy` | Xác định customer/cashier/kitchen/manager |
| `lastSeenNotificationId` | `NotificationRecoveryPolicy` | Cho phép sync lại khi CMD offline |
| `contextVersion` | `ConsoleCommandRoutingPolicy` | Phát hiện context stale sau chuyển bàn/role change |
