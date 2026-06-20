# Business Rules - Device Notification

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| NOTI_001 | Order submitted báo cashier/staff | `NotificationPolicy` | Required |
| NOTI_002 | Order accepted báo kitchen/bar | `NotificationPolicy` | Required |
| NOTI_003 | Cancel request báo cashier/staff | `NotificationPolicy` | Required |
| NOTI_004 | Task ready báo waiter/staff | `NotificationPolicy` | Required |
| NOTI_005 | Bill requested báo cashier | `NotificationPolicy` | Required |
| NOTI_006 | Notification không gửi sai role | `PermissionPolicy` | Required |

## Routing table

| Event | Recipient |
| --- | --- |
| `order_submitted` | Cashier/Staff CMD |
| `order_accepted` | Kitchen CMD by station |
| `task_ready` | Cashier/Staff CMD |
| `cancel_requested` | Cashier/Staff CMD |
| `bill_requested` | Cashier CMD |
| `item_sold_out` | Customer/Menu CMD, Staff CMD |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `NotificationRoutingPolicy` | Event này gửi tới recipient nào? | event type, resource, actor, branch | `NOTIFICATION_ROUTE_NOT_FOUND` | No audit unless critical route missing |
| `NotificationDedupPolicy` | Notification trùng có hiện lại không? | notificationId, recipient last seen | `DUPLICATE_NOTIFICATION` | No audit |
| `NotificationRecoveryPolicy` | Client offline/stale đồng bộ lại thế nào? | lastSeenId, resource version | `NOTIFICATION_SYNC_REQUIRED` | No audit |
| `NotificationAfterStateChangePolicy` | Có được gửi notification trước commit không? | transaction state, domain event | `NOTIFICATION_SYNC_REQUIRED` | Audit if critical event failed |

Notification là tín hiệu refresh; DB/domain state mới là source of truth.
