# Business Rules - Table Session

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| TABLE_001 | Chỉ staff có quyền mới mở bàn | `PermissionPolicy` | Required |
| TABLE_002 | Bàn `available` mới được mở | `TablePolicy` | Required |
| TABLE_003 | Một bàn chỉ có một active session | `TablePolicy` | Required |
| TABLE_004 | Session billing không được ghép/chuyển bàn | `TablePolicy` | Required |
| TABLE_005 | Ghép bàn dùng chung session và bill | `TablePolicy` | Required |
| TABLE_006 | Chuyển bàn không làm mất order/bill | `TablePolicy` | Required |
| TABLE_007 | Paid session chuyển mọi bàn sang `cleaning` | `PaymentPolicy` | Required |

## State rules

| Current | Allowed actions |
| --- | --- |
| `available` | `OpenTable` |
| `occupied` | `MergeTable`, `TransferTable`, `SubmitOrder`, `RequestBill` |
| `billing` | `ConfirmPayment`, no new order |
| `cleaning` | `MarkTableCleaned` |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `CanOpenTablePolicy` | Staff có được mở bàn không? | actor, table, active session | `TABLE_NOT_AVAILABLE`, `TABLE_ALREADY_ACTIVE` | Audit required, notify table/cashier |
| `CanMergeTablePolicy` | Hai bàn/session có được ghép không? | source/target sessions, bill state | `TABLE_MERGE_NOT_ALLOWED` | Audit required, notify affected tables |
| `CanTransferTablePolicy` | Session có được chuyển sang bàn đích không? | session, source table, target table | `TARGET_TABLE_OCCUPIED`, `TABLE_NOT_AVAILABLE` | Audit required |
| `TableCleanlinessPolicy` | Bàn cleaning có được mở lại chưa? | table status, cleaning marker | `TABLE_NOT_CLEANED` | Optional audit |
| `BillLockPolicy` | Session đang bill có bị khóa order/merge/transfer không? | session, active bill | `SESSION_LOCKED_FOR_BILLING` | Notify cashier if denied |

Các policy này là nguồn rule cho order/payment; module khác không tự suy luận table state.
