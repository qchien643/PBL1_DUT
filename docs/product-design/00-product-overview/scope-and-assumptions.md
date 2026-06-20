# Scope And Assumptions

## 1. In scope

| Nhóm | Nội dung |
| --- | --- |
| Runtime | Nhiều cửa sổ CMD |
| Nhà hàng | Casual dining, một chi nhánh |
| Table | Mở bàn, ghép bàn, chuyển bàn |
| Order | Submit nhiều lần, staff approval, hủy món đặt nhầm |
| Kitchen | Food/bar task qua CMD |
| Payment | Cuối bữa, cashier confirm |
| Recommendation | Latent factor + fallback |
| Reporting | Doanh thu, món bán chạy, order cancelled |
| Audit | Log action quan trọng |

## 2. Out of scope

| Nhóm | Lý do |
| --- | --- |
| Buffet/cafe/fast food | Sản phẩm chốt Casual dining only |
| Reservation | Không cần cho MVP đặt món tại bàn |
| Payment gateway/POS | Không đủ phạm vi đồ án |
| Printer thật | Kitchen CMD thay thế |
| Web/tablet UI | CMD thay thế |
| Split bill/refund | Tăng phức tạp billing |
| Customer account/loyalty | Recommendation dùng `DiningSession` |

## 3. Assumptions

- Một `DiningSession` là một lượt khách đang ăn tại bàn hoặc nhóm bàn.
- Một session có một bill cuối bữa.
- Staff phải duyệt order trước khi bếp làm.
- Món hết không được order và không được recommend.
- Hủy món đặt nhầm được xử lý theo trạng thái bếp.
- CMD không chứa nghiệp vụ; mọi rule đi qua service/policy.
