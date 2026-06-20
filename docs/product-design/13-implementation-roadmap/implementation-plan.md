# Implementation Plan - Roadmap

## Phase 1 - Foundation

| Task | Done when |
| --- | --- |
| Project structure | `console`, `application`, `policies`, `repositories` exist |
| Result/error model | Services return consistent errors |
| Enum states | No raw status strings in core logic |

## Phase 2 - Core MVP

| Task | Done when |
| --- | --- |
| Database schema + seed | Demo data ready |
| Table/menu/order/kitchen/payment services | Happy path works |
| Permission/policy integration | Invalid commands blocked |

## Phase 3 - Business hardening

Phase này bắt buộc đưa policy governance vào code trước khi xử lý thêm edge case.

| Task | Done when |
| --- | --- |
| PolicyDecision model | Có `allowed/code/message/requiredAction/auditRequired/notificationTargets` |
| Policy groups | Có permission, table, menu, order, kitchen, billing, governance policy |
| Cancel item flow | Placed-wrong-item case works |
| Sold-out at accept flow | Order chuyển `NEEDS_CUSTOMER_CONFIRMATION`, không partial accept mặc định |
| Bill blocker flow | Bill deny trả blocker list và deny code đúng |
| Notification polling | CMD sees events |
| Audit | Critical actions logged |

Ưu tiên P0: table active, sold-out at accept, cancel by kitchen state, kitchen issue, bill stale/payment amount.

## Phase 4 - Intelligence and demo

| Task | Done when |
| --- | --- |
| Recommendation fallback | Suggestions without model |
| Latent factor training | Manager trains/activates model |
| Reports | Manager sees revenue/top items |
| Demo script | End-to-end and edge cases pass |
