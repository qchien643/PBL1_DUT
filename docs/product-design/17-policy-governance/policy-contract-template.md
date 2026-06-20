# Policy Contract Template

## 1. Format Chuẩn

Mỗi policy trong tài liệu phải có đủ các trường sau:

| Field | Ý nghĩa |
|---|---|
| `PolicyName` | Tên policy duy nhất |
| `Owner module` | Module chịu trách nhiệm định nghĩa rule |
| `Business question` | Câu hỏi policy trả lời |
| `Input context` | Dữ liệu cần load trước khi check |
| `Allowed when` | Điều kiện cho phép |
| `Denied when` | Điều kiện từ chối |
| `Deny codes` | Error code trả về |
| `State transition` | Trạng thái thay đổi nếu allowed |
| `Audit` | Có cần audit không |
| `Notification` | Ai cần nhận thông báo |
| `Related scenarios` | Test/demo scenario liên quan |

## 2. Template Markdown

```markdown
## PolicyName

| Field | Value |
|---|---|
| Owner module | |
| Business question | |
| Input context | |
| Allowed when | |
| Denied when | |
| Deny codes | |
| State transition | |
| Audit | |
| Notification | |
| Related scenarios | |
```

## 3. Ví Dụ: CanOpenTablePolicy

| Field | Value |
|---|---|
| Owner module | `03-table-session` |
| Business question | Cashier/waiter có được mở bàn này không? |
| Input context | actor, table, active session by table, branch config |
| Allowed when | actor có quyền, table `AVAILABLE`, không có active session |
| Denied when | table occupied/cleaning/billing hoặc actor thiếu quyền |
| Deny codes | `PERMISSION_DENIED`, `TABLE_NOT_AVAILABLE`, `TABLE_ALREADY_ACTIVE` |
| State transition | table `AVAILABLE -> OCCUPIED`, tạo `DiningSession.ACTIVE` |
| Audit | Required |
| Notification | `customer:tableCode`, `cashier` |
| Related scenarios | TS-01, TS-02 |

## 4. Ví Dụ: CanCreateBillPolicy

| Field | Value |
|---|---|
| Owner module | `07-payment-billing` |
| Business question | Session này đã đủ ổn định để tạo bill chưa? |
| Input context | session, orders, order items, cancel requests, kitchen tasks, existing bill |
| Allowed when | session active, không còn order/cancel/task blocker, chưa có bill paid |
| Denied when | còn submitted order, needs customer confirmation, task pending/preparing/issue, cancel unresolved |
| Deny codes | `BILL_BLOCKED_BY_PENDING_ORDER`, `BILL_BLOCKED_BY_KITCHEN_TASK`, `BILL_ALREADY_EXISTS` |
| State transition | tạo bill `OPEN`, session `BILL_REQUESTED` hoặc `billLocked=true` |
| Audit | Required khi tạo bill |
| Notification | `cashier`, `customer:tableCode` |
| Related scenarios | BP-01, BP-02, BP-07, BP-08 |

## 5. Quy Tắc Đặt Tên Policy

| Pattern | Khi dùng | Ví dụ |
|---|---|---|
| `Can<Action>Policy` | Chặn/cho phép một command | `CanOpenTablePolicy` |
| `<Domain>GatePolicy` | Gate trước khi chuyển module | `KitchenCompletionGatePolicy` |
| `<Domain>CalculationPolicy` | Tính toán có rule | `BillCalculationPolicy` |
| `<Domain>RoutingPolicy` | Route notification/task | `KitchenRoutingPolicy` |
| `<Domain>DecisionPolicy` | Chọn nhánh nghiệp vụ | `UnavailableItemDecisionPolicy` |

