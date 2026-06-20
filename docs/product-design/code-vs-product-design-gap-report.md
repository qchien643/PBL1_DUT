# Code vs Product Design Gap Report

Ngày kiểm tra: 2026-06-20

Phạm vi kiểm tra:

- Source C++ trong `src/`
- Web UI trong `web/`
- Script chạy demo trong `scripts/`
- Đối chiếu với toàn bộ `docs/product-design/`

Kết quả build:

```text
cmake --build build-mingw
[100%] Built target restaurant_mvp
```

## 1. Tóm Tắt Nhanh

Code hiện tại đã có **MVP chạy được happy path**:

```text
open table
→ customer submit order
→ cashier accept
→ kitchen/bar start + ready
→ create bill
→ confirm payment
```

Tuy nhiên, so với `product-design`, code hiện tại mới đạt mức **prototype nghiệp vụ**, chưa đạt mức **policy-driven product**.

Thiếu lớn nhất:

- Chưa có `PolicyDecision` chuẩn; policy hiện tại chỉ trả `bool`.
- Chưa có deny code, `requiredAction`, `context`, `correlationId`.
- Chưa có permission/role/staff thật.
- Chưa có kitchen issue flow.
- Chưa có bill version/stale/reopen/bill lines/payment amount.
- Chưa có order idempotency dù frontend có gửi `idempotencyKey`.
- Chưa xử lý đúng edge case sold-out khi cashier accept order; code hiện tại vẫn partial accept/reject item.
- Database file hiện tại còn rất mỏng so với schema trong design.

## 2. Bảng Đối Chiếu Theo Module

