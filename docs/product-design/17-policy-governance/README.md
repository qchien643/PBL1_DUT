# 17 - Policy Governance

## 1. Mục Tiêu

Module này là **nguồn chuẩn** cho toàn bộ business policy của hệ thống đặt món tại bàn cho nhà hàng **Casual dining**.

Nó trả lời:

- Policy là gì và nằm ở layer nào?
- Mỗi policy phải khai báo những thông tin nào?
- Khi policy deny thì API/UI/CMD hiển thị lỗi ra sao?
- Hành động nào cần audit và notification?
- Edge case nào được policy nào bảo vệ?

## 2. Vì Sao Cần Policy Governance?

Nếu mỗi module tự viết `if else` riêng, hệ thống sẽ nhanh chóng bị lệch nghiệp vụ:

- Cashier có thể accept order khác với rule của kitchen.
- Billing có thể tự bỏ món mà chưa hỏi khách.
- Frontend có thể cho bấm nút nhưng server lại reject.
- Audit có thể thiếu ở các thao tác ảnh hưởng tiền.

Policy governance giúp mọi module dùng chung một cách ra quyết định:

```text
Actor command
→ Permission policy
→ Business policy
→ State transition
→ Audit/notification
→ Response to UI/CMD/API
```

## 3. Tài Liệu Trong Module

| File | Nội dung |
|---|---|
| [policy-architecture.md](policy-architecture.md) | Kiến trúc policy layer và vị trí gọi policy |
| [policy-contract-template.md](policy-contract-template.md) | Format chuẩn cho mỗi policy |
| [policy-catalog.md](policy-catalog.md) | Danh mục policy chính thức theo module |
| [deny-error-codes.md](deny-error-codes.md) | Chuẩn lỗi nghiệp vụ/API khi policy deny |
| [conflict-resolution.md](conflict-resolution.md) | Thứ tự ưu tiên khi rule xung đột |
| [audit-notification-mapping.md](audit-notification-mapping.md) | Mapping policy/action sang audit và notification |
| [policy-test-matrix.md](policy-test-matrix.md) | Test scenario để chứng minh policy đúng |

## 4. Reading Path Khuyến Nghị

```text
00-product-overview
→ 17-policy-governance
→ 01..11 module nghiệp vụ
→ 16-business-deep-dive
→ 13-implementation-roadmap
```

## 5. Quyết Định Mặc Định

| Chủ đề | Quyết định |
|---|---|
| Nhà hàng | Casual dining only |
| Policy owner | Application service/server, không nằm trong UI |
| Error format | Dùng deny code thống nhất |
| Audit | Bắt buộc với hành động ảnh hưởng tiền/trách nhiệm |
| Notification | Là tín hiệu thay đổi, không phải source of truth |
| Sold-out khi cashier accept | `REQUIRE_CUSTOMER_CONFIRMATION` |
| Billing gate | Chặn bill nếu còn order/kitchen/cancel issue chưa ổn định |
| READY billable | Chỉ dùng nếu bật explicit flag `ReadyCountsAsServedPolicy` |

