# Commands And Interfaces - Console Runtime

| Console | Commands |
| --- | --- |
| Customer/Menu CMD | `ViewMenu`, `ViewRecommendations`, `SubmitOrder`, `RequestCancelOrderItem`, `RequestBill` |
| Cashier/Staff CMD | `OpenTable`, `MergeTable`, `TransferTable`, `AcceptOrder`, `ApproveCancelOrderItem`, `ConfirmPayment` |
| Kitchen CMD | `ViewStationTasks`, `StartTask`, `MarkTaskReady`, `ReportIssue` |
| Manager CMD | `UpdateBranchConfig`, `CreateMenuItem`, `SetItemAvailability`, `TrainRecommendationModel`, `ViewDailyRevenue` |

## Base interface

| Method | Input | Output |
| --- | --- | --- |
| `startConsoleSession` | `role`, `contextId` | `consoleSessionId` |
| `dispatchCommand` | `commandName`, `payload`, `actorContext` | `Result` |
| `refreshNotifications` | `actorContext` | unread notifications |

## Policy-aware command dispatch

```text
dispatchCommand
→ build ActorContext
→ ConsoleActorContextPolicy
→ PermissionPolicy
→ route to application service
→ return PolicyDecision or command result
```

CMD không tự disable/enable nghiệp vụ bằng hard-coded rule; CMD chỉ phản ánh decision từ service.
