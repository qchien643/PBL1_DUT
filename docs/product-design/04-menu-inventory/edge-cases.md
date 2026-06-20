# Edge Cases - Menu Inventory

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Món sold out khi đang trong cart | Submit bị chặn, gợi ý món thay thế | Order/Recommendation |
| Giá đổi khi cart đang mở | Submit hiển thị giá mới và snapshot | Pricing |
| Món archived nhưng order cũ cần hiển thị | Order dùng snapshot | Reporting |
| Modifier bị disable trong lúc chọn | Submit validate lại modifier | Order |
| Bếp báo hết món sau accepted | Tạo kitchen issue, staff xử lý hủy/đổi món | Kitchen/Order |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Item sold-out sau khi khách thêm cart | `MenuAvailabilityPolicy` | Deny item tại submit/accept, hỏi lại khách nếu order đã submitted | Availability audit | Customer/cashier |
| Giá đổi khi cart đang mở | `PriceSnapshotPolicy` | Yêu cầu xác nhận giá mới hoặc dùng snapshot đã submit | Required nếu manager đổi giá | Customer if affected |
| Modifier bị disable | `ModifierAvailabilityPolicy` | Reject modifier, cho khách sửa cart | Optional | Customer |
| Item archived nhưng report/order cũ cần xem | `MenuCatalogStatusPolicy` | Dùng snapshot trong order/bill, không xóa item lịch sử | No | No |
