# Commands And Interfaces - Order Management

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `SubmitOrder` | `OrderService.submitOrder` | Customer | `OrderingPolicy`, `InventoryPolicy`, `PricingPolicy` |
| `AcceptOrder` | `OrderService.acceptOrder` | Cashier/Staff | `PermissionPolicy`, `ApprovalPolicy` |
| `RejectOrder` | `OrderService.rejectOrder` | Cashier/Staff | `PermissionPolicy`, `AuditPolicy` |
| `RequestCancelOrderItem` | `OrderService.requestCancelOrderItem` | Customer | `CancellationPolicy` |
| `ApproveCancelOrderItem` | `OrderService.approveCancelOrderItem` | Staff/Manager | `PermissionPolicy`, `CancellationPolicy` |
| `RejectCancelOrderItem` | `OrderService.rejectCancelOrderItem` | Staff/Manager | `PermissionPolicy`, `AuditPolicy` |

## Input/output

| Method | Input | Output |
| --- | --- | --- |
| `submitOrder` | `sessionId`, `clientRequestId`, `items[]` | `orderId`, `status` |
| `acceptOrder` | `orderId`, `actorId` | `preparationTaskIds[]` |
| `requestCancelOrderItem` | `orderItemId`, `reason` | `requestId`, `decision` |

## Policy-aware service flow

```text
submitOrder
→ CanSubmitOrderPolicy
→ MenuAvailabilityPolicy / ModifierAvailabilityPolicy
→ PriceSnapshotPolicy
→ OrderIdempotencyPolicy
→ persist order
→ audit + notify cashier
```

```text
acceptOrder
→ PermissionPolicy
→ CanAcceptOrderPolicy
→ MenuAvailabilityPolicy
→ UnavailableItemDecisionPolicy if needed
→ KitchenRoutingPolicy
→ create kitchen tasks only if fully accepted
```

Mọi deny response dùng format `PolicyDecision` từ `../17-policy-governance/policy-architecture.md`.
