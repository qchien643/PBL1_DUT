# Commands And Interfaces - Implementation Roadmap

## Build order by command availability

| Milestone | Commands available |
| --- | --- |
| M1 | `Login`, `ViewBranchConfig` |
| M2 | `OpenTable`, `ViewMenu` |
| M3 | `SubmitOrder`, `AcceptOrder` |
| M4 | `ViewStationTasks`, `StartTask`, `MarkTaskReady` |
| M5 | `RequestBill`, `ConfirmPayment` |
| M6 | `RequestCancelOrderItem`, `ApproveCancelOrderItem` |
| M7 | `ViewRecommendations`, `TrainRecommendationModel` |
| M8 | `ViewDailyRevenue`, `ViewAuditLog` |

## Service order

`ConfigurationService` -> `StaffService` -> `TableService` -> `MenuService` -> `OrderService` -> `KitchenService` -> `PaymentService` -> `RecommendationService` -> `ReportingService`.
