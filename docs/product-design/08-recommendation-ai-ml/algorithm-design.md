# Algorithm Design - Recommendation AI/ML

Tài liệu này mô tả phần tính toán cho chức năng đề xuất món. Mục tiêu của đồ án không phải xây một hệ AI phức tạp, mà là có một mô hình toán rõ ràng, dùng được với dữ liệu nhà hàng đang có, giải thích được vì sao món được đề xuất, và vẫn tuân thủ nghiệp vụ: món hết hàng, món đã gọi, phiên bàn đang thanh toán đều phải bị loại trước khi hiển thị.

## 1. Dữ Liệu Đầu Vào

Hệ thống không cần tài khoản khách hàng. Mỗi `DiningSession` được xem như một "người dùng tạm thời" trong ma trận đề xuất.

| Ký hiệu | Dữ liệu trong hệ thống | Ý nghĩa |
| --- | --- | --- |
| `s` | `DiningSession.id` | Phiên bàn đã phục vụ hoặc đã thanh toán |
| `i, j` | `MenuItem.id` | Món trong thực đơn |
| `q_si` | `OrderItem.quantity` | Số lượng món `i` trong phiên `s` |
| `status_si` | `OrderItem.status` | Trạng thái món: đã phục vụ, hủy, từ chối, sự cố |
| `price_i` | `OrderItem.unitPrice` hoặc `MenuItem.price` | Giá snapshot tại lúc gọi món |
| `category_i` | `MenuItem.category` | Nhóm món: món ăn, đồ uống, tráng miệng |
| `station_i` | `MenuItem.station` | Khu xử lý: bếp hoặc quầy nước |
| `available_i` | `MenuItem.availabilityStatus` | Món còn bán trong ngày hay đã hết |
| `prep_i` | `MenuItem.prepMinutes` | Thời gian chuẩn bị dự kiến |
| `t_s` | thời điểm hóa đơn hoặc order | Dùng cho trọng số gần đây |
| `event_si` | `recommendation_events` | Gợi ý đã được xem, bấm, thêm vào giỏ |

Chỉ nên đưa vào tập train các phiên đã đủ tin cậy:

```text
D_train = sessions where session.status in (CLOSED, PAID-like history)
          and bill.status = PAID
          and order item status in (READY, SERVED)
```

Các món `CANCELLED`, `REJECTED`, `ISSUE_PENDING_DECISION` không tạo tín hiệu thích món. Nếu muốn dùng về sau, chúng có thể tạo tín hiệu âm nhẹ, nhưng MVP nên loại khỏi train để tránh nhiễu.

## 2. Ma Trận Tương Tác Phiên-Món

Vì khách không đánh giá sao, đây là bài toán implicit feedback: có gọi món nghĩa là có tín hiệu quan tâm.

Trọng số tương tác giữa phiên `s` và món `i`:

```text
r_si = log(1 + q_si) * w_status(status_si) * decay(t_s) + w_event(event_si)
```

Trong đó:

| Thành phần | Công thức gợi ý | Phản ứng với dữ liệu nhà hàng |
| --- | --- | --- |
| `log(1 + q_si)` | Giảm độ phóng đại khi gọi nhiều phần | Bàn gọi 5 ly trà đá không làm trà đá áp đảo toàn bộ model |
| `w_status(status_si)` | `SERVED/READY = 1`, `CANCELLED/REJECTED = 0` | Chỉ học từ món thực sự được phục vụ hoặc sẵn sàng tính tiền |
| `decay(t_s)` | `exp(-age_days / tau)` | Đơn gần đây ảnh hưởng mạnh hơn đơn quá cũ |
| `w_event(event_si)` | `shown = 0`, `clicked = 0.2`, `added = 0.5` | Nếu khách bấm hoặc thêm món gợi ý, món đó tăng tín hiệu |

Ví dụ:

```text
Khách gọi 2 phần Cơm gà nướng, đã phục vụ, hóa đơn hôm nay:
r = log(1 + 2) * 1 * exp(0) = 1.0986

Khách gọi 1 Bánh flan nhưng hủy:
r = log(2) * 0 * decay = 0
```

Từ `r_si`, tạo hai ma trận cho implicit factorization:

```text
p_si = 1 if r_si > 0 else 0
c_si = 1 + alpha * r_si
```

Trong đó:

| Ký hiệu | Ý nghĩa |
| --- | --- |
| `p_si` | Có/không có tương tác tích cực |
| `c_si` | Độ tin cậy của tương tác |
| `alpha` | Hệ số khuếch đại confidence, MVP có thể dùng `alpha = 10` |

## 3. Mô Hình Latent Factor