| Module | Hiện trạng code | Thiếu so với product-design | Ưu tiên |
|---|---|---|---|
| `00-product-overview` | Có kiến trúc module C++ tách `modules/`, `policies/`, `server/`, `console/`. | Chưa có workflow governance xuyên suốt; policy/audit/notification chưa chạy thống nhất qua mọi command. | P1 |
| `01-restaurant-configuration` | Chưa có module code riêng; config đang hard-code qua seed data và vài trạng thái. | Thiếu `branch_configs`, `feature_flags`, `configVersion`, `RestaurantProfilePolicy`, `FeatureFlagPolicy`, config snapshot vào session/order/bill. | P1 |
| `02-console-runtime` | Có CMD cho manager/cashier/customer/kitchen và script chạy. | CMD không có notification panel, không có `ConsoleActorContextPolicy`, không có context version, không có command dispatcher thống nhất, role/context đang hard-code. | P1 |
| `03-table-session` | Có mở bàn, ghép bàn, chuyển bàn, mark cleaned. | Chưa chặn merge/transfer khi session `BILL_REQUESTED`; session phụ khi merge đang `CLOSED` thay vì `MERGED`; thiếu guest count, reason, session version, table history, notification nhất quán trong CMD. | P0/P1 |
| `04-menu-inventory` | Có menu item, catalog status, availability `AVAILABLE/SOLD_OUT`, manager đổi sold-out. | Thiếu category/modifier thật, sold-out reason, branch-specific availability, price version/snapshot policy, hidden/archived lifecycle, confirmation khi giá đổi. | P1 |
| `05-order-management` | Có submit, accept, reject, request cancel, approve cancel. | Thiếu idempotency thật; thiếu `NEEDS_CUSTOMER_CONFIRMATION`; sold-out tại accept đang reject item và partial accept phần còn lại; thiếu replace item, cancel reason, manager override, policy deny code. | P0 |
| `06-kitchen-fulfillment` | Có route task theo station, start task, mark ready. | Thiếu `reportIssue`, `ISSUE`, `SERVED`, `DELAYED`, station routing config, wrong item/wrong table issue, cancel/start race handling, ready-to-served policy. | P0 |
| `07-payment-billing` | Có create bill, tính tổng, confirm payment, đóng session/table cleaning. | Thiếu `bill_lines`, bill snapshot/version, `STALE`, reopen bill, payment amount validation, paid amount/change, payment idempotency, bill blocker list, adjustment/void. | P0 |
| `08-recommendation-ai-ml` | Có gợi ý bằng vector heuristic trong code và fallback theo sold quantity. | Chưa phải latent factor thật; thiếu `recommendation_models`, `item_latent_factors`, training/activation, interaction history chuẩn, model fallback policy, eligibility policy khi billing. | P1 |
| `09-staff-permission` | Actor chỉ là string như `"cashier"`, `"manager"`, `"customer"`. | Thiếu staff users, roles, permissions, actor scope, manager override approval, role safety policy. Mọi API hiện gần như không auth/authorize thật. | P0 |
| `10-device-notification` | Server có notification polling theo channel; web polling mỗi 1.5s. | CMD không nhận notification; notification không có recipient/read state thật; thiếu server-side dedup, resourceVersion, recovery policy, route policy; console actions không emit notification. | P1 |
| `11-reporting-audit` | Có audit message đơn giản và paid revenue summary. | Audit thiếu severity, actorId, reason, before/after, correlationId; report thiếu top-selling, cancelled filter rõ ràng, session merge reporting, audit immutability. | P1 |
| `12-database-design` | Có file database text với tables, sessions, menu, orders, tasks, bills, audit, notifications. | Thiếu nhiều bảng/field thiết kế: config, roles, bill_lines đầy đủ, payments riêng, status history, idempotency, notification recipients, recommendation model/vector, modifiers, issue records, version fields. | P0 |
| `13-implementation-roadmap` | Code đã tách module và build được. | Target policy layer chưa tách theo nhóm; chưa có test suite; chưa có `PolicyDecision`; nhiều phase business hardening chưa triển khai. | P1 |
| `14-web-frontend-ui-ux` | Có web UI customer/cashier/kitchen/manager, polling notification. | UX chưa dùng `requiredAction`; thiếu modal sold-out/customer decision; thiếu blocker list khi bill fail; API errors chỉ toast message; chưa có role/session auth UI. | P1 |
| `15-cpp-server-api` | Có C++ HTTP server, REST-ish endpoints, static web serving, mutex khi xử lý request. | Error envelope chưa theo governance; thiếu `requiredAction/context/correlationId`; thiếu endpoint issue kitchen, served, reopen bill, resolve customer confirmation, config/staff/role, idempotency. | P0/P1 |
| `16-business-deep-dive` | Một số edge case đã có trong code: bàn chưa mở, hủy pending, bill chặn pending/preparing. | Nhiều edge case chưa đúng/chưa có: EC-06 sold-out accept, kitchen issue, bill stale, READY vs SERVED, manager override, bill reopen, payment amount. | P0 |
| `17-policy-governance` | Chỉ có `business_policies.*` với vài hàm `bool`. | Chưa có policy contract trong code, chưa có deny code, conflict resolution, audit/notification mapping, `PolicyDecision`, policy test matrix triển khai. | P0 |

## 3. Các Gap P0 Cần Xử Lý Trước

| Gap | Vì sao P0 | Code hiện tại |
|---|---|---|
| Thiếu `PolicyDecision` chuẩn | Không thể map deny code/API/UI/audit như design | `src/policies/business_policies.hpp` chỉ có hàm `bool` |
| Sold-out khi cashier accept xử lý sai design | Design yêu cầu hỏi lại khách, không partial accept mặc định | `src/modules/order_management/order_management_service.cpp` reject item unavailable rồi accept item còn lại |
| Không có permission/role thật | Mọi nghiệp vụ nhạy cảm như payment/cancel/config chưa được bảo vệ | Actor đang truyền string trong console/server |
| Không có idempotency | Double submit/pay có thể tạo order/bill/action lặp | Frontend gửi `idempotencyKey` nhưng backend không lưu/kiểm tra |
| Không có kitchen issue flow | Không xử lý được bếp báo hết nguyên liệu/sai món/lỗi chất lượng | Kitchen service chỉ có start/ready |
| Billing thiếu snapshot/version/stale | Có thể thanh toán bill lỗi thời sau khi order thay đổi | `BillRecord` chỉ có `id/sessionId/total/status/paymentMethod` |
| Payment thiếu amount validation | Cashier có thể confirm payment không cần số tiền | `confirmPayment` chỉ nhận `paymentMethod` |
| Database thiếu state/version/history | Policy không có đủ dữ liệu để quyết định/audit | `FileDatabase` chưa có version, bill lines, status history, issue records |

