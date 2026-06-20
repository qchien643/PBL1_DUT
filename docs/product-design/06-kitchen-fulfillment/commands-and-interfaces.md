# Commands And Interfaces - Kitchen Fulfillment

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `ViewStationTasks` | `KitchenService.getStationTasks` | Kitchen | `PermissionPolicy` |
| `StartTask` | `KitchenService.startTask` | Kitchen | `PermissionPolicy` |
| `MarkTaskReady` | `KitchenService.markTaskReady` | Kitchen | `NotificationPolicy` |
| `ReportIssue` | `KitchenService.reportIssue` | Kitchen | `NotificationPolicy`, `AuditPolicy` |
| `MarkServed` | `KitchenService.markServed` | Waiter/Staff | `PermissionPolicy` |

## Interface notes

| Method | Key validation |
| --- | --- |
| `startTask` | Task must be `pending` and not cancelled |
| `markTaskReady` | Task must be `preparing` |
| `reportIssue` | Reason required |

## Policy-aware service flow

```text
createKitchenTasks
→ CanCreateKitchenTaskPolicy
→ KitchenRoutingPolicy
→ persist preparation_tasks/task_items
→ notify station
```

```text
start/ready/served/reportIssue
→ PermissionPolicy
→ KitchenTaskStatePolicy or KitchenIssuePolicy
→ persist state
→ audit/notify affected actors
```
