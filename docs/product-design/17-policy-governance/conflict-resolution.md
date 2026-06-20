# Conflict Resolution

## 1. Priority Chính Thức

Khi nhiều rule xung đột, áp dụng thứ tự ưu tiên:

```text
Data integrity
→ Money correctness
→ Kitchen cost protection
→ Customer fairness
→ UX convenience
```

## 2. Ý Nghĩa Từng Mức

| Priority | Ý nghĩa | Ví dụ |
|---|---|---|
| Data integrity | Không làm hỏng state/session/order/bill | Không tạo hai active session trên cùng bàn |
| Money correctness | Không tính thiếu/sai/nhân đôi tiền | Không pay bill stale |
| Kitchen cost protection | Bảo vệ chi phí nguyên liệu/công bếp | Không hủy tự động món đang preparing |
| Customer fairness | Không tự quyết thay khách | Sold-out khi accept phải hỏi lại khách |
| UX convenience | Thuận tiện thao tác | Gửi thông báo, disable button, auto refresh |

## 3. Ví Dụ Quyết Định

| Tình huống | Rule xung đột | Quyết định |
|---|---|---|
| Khách hủy món đang `PREPARING` | Customer fairness vs kitchen cost | Chặn hủy thường lệ, cần manager override |
| Cashier muốn tạo bill khi còn task `ISSUE` | UX nhanh vs money correctness | Chặn bill, resolve issue trước |
| Cashier accept order có món sold-out | UX nhanh vs customer fairness | Hỏi khách thay vì partial accept |
| Bill `OPEN`, khách gọi thêm | UX linh hoạt vs money correctness | Void/reopen bill, không sửa bill âm thầm |
| Notification bị mất | UX convenience vs source of truth | Reload state từ DB, không dựa notification |

## 4. Rule Precedence Trong Service

Application service nên check theo thứ tự:

```text
Permission
→ Resource exists
→ State integrity
→ Cross-module blockers
→ Money/kitchen impact
→ Optional UX rules
```

Ví dụ `requestBill`:

```text
1. PermissionPolicy
2. Session exists and active
3. No submitted/needs-confirmation order
4. No pending/preparing/issue kitchen task
5. No unresolved cancellation
6. No existing paid/stale/open bill conflict
7. Create bill snapshot
```

## 5. Không Được Giải Quyết Bằng Cách Nào?

- Không để UI tự bỏ qua rule để “demo nhanh”.
- Không dùng `delete` để xóa lịch sử nghiệp vụ.
- Không để billing tự hủy/order replacement thay module order.
- Không để kitchen tự quyết món có tính tiền hay không.
- Không gửi notification trước khi state chính commit thành công.

