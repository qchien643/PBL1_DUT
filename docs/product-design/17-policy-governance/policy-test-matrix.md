# Policy Test Matrix

## 1. Core Scenario Coverage

| Scenario | Policy chính | Expected result |
|---|---|---|
| TS-01 mở bàn available | `CanOpenTablePolicy` | Tạo active session |
| TS-02 mở bàn occupied | `CanOpenTablePolicy` | Deny `TABLE_ALREADY_ACTIVE` |
| TS-04 chuyển bàn sang bàn trống | `CanTransferTablePolicy` | Session đổi table |
| TS-05 chuyển sang bàn occupied | `CanTransferTablePolicy` | Deny `TARGET_TABLE_OCCUPIED` |
| TS-06 ghép hai bàn active | `CanMergeTablePolicy` | Session phụ merged |
| MI-02 submit món sold-out | `MenuAvailabilityPolicy` | Deny `ITEM_UNAVAILABLE` |
| MI-03 accept order có món sold-out | `UnavailableItemDecisionPolicy` | Order `NEEDS_CUSTOMER_CONFIRMATION` |
| OR-02 submit trùng request | `OrderIdempotencyPolicy` | Return existing order |
| OR-04 hủy item accepted/task pending | `CanCancelOrderItemPolicy` | Item/task `CANCELLED` |
| OR-05 hủy item preparing | `CanCancelOrderItemPolicy` | Deny hoặc manager override |
| KF-05 ready khi chưa preparing | `KitchenTaskStatePolicy` | Deny invalid state |
| KF-08 cancel/start race | `CancelKitchenRacePolicy` | Dùng state mới nhất |
| KF-10 bếp làm sai món | `KitchenIssuePolicy` | Task `ISSUE`, bill blocked |
| BP-02 bill khi task preparing | `CanCreateBillPolicy` | Deny bill |
| BP-07 bill khi order cần khách xác nhận | `CustomerDecisionGatePolicy` | Deny bill |
| BP-08 bill khi task issue | `KitchenCompletionGatePolicy` | Deny bill |
| BP-09 reopen bill để gọi thêm | `BillLockPolicy` | Void bill open, session active |
| BP-10 pay bill stale | `BillStalenessPolicy` | Deny payment |
| BP-11 cash thiếu tiền | `CanPayBillPolicy` | Deny payment |
| NA-05 notification lặp | `NotificationDedupPolicy` | Không hiện trùng |
| NA-06 manager void | `ManagerOverridePolicy`, `AuditRequiredPolicy` | Audit required |

## 2. Test Case Format

| Field | Required |
|---|---|
| Given state | Yes |
| Command | Yes |
| Actor context | Yes |
| Policy expected | Yes |
| State after | Yes |
| Audit expected | Yes |
| Notification expected | Yes |
| Error code if denied | Yes |

## 3. Acceptance Criteria

- Mỗi policy P0/P1 có ít nhất một scenario allowed và một scenario denied.
- Money-impact policy phải kiểm tra audit.
- Notification policy phải kiểm tra không gửi event trước khi state commit.
- Billing policy phải kiểm tra blocker list, không chỉ trả lỗi chung chung.
- Edge case sold-out tại accept phải hỏi khách, không partial accept mặc định.
