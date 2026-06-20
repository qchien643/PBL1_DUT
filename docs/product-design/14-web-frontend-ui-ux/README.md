# Web Frontend UI/UX Design

Module này chỉ tập trung vào **frontend UI/UX** cho hệ thống đặt món tại bàn của nhà hàng **Casual dining**.

Backend/server/API được tách riêng ở module [15-cpp-server-api](../15-cpp-server-api/README.md).

## 1. Mục Tiêu

- Thay trải nghiệm CMD bất tiện bằng giao diện web chạy trên browser.
- Thiết kế màn hình riêng cho từng actor: customer, cashier, kitchen/bar, manager.
- Đảm bảo frontend chỉ là UI layer, không tự quyết định business rule.
- Thiết kế notification UX bằng toast/badge nhưng không reload toàn trang.

## 2. Phạm Vi Frontend

| Nhóm | Quyết định |
|---|---|
| Công nghệ | HTML/CSS/JavaScript thuần |
| Framework | Chưa dùng ReactJS trong MVP |
| Chạy ở đâu | Browser trên máy cùng mạng/local |
| Data source | Gọi REST API từ C++ server |
| Realtime UX | Polling notification do server cung cấp |
| Local state | Cart, filter, last notification id |

## 3. Tài Liệu Trong Module

| File | Nội dung |
|---|---|
| [ui-ux-design.md](ui-ux-design.md) | Layout, component, UX rule cho từng actor |
| [frontend-state-and-components.md](frontend-state-and-components.md) | State frontend, component map, dữ liệu cần load |
| [notification-ux.md](notification-ux.md) | Cách hiển thị notification trên UI |
| [implementation-plan.md](implementation-plan.md) | Kế hoạch triển khai frontend |

## 4. Frontend Không Làm Gì

- Không lưu database.
- Không tự duyệt order.
- Không tự tính quyền hủy món.
- Không tự chuyển trạng thái bếp/task.
- Không tự quyết định bill có được tạo hay không.

Frontend chỉ gửi intent tới server, ví dụ:

```text
Customer clicks Submit Order
→ frontend POST /api/orders
→ server kiểm tra policy
→ frontend hiển thị kết quả
```

## 5. Policy Decision UX

Frontend phải hiển thị `PolicyDecision` từ server, không tự suy luận rule:

| Field | UX dùng để làm gì |
|---|---|
| `error.code` | Map icon/severity và test case |
| `error.message` | Hiển thị thông điệp cho user |
| `error.requiredAction` | Hiển thị CTA tiếp theo như gọi nhân viên, reload, chọn món khác |
| `error.context` | Hiển thị blocker list: món đang làm, item sold-out, bill stale |
| `correlationId` | Đưa cho staff khi cần tra audit |

Ví dụ: `ITEM_UNAVAILABLE_REQUIRES_CUSTOMER_DECISION` phải mở UI cho khách bỏ món, chọn món thay thế, hủy order hoặc gọi nhân viên.
