# Schema Overview

## 1. Schema groups

| Group | Purpose |
| --- | --- |
| Configuration | Nhà hàng, branch, config, feature |
| Staff | User, role, permission |
| Table Session | Bàn và phiên phục vụ |
| Menu Inventory | Menu, món, modifier, availability |
| Order | Order header/item/cancellation |
| Kitchen | Station, task, issue |
| Billing | Bill, line, adjustment, payment |
| Notification | Notification và recipient |
| Recommendation | Interaction, model, vector, event |
| Audit Reporting | Audit, config version, report sources |

## 2. Naming conventions

| Type | Convention |
| --- | --- |
| Table | snake_case plural |
| Primary key | `id` |
| Foreign key | `{entity}Id` |
| Status | enum string |
| Timestamp | `createdAt`, `updatedAt`, domain-specific time |

## 3. Transaction boundaries

| Command | Tables updated together |
| --- | --- |
| `OpenTable` | `dining_sessions`, `dining_session_tables`, `table_status_history`, `audit_events` |
| `SubmitOrder` | `order_headers`, `order_items`, `order_item_modifiers`, `order_status_history` |
| `AcceptOrder` | `order_headers`, `preparation_tasks`, `task_items`, `notifications`, `audit_events` |
| `ApproveCancelOrderItem` | `cancellation_requests`, `order_items`, `task_items`, `bill_lines`, `audit_events` |
| `ConfirmPayment` | `bills`, `payments`, `dining_sessions`, `dining_tables`, `audit_events` |

## 4. Policy support principles

| Principle | Required data |
| --- | --- |
| Policy decision phải dựa trên state mới nhất | status/version fields trên session/order/task/bill |
| Idempotency chống double submit/pay | `clientRequestId`, `idempotencyKey`, unique constraint |
| Bill snapshot không bị lỗi thời âm thầm | `sessionVersion`, `billVersion`, `status=STALE` |
| Kitchen issue chặn billing | task/item issue status + reason |
| Audit giải thích được action | actor, reason, before/after payload, correlationId |
| Notification không là source of truth | notification chỉ chứa event pointer/resource version |

Database không tự quyết business rule, nhưng phải lưu đủ dữ liệu để `17-policy-governance` kiểm tra rule.
