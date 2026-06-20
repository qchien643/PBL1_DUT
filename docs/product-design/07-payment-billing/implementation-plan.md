# Implementation Plan - Payment Billing

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Create billing schema | Bill/payment tables exist |
| 2 | Implement `PricingPolicy` | Calculates subtotal/tax/fee |
| 3 | Implement `PaymentPolicy` | Blocks invalid bill/payment |
| 4 | Implement request bill | Creates idempotent bill |
| 5 | Implement confirm payment | Marks bill paid and closes session |
| 6 | Integrate audit/reporting | Paid bill visible in report |

## Acceptance criteria

- Cancelled item not billed.
- Paid bill closes session.
- Confirm payment is audited.
