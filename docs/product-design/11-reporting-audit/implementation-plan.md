# Implementation Plan - Reporting Audit

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Create audit schema | `audit_events` exists |
| 2 | Implement `AuditPolicy` | Required events known |
| 3 | Integrate audit into services | Actions logged |
| 4 | Implement report queries | Revenue/top/cancel reports |
| 5 | Add manager CMD screens | Manager can view reports |
| 6 | Use reporting for recommendation | Interactions built from valid history |

## Acceptance criteria

- Paid bill appears in revenue.
- Cancelled items excluded from top selling.
- Payment/cancel/config changes are audited.
