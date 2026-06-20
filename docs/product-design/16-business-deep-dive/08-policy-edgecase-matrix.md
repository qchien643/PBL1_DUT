# Policy And Edge Case Matrix

## 1. Ma Trận Policy Tổng Hợp

| Policy | Câu hỏi nghiệp vụ | Nếu deny thì sao? |
|---|---|---|
| `TableSessionPolicy` | Bàn này có được mở không? | Báo bàn không available |
| `TableMergePolicy` | Hai bàn này có ghép được không? | Báo lý do session/table invalid |
| `TableTransferPolicy` | Có chuyển sang bàn target không? | Báo target không available |
| `MenuAvailabilityPolicy` | Món này có order được không? | Reject item |
| `OrderSubmissionPolicy` | Cart/session có hợp lệ không? | Không tạo order |
| `OrderApprovalPolicy` | Order có được accept/reject không? | Reject thao tác |
| `CancelPolicy` | Món có được hủy không? | Reject hoặc cần manager override |
| `KitchenTaskPolicy` | Task có được start/ready không? | Reject thao tác |
| `KitchenIssuePolicy` | Bếp có được báo issue/remake/void không? | Chặn bill, yêu cầu staff xử lý |
| `CancelKitchenRacePolicy` | Hủy món và bếp start cùng lúc xử lý thế nào? | Dùng state mới nhất, audit kết quả |
| `ReadyToServedPolicy` | `READY` đã đủ tính tiền chưa? | Chặn bill hoặc dùng shortcut MVP |
| `BillingPolicy` | Session có được tạo bill không? | Báo blocking reason |
| `BillLockPolicy` | Bill open có khóa session không? | Chặn order mới hoặc yêu cầu reopen |
| `BillStalenessPolicy` | Bill còn đúng sau thay đổi order không? | Chặn payment, yêu cầu recalculate |
| `PaymentPolicy` | Bill có được thanh toán không? | Reject payment |
| `NotificationPolicy` | Gửi event cho channel nào? | Không ảnh hưởng dữ liệu chính |
| `AuditPolicy` | Hành động nào cần audit? | Ghi log hoặc warning |

## 2. Edge Case Priority

| Priority | Edge case | Vì sao quan trọng |
|---|---|---|
| P0 | Double active session trên cùng bàn | Gây sai bill nghiêm trọng |
| P0 | Bill khi còn task preparing | Sai trải nghiệm và tiền |
| P0 | Bill khi order cần khách xác nhận | Billing tự bỏ món sẽ sai ý khách |
| P0 | Bill khi kitchen issue chưa xử lý | Chưa biết món tính tiền/hủy/remake |
| P0 | Cancel món đang bếp làm | Ảnh hưởng chi phí nhà hàng |
| P0 | Món cancelled vẫn tính tiền | Sai tiền khách trả |
| P1 | Sold-out sau khi khách add cart | Thực tế thường gặp |
| P1 | Cashier accept order hai lần | Dễ xảy ra khi nhiều tab |
| P1 | Ghép bàn có order cũ | Dễ mất order nếu thiết kế sai |
| P1 | Bill open rồi khách gọi thêm | Bill snapshot bị lỗi thời |
| P1 | Payment nhập thiếu tiền | Có thể đóng session khi chưa đủ tiền |
| P1 | Notification fail | User không biết có việc mới |
| P2 | Recommendation đề xuất món sold-out | UX xấu, không sai tiền |
| P2 | Audit thiếu cho thao tác quan trọng | Khó giải thích khi bảo vệ |

## 3. Ma Trận Ảnh Hưởng

| Edge case | Bill | Kitchen | Notification | Audit |
|---|---|---|---|---|
| Open occupied table | Không | Không | Không | Optional warning |
| Submit sold-out item | Không tính item | Không tạo task | Customer/cashier | Availability audit |
| Cancel pending item | Không tính | Cancel task | Cashier/customer | Required |
| Cancel preparing item | Có thể vẫn tính | Task tiếp tục | Customer reason | Required |
| Kitchen issue | Chặn bill | Task `ISSUE` | Cashier/waiter/customer | Required |
| Task ready chưa served | Tùy policy | Done nhưng chưa phục vụ | Waiter/cashier | Optional |
| Bill requested too early | Không tạo bill | Không | Customer message | Optional |
| Bill open then order changes | Bill stale/recalculate | Có thể không đổi | Cashier | Required |
| Payment amount invalid | Không paid | Không | Cashier message | Optional |
| Payment confirmed | Bill paid | Không | Customer/manager | Required |
| Merge sessions | Recalculate | Không đổi task | Affected customers | Required |

## 4. Rule Conflict Resolution

Khi nhiều rule cùng áp dụng, ưu tiên:

```text
Data integrity
→ Money correctness
→ Kitchen cost protection
→ Customer fairness
→ UX convenience
```

Ví dụ:

```text
Khách muốn hủy món đang PREPARING
Customer fairness muốn cho hủy
Kitchen cost protection không cho hủy
→ Reject cancel hoặc cần manager override
```

Ví dụ:

```text
Khách muốn thanh toán nhưng order đang NEEDS_CUSTOMER_CONFIRMATION
UX convenience muốn tạo bill nhanh
Money correctness và Customer fairness yêu cầu hỏi khách trước
→ Reject bill, quay lại order decision flow
```

## 5. Checklist Bảo Vệ Nghiệp Vụ

- [ ] Giải thích được dining session khác order.
- [ ] Giải thích được vì sao cần staff approval.
- [ ] Giải thích được hủy món theo trạng thái bếp.
- [ ] Giải thích được bill loại bỏ cancelled/rejected item.
- [ ] Giải thích được bill bị chặn khi order/kitchen chưa ổn định.
- [ ] Giải thích được bill open là snapshot và có thể bị stale.
- [ ] Giải thích được ghép/chuyển bàn không làm mất order.
- [ ] Giải thích được notification khác audit.
- [ ] Giải thích được policy layer giúp mở rộng rule.
