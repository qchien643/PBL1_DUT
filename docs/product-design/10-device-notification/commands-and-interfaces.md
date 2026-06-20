# Commands And Interfaces - Device Notification

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `StartConsoleSession` | `DeviceService.startConsoleSession` | Any CMD | `DeviceBindingPolicy` |
| `GetConsoleNotifications` | `NotificationService.getNotifications` | Any CMD | `NotificationPolicy` |
| `MarkNotificationRead` | `NotificationService.markRead` | Recipient | `PermissionPolicy` |
| `RegisterDevice` | `DeviceService.registerDevice` | Manager | `PermissionPolicy` |
| `AssignDeviceToTable` | `DeviceService.assignToTable` | Manager | `PermissionPolicy` |

## Internal interface

| Method | Input | Output |
| --- | --- | --- |
| `emitDomainEvent` | `eventType`, `resourceId`, `payload` | `eventId` |
| `routeNotification` | `eventId` | notification records |

## Policy-aware service flow

```text
domain state committed
→ emitDomainEvent
→ NotificationAfterStateChangePolicy
→ NotificationRoutingPolicy
→ NotificationDedupPolicy per recipient
→ persist notification_recipients
```

Client polling luôn kết hợp notification với reload resource state.
