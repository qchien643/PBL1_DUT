# Business Deep Dive For Casual Dining

Module này tập trung vào **phân tích nghiệp vụ chuyên sâu** và **edge case** để phục vụ phần đánh giá môn học.

Các policy chính thức, deny code, audit và notification mapping được chuẩn hóa tại `../17-policy-governance/README.md`. Module này dùng policy governance làm nguồn chuẩn và đi sâu vào tình huống thực tế.

Khác với tài liệu triển khai code, module này trả lời các câu hỏi:

- Nhà hàng casual dining vận hành thực tế như thế nào?
- Trạng thái nghiệp vụ chuyển đổi ra sao?
- Khi khách/nhân viên thao tác sai thì xử lý thế nào?
- Edge case ảnh hưởng gì tới bill, bếp, notification và audit?
- Business rule nào cần được policy bảo vệ?

## 1. Scope Nghiệp Vụ

| Nhóm | Quyết định |
|---|---|
| Mô hình | Casual dining only |
| Dòng khách | Khách ngồi tại bàn, gọi món nhiều lần trong một bữa |
| Mở bàn | Nhân viên/cashier mở bàn thủ công |
| Đặt món | Khách đặt trên màn hình bàn |
| Duyệt món | Cashier/staff duyệt trước khi xuống bếp |
| Bếp | Food/bar nhận task riêng |
| Hủy món | Có policy theo trạng thái bếp |
| Thanh toán | Cuối bữa, cashier xác nhận thủ công |
| Ngoài scope | Reservation, buffet, fast food, payment gateway |

## 2. Tài Liệu Trong Module

| File | Nội dung |
|---|---|
| [01-core-business-flow.md](01-core-business-flow.md) | Luồng nghiệp vụ end-to-end và state chính |
| [02-table-session-deep-dive.md](02-table-session-deep-dive.md) | Mở bàn, ghép bàn, chuyển bàn, edge case |
| [03-order-cancellation-deep-dive.md](03-order-cancellation-deep-dive.md) | Đặt món, duyệt order, hủy món đặt nhầm |
| [04-kitchen-fulfillment-deep-dive.md](04-kitchen-fulfillment-deep-dive.md) | Bếp/bar, task routing, issue thực tế |
| [05-billing-payment-deep-dive.md](05-billing-payment-deep-dive.md) | Bill cuối bữa, món hủy, thanh toán |
| [06-menu-inventory-recommendation-deep-dive.md](06-menu-inventory-recommendation-deep-dive.md) | Menu, sold-out, recommendation |
| [07-notification-audit-deep-dive.md](07-notification-audit-deep-dive.md) | Notification, audit, đồng bộ trạng thái |
| [08-policy-edgecase-matrix.md](08-policy-edgecase-matrix.md) | Ma trận policy + edge case tổng hợp |
| [09-edge-case-resolution-playbook.md](09-edge-case-resolution-playbook.md) | Playbook xử lý edge case theo trigger, policy, bill, notification, audit |
| [10-business-test-scenarios.md](10-business-test-scenarios.md) | Bộ scenario kiểm thử/demonstration cho nghiệp vụ và edge case |

## 3. Cách Đọc Khi Bảo Vệ Đồ Án

Nên trình bày theo thứ tự:

```text
Business flow
→ State machine
→ Business rule
→ Edge case
→ Policy xử lý
→ Ảnh hưởng bill/notification/audit
```

Điểm quan trọng cần nhấn mạnh:

- Hệ thống không chỉ CRUD món ăn/order.
- Mỗi hành động đều bị ràng buộc bởi trạng thái nghiệp vụ.
- Edge case quan trọng nhất là hủy món, bill khi bếp chưa xong, ghép/chuyển bàn, sold-out sau khi khách thêm vào cart.
- Policy layer giúp tránh viết `if else` rải rác và dễ mở rộng rule.
- Với phần bảo vệ đồ án, nên dùng `09-edge-case-resolution-playbook.md` để giải thích hướng xử lý và `10-business-test-scenarios.md` để demo/kiểm chứng.
- Khi có rule trùng/lệch giữa tài liệu, ưu tiên `17-policy-governance` làm nguồn chuẩn, sau đó cập nhật module chi tiết cho khớp.
