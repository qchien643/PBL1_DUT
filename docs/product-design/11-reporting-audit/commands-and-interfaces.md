# Commands And Interfaces - Reporting Audit

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `ViewDailyRevenue` | `ReportingService.getDailyRevenue` | Manager | `ReportAccessPolicy` |
| `ViewTopSellingItems` | `ReportingService.getTopSellingItems` | Manager | `ReportAccessPolicy` |
| `ViewCancelledOrders` | `ReportingService.getCancelledOrders` | Manager | `ReportAccessPolicy` |
| `ViewKitchenPerformance` | `ReportingService.getKitchenPerformance` | Manager | `ReportAccessPolicy` |
| `ViewAuditLog` | `AuditService.getEvents` | Manager | `PermissionPolicy` |

## Internal event interface

| Method | Input | Output |
| --- | --- | --- |
| `recordAuditEvent` | event type, actor, resource, payload | audit id |
| `getAuditTrail` | resource type/id | event list |

## Policy-aware service flow

```text
domain action
→ AuditRequiredPolicy
→ persist state and audit in same transaction if critical
→ report reads immutable audit/bill/order snapshots
```

Report command phải dùng `ReportFilterPolicy` để không tính món cancelled/rejected/voided sai.
