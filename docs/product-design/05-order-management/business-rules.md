# Business Rules - Order Management

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| ORDER_001 | Session active mới được submit order | `OrderingPolicy` | Required |
| ORDER_002 | Session billing không được submit order | `OrderingPolicy`, `PaymentPolicy` | Required |
| ORDER_003 | Cart không được rỗng | `OrderingPolicy` | Required |
| ORDER_004 | Món phải orderable | `InventoryPolicy` | Required |
| ORDER_005 | Staff phải duyệt trước khi xuống bếp | `ApprovalPolicy` | Required |
| ORDER_006 | Accepted order mới tạo kitchen task | `KitchenRoutingPolicy` | Required |
| ORDER_007 | Hủy món đặt nhầm theo `OrderItem`, không hủy cả order mặc định | `CancellationPolicy` | Required |
| ORDER_008 | Hủy món phải có reason và audit | `CancellationPolicy`, `AuditPolicy` | Required |
| ORDER_009 | Submit order nên idempotent bằng `clientRequestId` | `OrderingPolicy` | Recommended |

## Cancellation rules

| Item state | Decision |
| --- | --- |
| `awaiting_approval` | Staff cancel, không tính bill |
| `accepted/pending_kitchen` | Staff cancel, báo bếp |
| `preparing` | Manager approval hoặc block |
| `ready/served` | Không dùng cancel flow, chuyển complaint/adjustment |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `CanSubmitOrderPolicy` | Customer có được submit cart không? | session, table, cart, menu availability | `TABLE_NOT_ACTIVE`, `EMPTY_CART`, `SESSION_LOCKED_FOR_BILLING` | Audit submit, notify cashier |
| `OrderIdempotencyPolicy` | Submit trùng có tạo order mới không? | `sessionId`, `clientRequestId` | `DUPLICATE_REQUEST` | Optional debug audit |
| `CanAcceptOrderPolicy` | Cashier có được accept order không? | order status, session status, item availability | `ORDER_NOT_ACCEPTABLE` | Audit required |
| `UnavailableItemDecisionPolicy` | Sold-out tại accept xử lý thế nào? | order items, availability | `ITEM_UNAVAILABLE_REQUIRES_CUSTOMER_DECISION` | Audit + notify customer |
| `CanCancelOrderItemPolicy` | Item có được hủy theo trạng thái hiện tại không? | order item, task status, actor role | `CANCEL_TOO_LATE`, `MANAGER_OVERRIDE_REQUIRED` | Audit required |

Default: không accept một phần khi món hết; order chuyển `NEEDS_CUSTOMER_CONFIRMATION`.