Mỗi phiên `s` và món `i` được biểu diễn bằng vector ẩn kích thước `k`.

```text
x_s in R^k  = vector đại diện khẩu vị phiên bàn
y_i in R^k  = vector đại diện đặc trưng ẩn của món
b_s         = bias của phiên
b_i         = bias của món
mu          = mức phổ biến trung bình
```

Điểm dự đoán cơ bản:

```text
score_lf(s, i) = mu + b_s + b_i + dot(x_s, y_i)
```

Hàm mất mát khi train:

```text
L = sum_s sum_i c_si * (p_si - mu - b_s - b_i - dot(x_s, y_i))^2
    + lambda * (||x_s||^2 + ||y_i||^2 + b_s^2 + b_i^2)
```

Ý nghĩa:

| Thành phần | Ý nghĩa nghiệp vụ |
| --- | --- |
| `dot(x_s, y_i)` | Phiên bàn có "gu" gần món đó không |
| `b_i` | Món phổ biến tự nhiên, ví dụ Trà đá thường được gọi nhiều |
| `b_s` | Một phiên có xu hướng gọi nhiều món hay ít món |
| `c_si` | Tương tác có đáng tin không |
| `lambda` | Chống học quá khớp khi dữ liệu ít |

MVP có thể train bằng SGD:

```text
e_si = p_si - predicted_si
x_s <- x_s + eta * (c_si * e_si * y_i - lambda * x_s)
y_i <- y_i + eta * (c_si * e_si * x_s - lambda * y_i)
b_s <- b_s + eta * (c_si * e_si - lambda * b_s)
b_i <- b_i + eta * (c_si * e_si - lambda * b_i)
```

Tham số gợi ý cho đồ án:

| Tham số | Giá trị MVP | Lý do |
| --- | --- | --- |
| `k` | 8 hoặc 16 | Đủ nhỏ cho dữ liệu nhà hàng, dễ chạy bằng C++ |
| `eta` | 0.01 | Learning rate dễ ổn định |
| `lambda` | 0.05 | Giảm overfit |
| `epochs` | 20-50 | Dữ liệu nhỏ, train nhanh |
| `tau` | 30 ngày | Ưu tiên xu hướng gần đây |

## 4. Vector Cho Phiên Đang Ngồi

Phiên hiện tại thường chưa có lịch sử thanh toán, nên không có `x_s` train sẵn. Ta suy ra vector phiên hiện tại từ các món đang có trong giỏ hoặc đã gọi.

```text
x_current = normalize(
    sum_i a_i * y_i / sum_i a_i
)
```

Với:

```text
a_i = 1.0 nếu món đã được thu ngân duyệt
a_i = 0.7 nếu món đang trong giỏ
a_i = 0.4 nếu món mới chỉ được bấm xem từ gợi ý
```

Ý nghĩa:

| Tín hiệu hiện tại | Tác động |
| --- | --- |
| Khách chọn món bò | Vector phiên nghiêng về các món thường đi kèm món bò |
| Khách chọn đồ ăn nhưng chưa chọn đồ uống | Model có thể gợi ý nước phù hợp |
| Khách đã gọi món tráng miệng | Hệ thống không gợi ý lại món đó |

Nếu giỏ và order đều rỗng, không tính được `x_current`; hệ thống chuyển sang fallback.

## 5. Công Thức Chấm Điểm Hybrid

Sau khi có `x_current`, mỗi món ứng viên `j` được chấm điểm:

```text
score(j) =
    w_lf   * dot(x_current, y_j)
  + w_pop  * popularity(j)
  + w_pair * pair_score(C, j)
  + w_cat  * category_boost(C, j)
  + w_time * time_boost(j, now)
  + w_bus  * business_boost(j)
  - w_prep * prep_penalty(j)
  - w_dup  * duplicate_penalty(C, j)
```

Trong đó `C` là tập món hiện có trong giỏ hoặc order của phiên hiện tại.

### 5.1. Độ Phổ Biến

```text
popularity(j) = log(1 + served_count_j) / log(1 + max_served_count)
```

Phản ứng với dữ liệu:

- Món bán chạy có điểm nền tốt hơn.
- Dùng log để món quá phổ biến không áp đảo tất cả.
- Chỉ tính `served_count`, không tính món bị hủy hoặc từ chối.

### 5.2. Món Thường Đi Kèm

Đếm số phiên đã gọi cả món `i` và món `j`:

```text
cooc(i, j) = count(session contains i and j)
support(i) = count(session contains i)
support(j) = count(session contains j)

confidence(i -> j) = cooc(i, j) / support(i)
lift(i, j) = confidence(i -> j) / (support(j) / total_sessions)
```

