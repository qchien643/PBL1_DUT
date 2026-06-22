# Menu, Inventory And Recommendation Deep Dive

## 1. Menu Status

| Trạng thái | Ý nghĩa |
|---|---|
| `catalogStatus` | Món có thuộc menu không: `ACTIVE`, `HIDDEN` |
| `availabilityStatus` | Hôm nay còn bán không: `AVAILABLE`, `SOLD_OUT` |

```text
HIDDEN = không còn xuất hiện trong menu
SOLD_OUT = vẫn là món trong menu nhưng hôm nay hết
```

## 2. Business Rules

| Rule | Lý do |
|---|---|
| Customer chỉ thấy item `ACTIVE` | Không hiển thị món ngừng bán |
| Customer chỉ order item `AVAILABLE` | Không bán món hết |
| Manager/cashier có thể thấy sold-out | Để giải thích cho khách |
| Availability kiểm tra lại khi submit/accept | Món có thể hết sau khi khách thêm cart |
| Sold-out không tự hủy order đã accepted | Bếp vẫn phải xử lý món đã nhận nếu nguyên liệu đã cấp |

## 3. Edge Cases Menu/Inventory

| Edge case | Tình huống | Xử lý | Notification |
|---|---|---|---|
| Món sold-out sau khi khách thêm cart | Customer add lúc còn, submit lúc hết | Reject item khi submit/accept | Customer thấy item unavailable |
| Manager set sold-out khi order pending | Order chưa accepted | Cashier accept sẽ reject item sold-out | Cashier/customer |
| Manager set sold-out khi task preparing | Món đã xuống bếp | Không ảnh hưởng task đang làm | Không hoặc manager audit |
| Menu item hidden nhưng còn trong order cũ | Historical order vẫn hiển thị tên item | Không xóa dữ liệu cũ | Không |
| Giá thay đổi giữa session | MVP nên dùng `unitPrice` tại lúc order | Bill không bị đổi theo giá mới | Audit price change nếu có |

## 4. Recommendation Latent Factor MVP

Recommendation không thay thế business rule. Nó chỉ đề xuất món orderable.

```text
Candidate items
→ filter ACTIVE + AVAILABLE
→ exclude already ordered items
→ score by latent factor
→ fallback best-seller/category
```

Thiết kế thuật toán chi tiết nằm ở `../08-recommendation-ai-ml/algorithm-design.md`. Công thức tổng quan:

```text
score(j) =
    w_lf   * dot(x_current, y_j)
  + w_pop  * popularity(j)
  + w_pair * pair_score(C, j)
  + w_cat  * category_boost(C, j)
  + w_time * time_boost(j, now)
  - w_prep * prep_penalty(j)
```

Trong đó `C` là các món khách đã chọn/gọi trong phiên hiện tại. `x_current` là vector khẩu vị suy ra từ `C`, còn `y_j` là vector ẩn của món ứng viên `j`. Các thành phần `popularity`, `pair_score`, `category_boost`, `time_boost` giúp mô hình phản ứng với dữ liệu thật của nhà hàng như món bán chạy, món thường đi kèm, nhóm món còn thiếu, thời điểm bán tốt và thời gian chuẩn bị.

Vì MVP không có tài khoản khách hàng và không có đánh giá sao, hệ thống dùng implicit feedback:

```text
r_si = log(1 + quantity) * statusWeight * recencyDecay + eventWeight
```

Món đã phục vụ tạo tín hiệu tích cực. Món `CANCELLED`, `REJECTED`, `ISSUE_PENDING_DECISION` không được xem là khách thích món đó.

## 5. Recommendation Edge Cases

| Edge case | Tình huống | Xử lý |
|---|---|---|
| Session chưa order gì | Không có vector session | Fallback best-seller/category |
| Tất cả món đề xuất sold-out | Filter hết candidate | Hiển thị “No recommendation” |
| Khách đã gọi món đó rồi | Candidate trùng món đã gọi | Exclude hoặc giảm score |
| Item cancelled | Có nên dùng làm signal? | MVP: không dùng cancelled/rejected |
| Dữ liệu lịch sử ít | Latent factor chưa mạnh | Dùng rule-based fallback |

## 6. Điểm Cần Nhấn Khi Bảo Vệ

- Recommendation không cần biết danh tính khách ở MVP.
- Dùng session hiện tại thay cho user history.
- Luôn filter business rule trước khi recommend.
- AI/ML chỉ hỗ trợ bán hàng, không phá rule vận hành.
