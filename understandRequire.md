# Bóc tách yêu cầu báo cáo đồ án

## 1. Nguồn tài liệu đã đọc

- Mẫu báo cáo: `docs/report/Mẫu báo cáo đồ án của GVHD (2).docx`.
- Thiết kế sản phẩm tổng quan: `docs/product-design/README.md`, `00-product-overview/*`.
- Thiết kế nghiệp vụ chính: `03-table-session`, `04-menu-inventory`, `05-order-management`, `06-kitchen-fulfillment`, `07-payment-billing`, `08-recommendation-ai-ml`, `09-staff-permission`, `10-device-notification`, `11-reporting-audit`.
- Thiết kế dữ liệu: `12-database-design/*`.
- Kiến trúc triển khai: `13-implementation-roadmap/code-architecture-map.md`, `15-cpp-server-api`, `14-web-frontend-ui-ux`.
- Policy governance: `17-policy-governance/*`.
- Thông tin code hiện tại: `README.md`, `CMakeLists.txt`, `src/modules/recommendation_ai_ml/*`, `docs/product-design/code-vs-product-design-gap-report.md`.

## 2. Đề tài cần báo cáo

Tên đề tài dự kiến:

**Xây dựng ứng dụng đặt món ăn trong nhà hàng sử dụng thuật toán đề xuất**

Định hướng báo cáo:

- Đây là đồ án lập trình tính toán, nên cần nhấn mạnh bài toán, dữ liệu đầu vào/đầu ra, cấu trúc dữ liệu và thuật toán.
- Dự án thuộc ngữ cảnh nhà hàng casual dining, một chi nhánh, khách ngồi tại bàn, gọi món nhiều lần trong một `DiningSession`.
- Chương trình chạy được bằng nhiều cửa sổ CMD và có hướng nâng cấp Web UI + C++ server/API.
- Thuật toán đề xuất trong product-design là hybrid recommendation: latent factor theo ma trận `DiningSession x MenuItem` kết hợp popularity, item-pair rule, category/time boost và rule-based fallback.
- Chương trình hiện tại có C++17, CMake, file database `.txt`, nhiều module nghiệp vụ, server/web demo và recommendation dạng vector heuristic mô phỏng latent factor/fallback. Khi viết báo cáo nên trình bày thuật toán theo thiết kế chuẩn, đồng thời ghi nhận mức cài đặt hiện có ở phần kết quả/nhận xét.

## 3. Mục lục mẫu cần hoàn thành

Mục lục trích từ mẫu báo cáo:

```text
MỞ ĐẦU
1. TỔNG QUAN ĐỀ TÀI
2. CƠ SỞ LÝ THUYẾT
   2.1. Ý tưởng
   2.2. Cơ sở lý thuyết
3. TỔ CHỨC CẤU TRÚC DỮ LIỆU VÀ THUẬT TOÁN
   3.1. Phát biểu bài toán
   3.2. Cấu trúc dữ liệu
   3.3. Thuật toán
4. CHƯƠNG TRÌNH VÀ KẾT QUẢ
   4.1. Tổ chức chương trình
   4.2. Ngôn ngữ cài đặt
   4.3. Kết quả
      4.3.1. Giao diện chính của chương trình
      4.3.2. Kết quả thực thi của chương trình
      4.3.3. Nhận xét đánh giá
5. KẾT LUẬN VÀ HƯỚNG PHÁT TRIỂN
   5.1. Kết luận
   5.2. Hướng phát triển
TÀI LIỆU THAM KHẢO
PHỤ LỤC
```

## 4. Nội dung cần đưa vào từng mục

### MỞ ĐẦU

Cần trình bày:

- Lý do chọn đề tài: nhà hàng cần số hóa quy trình gọi món, giảm sai sót, rút ngắn thời gian phục vụ, hỗ trợ nhân viên/bếp/quản lý.
- Mục tiêu: xây dựng ứng dụng đặt món tại bàn, quản lý phiên bàn, menu, order, bếp, thanh toán và gợi ý món.
- Đối tượng nghiên cứu: quy trình casual dining, dữ liệu order, phiên ăn, món ăn, trạng thái bếp/thanh toán, thuật toán đề xuất.
- Phạm vi: một chi nhánh, casual dining, CMD/Web, thanh toán thủ công, không tích hợp payment gateway/reservation/printer thật.
- Phương pháp: phân tích nghiệp vụ, thiết kế module, thiết kế dữ liệu, xây dựng thuật toán đề xuất, cài đặt C++17, kiểm thử theo luồng demo và edge case.
- Cấu trúc báo cáo: 5 chương theo mẫu.