Điểm pair cho cả giỏ:

```text
pair_score(C, j) = max_i_in_C log(1 + cooc(i, j)) * lift(i, j)
```

Ý nghĩa:

- Nếu nhiều bàn gọi Cơm gà nướng cùng Nước cam, cặp này có `cooc` cao.
- Nếu Nước cam vốn đã rất phổ biến, `lift` giúp phân biệt "phổ biến chung" với "thật sự đi kèm".

### 5.3. Tăng Điểm Theo Nhóm Món

```text
category_boost(C, j) =
    +1 nếu C có món ăn nhưng chưa có đồ uống và j là đồ uống
    +0.5 nếu C có món chính nhưng chưa có tráng miệng và j là tráng miệng
    -1 nếu j cùng nhóm với quá nhiều món đã chọn
```

Ý nghĩa:

- Nhà hàng casual dining thường muốn gợi ý thêm đồ uống hoặc tráng miệng.
- Khách gọi toàn món chính thì gợi ý thêm nước hợp lý hơn gợi ý thêm một món chính tương tự.

### 5.4. Yếu Tố Thời Điểm

```text
time_boost(j, now) =
    average_orders(j, same_hour_or_meal_period) / average_orders(j, all_day)
```

Ví dụ:

- Buổi trưa: món cơm có thể được tăng nhẹ.
- Buổi tối: món ăn kèm hoặc tráng miệng có thể được tăng.
- MVP chưa cần dữ liệu giờ quá chi tiết; có thể chia `lunch`, `dinner`, `other`.

### 5.5. Yếu Tố Vận Hành Nhà Hàng

```text
business_boost(j) =
    margin_norm(j) * w_margin
  + stock_priority(j) * w_stock
```

Các yếu tố có thể thêm khi có dữ liệu:

| Yếu tố | Dữ liệu cần có | Cách dùng |
| --- | --- | --- |
| Lợi nhuận biên | cost hoặc margin của món | Ưu tiên món có biên lợi nhuận tốt |
| Nguyên liệu cần đẩy | tồn kho/ngày hết hạn | Tăng điểm món dùng nguyên liệu sắp hết hạn |
| Tải bếp | số task đang pending theo station | Giảm món làm lâu nếu bếp đang quá tải |
| Thời gian phục vụ | `prepMinutes` | Không gợi ý món lâu khi khách sắp thanh toán |
| Giờ ăn | thời điểm order | Gợi ý món phù hợp bữa trưa/tối |
| Combo thủ công | rule do quản lý cấu hình | Đảm bảo gợi ý dễ giải thích khi dữ liệu ít |

### 5.6. Phạt Món Trùng Và Món Lâu

```text
duplicate_penalty(C, j) =
    1 nếu j đã có trong C
    0 nếu chưa có

prep_penalty(j) = prepMinutes_j / maxPrepMinutes
```

Trước khi chấm điểm vẫn phải filter:

```text
candidate_j is valid if:
    catalogStatus_j = ACTIVE
    availabilityStatus_j = AVAILABLE
    j not in current cart/order
    session.status = ACTIVE
```

## 6. Trọng Số MVP

Trọng số ban đầu nên đơn giản để dễ bảo vệ:

| Trọng số | Giá trị gợi ý | Vai trò |
| --- | --- | --- |
| `w_lf` | 0.45 | Khẩu vị ẩn từ latent factor |
| `w_pop` | 0.20 | Món phổ biến |
| `w_pair` | 0.20 | Món thường đi kèm |
| `w_cat` | 0.10 | Cân bằng nhóm món |
| `w_time` | 0.05 | Phù hợp thời điểm |
| `w_bus` | 0.00-0.10 | Bật khi có dữ liệu margin/tồn kho |
| `w_prep` | 0.05 | Giảm món làm lâu |
| `w_dup` | rất lớn | Loại món đã có trong giỏ/order |

MVP có thể chuẩn hóa điểm mỗi thành phần về `[0, 1]` trước khi cộng, để tránh thành phần có thang đo lớn áp đảo.

## 7. Fallback Khi Dữ Liệu Ít

Hệ thống dùng thứ tự sau:

```text
1. Latent factor + hybrid score
2. Item pair rule nếu giỏ/order có món
3. Best seller theo món đã phục vụ
4. Best seller theo nhóm món còn thiếu
5. Không hiển thị gợi ý nếu mọi candidate bị filter
```

Điều kiện chuyển fallback:

| Tình huống | Cách xử lý |
| --- | --- |
| Chưa có model active | Best seller hoặc item pair |
| Món mới chưa có vector | Dùng category/popularity fallback |
| Phiên chưa gọi món nào | Best seller theo thời điểm |
| Tất cả món gợi ý đã hết | Loại hết, trả danh sách rỗng |
| Session đã yêu cầu hóa đơn | Không gợi ý thêm món |

