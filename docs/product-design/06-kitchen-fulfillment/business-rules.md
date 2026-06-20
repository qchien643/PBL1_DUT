# Business Rules - Kitchen Fulfillment

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| KIT_001 | Chỉ order accepted mới tạo task | `KitchenRoutingPolicy` | Required |
| KIT_002 | Mỗi order item phải route được station | `KitchenRoutingPolicy` | Required |
| KIT_003 | Kitchen CMD chỉ cập nhật task station của mình | `PermissionPolicy` | Required |
| KIT_004 | Task ready phải báo staff | `NotificationPolicy` | Required |
| KIT_005 | TaskItem cancelled không hiển thị pending | `KitchenRoutingPolicy` | Required |
| KIT_006 | Kitchen issue phải báo cashier/staff | `NotificationPolicy` | Required |

## Task states

| State | Meaning |
| --- | --- |
| `pending` | Chờ bếp làm |
| `preparing` | Đang làm |
| `ready` | Sẵn sàng giao |
| `served` | Đã giao |
| `issue_reported` | Có vấn đề, cần staff xử lý |
| `cancelled` | Bị hủy trước khi làm |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `CanCreateKitchenTaskPolicy` | Item đã đủ điều kiện xuống bếp chưa? | order item, order status, customer decision state | `ORDER_NOT_READY_FOR_FULFILLMENT` | Audit accept/task create |
| `KitchenRoutingPolicy` | Item route tới kitchen/bar station nào? | category, item station, station status | `KITCHEN_STATION_NOT_CONFIGURED` | Notify station/manager |
| `KitchenTaskStatePolicy` | Task có được start/ready/served không? | task status, actor station | `KITCHEN_TASK_INVALID_STATE` | Audit warning nếu state sai |
| `KitchenIssuePolicy` | Bếp có được report issue không? | task status, issue reason | `KITCHEN_ISSUE_NOT_ALLOWED` | Audit high, notify cashier/customer |
| `ReadyToServedPolicy` | `READY` đã đủ billable chưa? | waiter flow config, task status | `ITEM_NOT_SERVED_YET` | Optional audit |

Kitchen không tự hủy/thay món; issue quay về order/billing resolution flow.