## 4. Bằng Chứng Code Quan Trọng

| Nhận định | Bằng chứng |
|---|---|
| Policy chỉ trả bool | `src/policies/business_policies.hpp` |
| Domain model còn mỏng | `src/domain/models.hpp` |
| Accept order đang partial accept khi item unavailable | `src/modules/order_management/order_management_service.cpp` |
| Bill chưa có line/version/stale | `src/modules/payment_billing/payment_billing_service.cpp` |
| Kitchen chưa có issue/served | `src/modules/kitchen_fulfillment/kitchen_fulfillment_service.cpp` |
| Server error chưa có `requiredAction/context/correlationId` | `src/server/server_app.cpp`, `src/server/json_helpers.cpp` |
| Web chỉ toast `error.message` | `web/assets/api.js` |
| Notification mới chỉ polling channel đơn giản | `web/assets/notifications.js`, `src/infrastructure/file_database.cpp` |

## 5. Mức Độ Hoàn Thành Ước Lượng

| Nhóm | Mức đạt hiện tại | Nhận xét |
|---|---:|---|
| Happy path casual dining | 65% | Demo cơ bản chạy được |
| Policy governance | 15% | Có vài bool policy nhưng chưa đúng contract |
| Edge case nghiệp vụ P0 | 35% | Có hủy pending/bill blocker cơ bản, thiếu nhiều case quan trọng |
| Database theo design | 30% | File DB đơn giản, thiếu version/history/entity |
| Web/server theo design | 45% | Có server/web nhưng error/policy UX chưa đạt |
| Staff/permission | 5% | Gần như chưa triển khai |
| Reporting/audit | 25% | Có audit text và revenue, chưa đủ governance |
| Recommendation latent factor | 30% | Có heuristic vector, chưa có model/training latent factor thật |

## 6. Đề Xuất Thứ Tự Triển Khai Tiếp

| Thứ tự | Việc cần làm | Module liên quan |
|---:|---|---|
| 1 | Tạo `PolicyDecision` và deny code chuẩn | `17`, `15`, toàn bộ service |
| 2 | Sửa sold-out at accept: `NEEDS_CUSTOMER_CONFIRMATION` | `05`, `04`, `14`, `15` |
| 3 | Thêm idempotency cho submit/pay | `05`, `07`, `12`, `15` |
| 4 | Bổ sung kitchen issue flow | `06`, `05`, `07`, `10` |
| 5 | Nâng cấp billing: bill lines, version, stale, reopen, payment amount | `07`, `12`, `14`, `15` |
| 6 | Thêm permission/role/manager override tối thiểu | `09`, toàn bộ command nhạy cảm |
| 7 | Chuẩn hóa notification/audit payload | `10`, `11`, `17` |
| 8 | Hoàn thiện web UX theo `requiredAction` | `14`, `15` |
| 9 | Nâng recommendation từ heuristic lên latent factor có model/interactions | `08`, `12` |

## 7. Kết Luận

Code hiện tại phù hợp làm **MVP demo luồng chính**, nhưng chưa khớp với thiết kế sản phẩm đã phân tích sâu.

Nếu giáo viên đánh mạnh nghiệp vụ, cần ưu tiên triển khai các phần P0 trước, đặc biệt là:

- Policy decision chuẩn.
- Sold-out/customer confirmation.
- Kitchen issue.
- Billing correctness.
- Staff permission/manager override.
- Audit/notification có cấu trúc.

## 8. CodeGraph Audit Pass

Phần này là lượt rà soát bổ sung bằng CodeGraph sau khi dự án đã có `.codegraph/codegraph.db`.

### 8.1 Trạng Thái Index

