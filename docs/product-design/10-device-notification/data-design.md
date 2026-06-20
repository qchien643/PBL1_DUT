# Data Design - Device Notification

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `devices` | Device logical/physical | `id`, `type`, `code`, `status` |
| `device_assignments` | Device-table/station mapping | `deviceId`, `tableId`, `stationId` |
| `console_sessions` | CMD runtime context | `id`, `role`, `contextId`, `startedAt` |
| `notifications` | Notification record | `id`, `eventId`, `eventType`, `title`, `message`, `priority` |
| `notification_recipients` | Recipients/read status | `notificationId`, `recipientType`, `recipientId`, `readAt` |

## Indexes

| Table | Index |
| --- | --- |
| `notifications` | unique `eventId`, `eventType` |
| `notification_recipients` | `recipientId`, `readAt` |
| `device_assignments` | `tableId`, `stationId` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `notifications.eventId` | `NotificationDedupPolicy` | Chống duplicate route |
| `notification_recipients.lastSeenId` | `NotificationRecoveryPolicy` | Polling/replay |
| `device_assignments` | `NotificationRoutingPolicy` | Map table/station/device |
| `resourceVersion` in payload | `NotificationRecoveryPolicy` | Client biết cần reload |
