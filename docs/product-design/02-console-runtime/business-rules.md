# Business Rules - Console Runtime

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| CONSOLE_001 | CMD không tự đổi domain state | N/A | Required |
| CONSOLE_002 | Mọi command phải gửi actor/context | `PermissionPolicy` | Required |
| CONSOLE_003 | Customer CMD chỉ thao tác session của bàn đang bind | `DeviceBindingPolicy` | Required |
| CONSOLE_004 | Kitchen CMD chỉ thấy station của mình | `DeviceBindingPolicy` | Required |
| CONSOLE_005 | Sau mỗi command phải refresh notification | `NotificationPolicy` | Required |

## Context rules

| Context | Required fields |
| --- | --- |
| Customer | `tableId`, active `sessionId` |
| Cashier/Staff | `staffId`, `roles`, `branchId` |
| Kitchen | `staffId`, `stationId` |
| Manager | `staffId`, `branchId` |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `ConsoleActorContextPolicy` | Màn hình CMD/web này có đại diện đúng actor/table/station không? | role, tableId, stationId, staffId | `INVALID_ACTOR_CONTEXT` | Audit warning khi context nhạy cảm sai |
| `ConsoleCommandRoutingPolicy` | Command có được chạy trong context này không? | command name, actor context | `COMMAND_NOT_AVAILABLE_IN_CONTEXT` | Không notify nếu deny |
| `NotificationRecoveryPolicy` | CMD/web stale/offline đồng bộ lại thế nào? | lastSeenNotificationId, resource state | `NOTIFICATION_SYNC_REQUIRED` | Notification replay nếu cần |

Console chỉ là adapter; mọi rule chính thức dùng `../17-policy-governance/`.
