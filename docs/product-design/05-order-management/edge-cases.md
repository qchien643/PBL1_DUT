# Edge Cases - Order Management

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Customer bấm submit hai lần | `clientRequestId` trả order cũ | Không duplicate |
| Món sold out lúc submit | `InventoryPolicy` chặn | Customer chọn món khác |
| Staff accept khi session billing | `ApprovalPolicy` revalidate session | Không xuống bếp |
| Hủy một món trong order nhiều món | Chỉ `OrderItem` đó cancelled | Bill/task cập nhật |
| Hủy khi kitchen đang preparing | Manager approval hoặc block | Audit high |
| Kitchen task pending nhưng item cancelled | TaskItem cancelled và ẩn khỏi Kitchen CMD | Kitchen |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Submit order khi bàn chưa mở | `CanSubmitOrderPolicy` | Deny, không tạo order | No | Customer |
| Double submit | `OrderIdempotencyPolicy` | Return order cũ, không nhân đôi task/bill | Optional | No duplicate |
| Cashier accept có item sold-out | `UnavailableItemDecisionPolicy` | Chuyển order `NEEDS_CUSTOMER_CONFIRMATION` | Required | Customer/cashier |
| Hủy item task pending | `CanCancelOrderItemPolicy` | Item/task `CANCELLED`, không tính bill | Required | Kitchen/customer |
| Hủy item task preparing | `CanCancelOrderItemPolicy`, `ManagerOverridePolicy` | Deny hoặc manager void/discount | Required | Customer/cashier |