### 1. TỔNG QUAN ĐỀ TÀI

Nội dung chính:

- Mô tả hệ thống đặt món trong nhà hàng casual dining.
- Actor: Customer, Cashier/Staff, Waiter, Kitchen/Bar, Manager, System.
- Luồng tổng quát:
  `OpenTable -> ViewMenu -> ViewRecommendations -> SubmitOrder -> AcceptOrder -> KitchenTask -> Ready/Served -> RequestBill -> ConfirmPayment -> CloseSession`.
- Các module:
  `table_session`, `menu_inventory`, `order_management`, `kitchen_fulfillment`, `payment_billing`, `recommendation_ai_ml`, `reporting_audit`, `policies`, `infrastructure`, `console`, `server`, `web`.
- Giá trị của thuật toán đề xuất: tăng trải nghiệm khách, gợi ý đồ uống/tráng miệng/món đi kèm, vẫn tuân thủ inventory và trạng thái session.

### 2. CƠ SỞ LÝ THUYẾT

#### 2.1. Ý tưởng

Ý tưởng chính:

- Mỗi lượt khách tại bàn là một `DiningSession`.
- Khách không cần tài khoản, hệ thống vẫn học từ lịch sử session và món đã gọi.
- Dữ liệu order là implicit feedback: món được gọi/đã phục vụ là tín hiệu quan tâm.
- Gợi ý món dựa trên giỏ/order hiện tại, lịch sử món bán chạy và quan hệ món thường đi kèm.
- Hệ thống dùng policy để chặn thao tác không hợp lệ: món hết, session đang thanh toán, order chưa duyệt, bill còn blocker.

#### 2.2. Cơ sở lý thuyết

Cần trình bày ngắn gọn các khái niệm:

- Hệ gợi ý: content-based, collaborative filtering, hybrid recommendation.
- Implicit feedback: dùng hành vi gọi món thay cho rating sao.
- Matrix factorization/latent factor:
  `DiningSession x MenuItem`, vector ẩn `x_s`, `y_i`, điểm `dot(x_s, y_i)`.
- Confidence matrix:
  `p_si = 1 if r_si > 0 else 0`, `c_si = 1 + alpha * r_si`.
- Hybrid score:
  latent factor + popularity + item-pair + category/time boost - prep/duplicate penalty.
- Rule-based fallback khi chưa có model hoặc chưa có context.
- Policy layer và layered architecture: UI/console chỉ gửi intent; service/policy quyết định nghiệp vụ; database lưu state.

### 3. TỔ CHỨC CẤU TRÚC DỮ LIỆU VÀ THUẬT TOÁN

#### 3.1. Phát biểu bài toán

Input:

- Thông tin bàn/session: `sessionId`, `tableId`, `guestCount`, `status`.
- Menu: `menuItemId`, tên món, category, price, station, `catalogStatus`, `availabilityStatus`, thời gian chuẩn bị.
- Cart/order hiện tại: danh sách món, số lượng, trạng thái order item.
- Lịch sử order/session đã thanh toán.
- Event recommendation: shown/clicked/added nếu có.

Output:

- Danh sách món có thể order.
- Order được submit/duyệt/từ chối/hủy theo trạng thái hợp lệ.
- Task bếp/bar.
- Bill cuối bữa.
- Top N món gợi ý kèm điểm và lý do.
- Báo cáo/audit/notification.

Ràng buộc:

- Chỉ session `ACTIVE` mới được order/recommend.
- Món phải `ACTIVE` và `AVAILABLE`.
- Không gợi ý món đã có trong cart/order.
- Order phải được staff duyệt trước khi xuống bếp.
- Bill chỉ tạo/thanh toán khi không còn blocker.

#### 3.2. Cấu trúc dữ liệu

Nhóm dữ liệu từ product-design:

- Table/session: `dining_tables`, `dining_sessions`, `dining_session_tables`.
- Menu/inventory: `menu_items`, `item_availability`, category/modifier.
- Order: `order_headers`, `order_items`, `cancellation_requests`.
- Kitchen: `preparation_tasks`, `task_items`.
- Billing: `bills`, `bill_lines`, `payments`.
- Recommendation: `recommendation_interactions`, `recommendation_models`, `item_latent_factors`, `session_latent_factors`, `recommendation_events`, `item_pair_rules`.
- Staff/permission: `staff_users`, `roles`, `permissions`.
- Governance: `notifications`, `audit_events`.