## 8. Giải Thích Kết Quả Cho Người Dùng

Mỗi recommendation nên trả thêm `reason`:

| Điều kiện nổi bật | Reason |
| --- | --- |
| `pair_score` cao | "Thường được gọi cùng món bạn đã chọn" |
| `category_boost` cao | "Bàn chưa có đồ uống/tráng miệng" |
| `popularity` cao | "Món được nhiều bàn gọi" |
| `time_boost` cao | "Phù hợp thời điểm hiện tại" |
| fallback | "Gợi ý phổ biến hôm nay" |

Không nên hiển thị công thức toán cho khách. Công thức dùng trong thiết kế và kiểm thử; UI chỉ cần reason ngắn, dễ hiểu.

## 9. Vì Sao Chọn Giải Pháp Này

| Lựa chọn | Lý do phù hợp đồ án |
| --- | --- |
| `DiningSession x MenuItem` thay vì `Customer x MenuItem` | MVP không cần tài khoản khách, vẫn có dữ liệu đủ để tính |
| Implicit feedback | Nhà hàng có order, số lượng, thanh toán; không có rating sao |
| Latent factor | Có công thức toán rõ, thể hiện được quan hệ ẩn giữa món |
| Hybrid score | Kết hợp ML với rule nghiệp vụ để tránh gợi ý vô lý |
| Fallback rule-based | Dữ liệu nhà hàng ban đầu ít vẫn hoạt động |
| Filter theo policy sau cùng | Không bao giờ đề xuất món hết hoặc món không được bán |
| Reason generation | Dễ giải thích khi bảo vệ và dễ kiểm thử |

So với deep learning, cách này nhẹ hơn, dễ chạy bằng C++, dễ debug, đủ tốt cho quy mô MVP. So với best-seller thuần, latent factor và pair rule phản ứng tốt hơn với giỏ món hiện tại.

## 10. Đánh Giá Mô Hình

Offline evaluation:

```text
Với mỗi session đã thanh toán:
    ẩn 1 món cuối cùng làm ground truth
    dùng các món còn lại làm context
    recommend top K
    kiểm tra món bị ẩn có nằm trong top K không
```

Chỉ số:

| Metric | Công thức | Ý nghĩa |
| --- | --- | --- |
| `Precision@K` | số món đúng trong top K / K | Gợi ý top K có chính xác không |
| `Recall@K` | số món đúng trong top K / số món bị ẩn | Có tìm lại được món khách thật sự gọi không |
| `HitRate@K` | 1 nếu món đúng nằm trong top K | Dễ trình bày trong demo |
| `Coverage` | số món từng được recommend / tổng số món | Tránh chỉ gợi ý vài món bán chạy |
| `AddToCartRate` | số lần thêm từ gợi ý / số lần gợi ý được xem | Đánh giá online trong app |

MVP acceptance:

```text
HitRate@5 tốt hơn best-seller fallback
Không recommend món SOLD_OUT
Không recommend món đã có trong cart/order
Có reason cho từng món gợi ý
```

## 11. Pseudocode

```text
recommend(sessionId, topN):
    session = loadSession(sessionId)
    if session.status != ACTIVE:
        return []

    contextItems = cartItems(session) + acceptedOrderItems(session)
    candidates = activeAvailableMenuItems()
    candidates = candidates - contextItems

    if activeModel exists and contextItems not empty:
        x = weightedAverage(itemVector[i] for i in contextItems)
        for candidate in candidates:
            score[candidate] = hybridScore(x, contextItems, candidate)
        strategy = "LATENT_FACTOR_HYBRID"
    else:
        score = fallbackScore(contextItems, candidates)
        strategy = "RULE_BASED_FALLBACK"

    ranked = sortByScoreDescending(candidates)
    return topN with reason and strategy
```

## 12. Dữ Liệu Có Thể Bổ Sung Sau MVP

| Dữ liệu bổ sung | Lợi ích |
| --- | --- |
| `ingredient_stock` | Gợi ý món phù hợp tồn kho |
| `food_cost` hoặc `margin` | Ưu tiên món có lợi nhuận tốt |
| `prep_queue_length` | Tránh gợi ý món làm lâu khi bếp đang quá tải |
| `meal_period` | Gợi ý khác nhau theo trưa/tối |
| `table_size` | Gợi ý phần ăn hoặc combo theo số khách |
| `manual_combo_rules` | Cho quản lý cài cặp món cần đẩy |
| `recommendation_click/add events` | Học từ phản hồi thật của khách |
