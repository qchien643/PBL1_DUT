# Casual Dining Ordering System - Product Design

## 1. Mục tiêu

Bộ tài liệu này là thiết kế sản phẩm cho hệ thống đặt món tại bàn chuyên sâu cho **nhà hàng Casual dining**. MVP chạy bằng nhiều cửa sổ CMD nhưng thiết kế vẫn tách rõ nghiệp vụ, database, policy và kế hoạch triển khai để sau này thay CMD bằng UI thật.

## 2. Cấu trúc tài liệu

| Module | Nội dung |
| --- | --- |
| [00-product-overview](00-product-overview/README.md) | Tổng quan hệ thống, context, module map, workflow |
| [01-restaurant-configuration](01-restaurant-configuration/README.md) | Cấu hình nhà hàng Casual dining |
| [02-console-runtime](02-console-runtime/README.md) | Runtime nhiều cửa sổ CMD |
| [03-table-session](03-table-session/README.md) | Bàn, session, ghép bàn, chuyển bàn |
| [04-menu-inventory](04-menu-inventory/README.md) | Menu, món ăn, modifier, trạng thái còn/hết |
| [05-order-management](05-order-management/README.md) | Order, duyệt order, hủy món đặt nhầm |
| [06-kitchen-fulfillment](06-kitchen-fulfillment/README.md) | Bếp/bar, task chế biến, issue |
| [07-payment-billing](07-payment-billing/README.md) | Hóa đơn và thanh toán cuối bữa |
| [08-recommendation-ai-ml](08-recommendation-ai-ml/README.md) | Đề xuất món bằng latent factor + fallback |
| [09-staff-permission](09-staff-permission/README.md) | Nhân viên, role, permission |
| [10-device-notification](10-device-notification/README.md) | CMD context, notification, polling |
| [11-reporting-audit](11-reporting-audit/README.md) | Báo cáo và audit |
| [12-database-design](12-database-design/README.md) | Thiết kế database tổng hợp |
| [13-implementation-roadmap](13-implementation-roadmap/README.md) | Roadmap triển khai code |
| [14-web-frontend-ui-ux](14-web-frontend-ui-ux/README.md) | Thiết kế frontend UI/UX bằng HTML/CSS/JS thuần |
| [15-cpp-server-api](15-cpp-server-api/README.md) | Thiết kế C++ server, REST API và notification polling |
| [16-business-deep-dive](16-business-deep-dive/README.md) | Phân tích sâu nghiệp vụ, state machine, policy và edge case |
| [17-policy-governance](17-policy-governance/README.md) | Nguồn chuẩn cho policy contract, deny code, audit, notification và test coverage |
| [code-vs-product-design-gap-report.md](code-vs-product-design-gap-report.md) | Báo cáo đối chiếu code hiện tại với thiết kế sản phẩm và các phần còn thiếu |

## 3. Cấu trúc chuẩn của module

Mỗi module nghiệp vụ có các file:

| File | Vai trò |
| --- | --- |
| `README.md` | Mục tiêu, actor, phạm vi, workflow chính |
| `business-rules.md` | Business rules và policy |
| `data-design.md` | Database tables, field, index |
| `commands-and-interfaces.md` | CMD command, service, input/output |
| `edge-cases.md` | Edge cases và cách xử lý |
| `implementation-plan.md` | Thứ tự triển khai code và tiêu chí hoàn thành |

Mọi module nghiệp vụ phải tham chiếu `17-policy-governance` khi mô tả rule, edge case, command/service và error code.

Ngoại lệ:

- `00-product-overview`: dùng `system-context`, `module-map`, `end-to-end-workflow`, `scope-and-assumptions`.
- `12-database-design`: dùng `schema-overview`, `table-groups`, `relationships`, `seed-data`.

## 4. Phạm vi chốt

| Nhóm | Quyết định |
| --- | --- |
| Loại nhà hàng | Casual dining only |
| UI MVP | Nhiều cửa sổ CMD |
| Mở bàn | Staff mở thủ công |
| Order | Khách gọi nhiều lần, staff duyệt |
| Kitchen | Food/bar task qua Kitchen CMD |
| Payment | Cuối bữa, cashier xác nhận thủ công |
| Recommendation | Latent factor theo `DiningSession x MenuItem` + fallback |
| Web UI nâng cấp | HTML/CSS/JS thuần, tách riêng khỏi server/API |
| Server nâng cấp | C++ HTTP server + REST API + notification polling |
| Ngoài scope | Buffet, cafe, fast food, reservation, payment gateway, SaaS multi-tenant |

## 5. Reading Path Khuyến Nghị

```text
00-product-overview
→ 17-policy-governance
→ 01..11 business modules
→ 12-database-design
→ 14-web-frontend-ui-ux
→ 15-cpp-server-api
→ 16-business-deep-dive
→ 13-implementation-roadmap
```

Nguyên tắc đọc: trước khi xem chi tiết từng module, đọc `17-policy-governance` để hiểu cách hệ thống ra quyết định, trả lỗi, ghi audit và gửi notification.