Chương trình hiện tại dùng:

- `FileDatabase` facade đọc/ghi nhiều bảng file `.txt`.
- Domain records trong `src/domain/models.hpp`.
- Module service theo nghiệp vụ.
- Policy trong `src/policies/business_policies.*`.
- Recommendation service dùng vector yếu tố món và điểm số để mô phỏng gợi ý.

#### 3.3. Thuật toán

Các thuật toán cần trình bày:

- Thuật toán mở bàn và tạo session.
- Thuật toán submit/accept order.
- Thuật toán route task bếp/bar.
- Thuật toán tạo bill và xác nhận thanh toán.
- Thuật toán đề xuất món:
  1. Load session và context món hiện tại.
  2. Lọc candidate theo policy.
  3. Nếu có model/context: tính `x_current`, chấm hybrid score.
  4. Nếu không: fallback theo item-pair/best-seller/category.
  5. Sắp xếp giảm dần, lấy Top N, sinh reason.
- Phân tích độ phức tạp:
  với `M` món, `C` món trong context, `k` chiều vector, scoring khoảng `O(M*k + M*C)` nếu có pair score; fallback best-seller khoảng `O(H + M log M)` tùy cách tổng hợp lịch sử.

### 4. CHƯƠNG TRÌNH VÀ KẾT QUẢ

#### 4.1. Tổ chức chương trình

Từ repo hiện tại:

- `src/domain`: mô hình dữ liệu/trạng thái.
- `src/shared`: tiện ích chung.
- `src/infrastructure`: lưu trữ file database.
- `src/policies`: business rule/policy.
- `src/modules`: các service nghiệp vụ.
- `src/console`: màn hình CMD cho manager/cashier/customer/kitchen.
- `src/server`: C++ HTTP server/API.
- `web`: HTML/CSS/JS cho customer, cashier, kitchen/bar, manager.
- `scripts`: build/run/demo/reset.

#### 4.2. Ngôn ngữ cài đặt

- C++17 cho backend/core logic.
- CMake để build.
- HTML/CSS/JavaScript thuần cho Web UI.
- File `.txt` theo bảng trong `data/db/` làm cơ chế lưu trữ.
- Không dùng thư viện ngoài phức tạp để dễ chạy trong môi trường học phần.

#### 4.3. Kết quả

Cần mô tả:

- Luồng demo chính chạy được.
- Giao diện/role chính: cashier, customer, kitchen/bar, manager, web index.
- Kết quả thực thi:
  mở bàn, xem menu, nhận gợi ý, submit order, duyệt order, bếp xử lý, request bill, confirm payment, xem audit/report.
- Kiểm thử nghiệp vụ P0:
  idempotency submit, sold-out, replacement, kitchen issue, bill stale, payment amount, permission tối thiểu theo README hiện tại.
- Nhận xét:
  hệ thống đáp ứng luồng nghiệp vụ chính; thuật toán đề xuất trong code hiện là bản triển khai theo hướng vector/fallback và có thể nâng cấp thêm theo latent factor đầy đủ; policy governance/data schema còn có thể nâng cấp.

### 5. KẾT LUẬN VÀ HƯỚNG PHÁT TRIỂN

Kết luận:

- Hoàn thành thiết kế và cài đặt ứng dụng đặt món nhà hàng trong phạm vi đồ án.
- Tách module rõ, có policy, có recommendation, có lưu trữ, có console/web/server.
- Hệ thống minh họa được quy trình nghiệp vụ end-to-end.

Hướng phát triển:

- Hoàn thiện latent factor training thật với bảng model/vector/interactions.
- Nâng cấp database sang SQLite/PostgreSQL.
- Chuẩn hóa `PolicyDecision`, deny code, audit/notification.
- Bổ sung authentication/authorization.
- Cải thiện Web UI, realtime notification, payment gateway, printer, thống kê nâng cao.

### TÀI LIỆU THAM KHẢO

Nên liệt kê:

- Tài liệu product-design trong repo.
- README dự án.
- Tài liệu về recommender systems/matrix factorization/implicit feedback.
- Tài liệu C++17/CMake nếu cần.

### PHỤ LỤC

Nên đặt:

- Lệnh build/run.
- Cấu trúc thư mục.
- Pseudocode thuật toán đề xuất.
- Một số đoạn code minh họa nếu giáo viên yêu cầu.
