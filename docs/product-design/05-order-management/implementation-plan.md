# Implementation Plan - Order Management

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Create order schema | Tables exist |
| 2 | Implement `OrderingPolicy` | Invalid submit blocked |
| 3 | Implement `submitOrder` | Creates awaiting order with snapshots |
| 4 | Implement approval flow | Staff can accept/reject |
| 5 | Integrate kitchen routing | Accepted order creates tasks |
| 6 | Implement cancellation flow | Customer can request cancel item |
| 7 | Add idempotency | Duplicate submit safe |
| 8 | Add audit/notification | Order events visible |

## Acceptance criteria

- Order cannot bypass staff approval.
- Cancelled item is not billed.
- Cancellation reason is stored.