| Hạng mục | Kết quả |
|---|---:|
| Files indexed | 49 |
| Symbols/nodes | 437 |
| Call edges | 853 |
| Indexed language | C++: 43 files, JavaScript: 6 files |
| Test symbols | Chưa thấy test suite/business scenario test đáng kể |

### 8.2 Gap Được CodeGraph Xác Nhận

| Khu vực | Bằng chứng CodeGraph | Thiếu so với `product-design` | Mức |
|---|---|---|---|
| Policy contract | `OperationResult` trong `src/domain/models.hpp` chỉ có `ok/message/id`; search `PolicyDecision` không có kết quả | Chưa có `allowed/code/requiredAction/affectedEntities/auditRequired/notificationTargets` | P0 |
| Deny/error governance | `ServerApp::failure` chỉ trả `jsonError(code, message)`; `web/assets/api.js` chỉ throw message | API chưa trả `requiredAction/context/correlationId`, UI không xử lý decision | P0 |
| Staff permission | Search `Permission` không có symbol nghiệp vụ; actor hiện là string | Chưa có staff user, role matrix, authorization policy, manager override thật | P0 |
| Idempotency | `web/assets/customer.js` có gửi `idempotencyKey`, nhưng model/database/order service không có field hoặc lookup tương ứng | Double submit/pay vẫn có nguy cơ tạo dữ liệu lặp | P0 |
| Sold-out khi accept | `acceptOrder` đang reject item unavailable rồi accept phần còn lại | Sai policy `UnavailableItemDecisionPolicy = REQUIRE_CUSTOMER_CONFIRMATION` | P0 |
| Kitchen fulfillment | Kitchen service/UI chỉ có `start/ready`; không thấy flow `reportIssue`, `SERVED`, `ISSUE` | Thiếu xử lý bếp báo lỗi, sai món, hết nguyên liệu, ready-to-served | P0 |
| Billing correctness | `billTotal` chỉ cộng item không cancelled/rejected; `BillRecord` không có version/lines; `confirmPayment` chỉ nhận method | Thiếu bill snapshot, stale detection, blocker list, paid amount, reopen/adjustment | P0 |
| Notification routing | `notificationsAfter` chỉ lấy theo channel/after id | Thiếu recipient/read state, dedup, resourceVersion, recovery policy | P1 |
| Audit structure | `addAudit` được gọi nhiều nơi nhưng audit model chỉ có role/message/time | Thiếu actorId, action, severity, reason, before/after, correlationId | P1 |
| Latent factor recommendation | Recommendation hiện thiên về vector heuristic/fallback; không thấy model table/vector persistence chuẩn | Chưa có training model, factor version, activation/fallback policy đúng thiết kế | P1 |

### 8.3 Nhận Xét Từ Call Graph

| Symbol/flow | Nhận xét |
|---|---|
| `billTotal` | Chỉ được gọi từ `createBill`, nên nâng cấp bill snapshot/version có phạm vi sửa tương đối gọn. |
| `addAudit` | Có nhiều caller ở seed, table, menu, order, kitchen, payment; nếu đổi audit contract cần refactor xuyên module. |
| `ServerApp` | Đang gom nhiều endpoint trong một lớp lớn; nên tách controller/API handler theo module khi triển khai policy decision. |
| Notification polling | Web có polling theo channel, nhưng logic notification chưa gắn chặt với policy decision và recipient cụ thể. |

### 8.4 Kết Luận Sau CodeGraph

CodeGraph xác nhận kết luận chính không thay đổi: code hiện tại đủ demo happy path, nhưng chưa đạt lớp nghiệp vụ/policy governance của `product-design`.

Ưu tiên kỹ thuật nên bắt đầu từ các điểm có ảnh hưởng rộng:

1. Chuẩn hóa `PolicyDecision` + API error envelope.
2. Sửa sold-out accept sang customer-confirmation flow.
3. Thêm idempotency cho submit order/payment.
4. Bổ sung kitchen issue và served state.
5. Nâng billing sang bill lines/version/stale/payment amount.
6. Thêm staff permission tối thiểu cho cashier/waiter/kitchen/manager.
