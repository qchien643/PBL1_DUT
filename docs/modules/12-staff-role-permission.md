# Module 12 - Staff, Role, Permission

## 1. Mục tiêu

Module này quản lý nhân viên, vai trò và quyền thao tác. Trong MVP CMD, mỗi cửa sổ console có thể đăng nhập bằng một staff hoặc chạy với một role mô phỏng để demo.

## 1.1. Phạm vi Casual dining

| Quyết định | Giá trị |
| --- | --- |
| Role | Manager, cashier, waiter, kitchen, customer |
| Customer auth | Không cần login, dùng table/session context |
| Dynamic permission UI | Không thuộc MVP |
| Branch scope | Một branch |

## 2. Phạm vi

| Nội dung | MVP Casual dining | Ngoài phạm vi Casual dining MVP |
| --- | --- | --- |
| Staff account | Có | SSO hoặc nhiều chi nhánh |
| Role | Manager, cashier, waiter, kitchen | Role tùy chỉnh |
| Permission | Theo role cố định | Permission matrix động |
| Branch scope | Một chi nhánh | Nhiều chi nhánh |
| Audit | Ghi actor cho hành động quan trọng | Audit nâng cao |
| Console role | Mỗi CMD chạy theo một role/context | UI thật với session/token |

## 3. Entity đề xuất

| Entity | Ý nghĩa |
| --- | --- |
| `StaffUser` | Tài khoản nhân viên |
| `Role` | Vai trò |
| `Permission` | Quyền cụ thể |
| `StaffRoleAssignment` | Gán role cho nhân viên |
| `StaffSession` | Phiên đăng nhập |

## 4. Policy liên quan

### 4.1. PermissionPolicy

Input:

- Actor.
- Action.
- Resource.
- Branch scope.

Output:

- `allowed`.
- `missingPermission`.

Ví dụ action:

```json
[
  "table.open",
  "order.accept",
  "order.cancel",
  "bill.discount",
  "payment.confirm",
  "menu.update",
  "report.view"
]
```

## 5. Role matrix MVP

| Permission | Manager | Cashier | Waiter | Kitchen |
| --- | --- | --- | --- | --- |
| `menu.update` | Yes | No | No | No |
| `table.open` | Yes | Yes | Yes | No |
| `order.accept` | Yes | Yes | No | No |
| `order.cancel` | Yes | Yes | No | No |
| `task.update` | Yes | No | No | Yes |
| `item.served` | Yes | No | Yes | No |
| `payment.confirm` | Yes | Yes | No | No |
| `report.view` | Yes | No | No | No |

## 6. Business rules

| Rule ID | Rule | MVP |
| --- | --- | --- |
| PERM_001 | Mọi command nhạy cảm phải kiểm tra quyền | Có |
| PERM_002 | Kitchen không được xác nhận thanh toán | Có |
| PERM_003 | Waiter không được sửa giá/discount | Có |
| PERM_004 | Manager có toàn quyền trong chi nhánh | Có |
| PERM_005 | Actor phải được ghi vào audit log | Có |
| PERM_006 | Console không được tự bypass PermissionPolicy | Có |
| PERM_007 | Customer chỉ được thao tác trên session/table của mình | Có |
| PERM_008 | Kitchen không được thao tác payment/order approval | Có |
| PERM_009 | Manager override hủy món preparing phải audit | Có |

## 7. Console theo role

| Console | Role mặc định | Ghi chú |
| --- | --- | --- |
| `Customer/Menu CMD` | `customer` hoặc anonymous theo `tableId` | Chỉ được xem menu, gửi order, yêu cầu thanh toán |
| `Kitchen CMD` | `kitchen` | Chỉ cập nhật preparation task |
| `Cashier/Staff CMD` | `cashier` hoặc `reception` | Mở bàn, duyệt order, thanh toán |
| `Manager CMD` | `manager` | Quản lý menu, config, báo cáo |

Nếu muốn đơn giản hóa demo, có thể hard-code role khi mở từng CMD. Tuy nhiên service vẫn phải nhận `actor` và gọi `PermissionPolicy`.

## 8. API/Command gợi ý

| Command/Query | Mô tả |
| --- | --- |
| `Login` | Đăng nhập |
| `CreateStaffUser` | Tạo nhân viên |
| `AssignRole` | Gán role |
| `GetMyPermissions` | Lấy quyền hiện tại |
| `CheckPermission` | Kiểm tra quyền nội bộ |

## 9. Edge cases

- Nhân viên bị đổi role khi đang đăng nhập.
- Staff thao tác trên chi nhánh không thuộc phạm vi.
- Tài khoản kitchen cố gọi API payment.
- Manager xóa chính tài khoản của mình.
- Cửa sổ CMD chạy sai role nhưng vẫn gọi command nhạy cảm.
- Waiter cố giảm giá bill.
- Customer CMD dùng sessionId của bàn khác.
- Cashier hủy món preparing mà không có manager approval.

## 9.1. Cách xử lý edge case quan trọng

| Edge case | Cách xử lý |
| --- | --- |
| Customer spoof session | Command phải validate table/device/session binding |
| Staff role bị đổi | Service đọc role hiện tại từ DB trước command nhạy cảm |
| Manager override | Yêu cầu reason và ghi audit |

## 10. Lưu ý triển khai

- MVP có thể dùng role cố định thay vì permission table phức tạp.
- Dù role cố định, nên gom kiểm tra quyền vào `PermissionPolicy`.
- Không nên check role rải trong controller bằng nhiều `if`.
- CMD có thể truyền `actorId`, `role`, `tableId` hoặc `stationId` vào service context.
