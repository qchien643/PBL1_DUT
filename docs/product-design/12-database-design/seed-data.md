# Seed Data

## 1. Required seed

| Group | Data |
| --- | --- |
| Branch | `branch_main`, `businessProfile = casual_dining` |
| Config | Casual dining default config |
| Staff | Manager, cashier, waiter, kitchen |
| Tables | 8-12 dining tables |
| Stations | `kitchen`, `bar` |
| Menu categories | Food, Drink, Dessert |
| Menu items | 15-25 items |
| Availability | All items available initially, 1-2 sold out for testing |

## 2. Recommendation seed

| Data | Purpose |
| --- | --- |
| 30-50 historical sessions | Train latent factor |
| Paid bills for historical sessions | Valid interactions |
| Item pair rules | Fallback recommendations |
| Recommendation events sample | Reporting/demo |

## 3. Demo scenarios seed

| Scenario | Seed needed |
| --- | --- |
| Sold out item | One drink or dessert sold out |
| Merge table | Adjacent available tables |
| Transfer table | One occupied, one available table |
| Cancel item | Pending order item |
| Kitchen issue | Item with available false after accepted |

## 4. Reset command

MVP nên có `ResetDemoData` cho Manager hoặc dev mode để trả DB về trạng thái seed trước khi demo.

## 5. Policy scenario seed

| Scenario | Seed data |
| --- | --- |
| `CanOpenTablePolicy` deny | Một bàn `OCCUPIED`, một bàn `CLEANING` |
| `UnavailableItemDecisionPolicy` | Một order submitted có item sau đó sold-out |
| `CanCancelOrderItemPolicy` | Một item task `PENDING`, một item task `PREPARING` |
| `KitchenIssuePolicy` | Một task có thể chuyển `ISSUE` với reason |
| `BillStalenessPolicy` | Một bill open có `sessionVersion` cũ |
| `CanPayBillPolicy` | Bill total mẫu để test thiếu/thừa tiền |
| `NotificationRecoveryPolicy` | Notification unread/read theo recipient |
| `AuditRequiredPolicy` | Audit mẫu cho payment, void, config change |
