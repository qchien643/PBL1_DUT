# Data Design - Kitchen Fulfillment

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `preparation_stations` | Kitchen/bar station | `id`, `branchId`, `name`, `type`, `status` |
| `station_routing_rules` | Category -> station | `categoryId`, `stationId` |
| `preparation_tasks` | Task theo order/station | `id`, `orderId`, `stationId`, `status`, `createdAt` |
| `task_items` | Món trong task | `taskId`, `orderItemId`, `quantity`, `status` |
| `task_status_history` | Lịch sử task | `taskId`, `fromStatus`, `toStatus`, `actorId` |
| `kitchen_issues` | Bếp báo vấn đề | `taskItemId`, `reason`, `status`, `reportedBy` |

## Indexes

| Table | Index |
| --- | --- |
| `preparation_tasks` | `stationId`, `status` |
| `task_items` | `orderItemId`, `status` |
| `station_routing_rules` | unique `categoryId` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `preparation_tasks.status` | `KitchenTaskStatePolicy`, billing gates | `PENDING/PREPARING/READY/SERVED/ISSUE/CANCELLED` |
| `task_items.orderItemId` | cancel/bill linkage | Không mất liên kết order item |
| `station_routing_rules` | `KitchenRoutingPolicy` | Không fallback âm thầm |
| `kitchen_issue.reason` | `KitchenIssuePolicy`, `AuditRequiredPolicy` | Required cho issue ảnh hưởng bill |
