# Notification And Audit Deep Dive

## 1. Notification Khác Audit Như Thế Nào

| Khái niệm | Mục tiêu | Ví dụ |
|---|---|---|
| Notification | Báo cho actor cần hành động | `NEW_ORDER` cho cashier |
| Audit | Ghi lại lịch sử để truy vết | Cashier accepted order #3 |

Notification có thể biến mất sau khi user thấy, nhưng audit phải giữ để giải thích nghiệp vụ.

## 2. Notification Events

| Event | Producer | Consumer | UI action |
|---|---|---|---|
| `TABLE_OPENED` | Cashier | Customer table | Reload session |
| `NEW_ORDER` | Customer | Cashier | Reload pending orders |
| `ORDER_ACCEPTED` | Cashier | Customer | Reload order status |
| `TASK_CREATED` | Server after accept | Kitchen/Bar | Reload task board |
| `TASK_READY` | Kitchen/Bar | Cashier/Customer | Reload status |
| `CANCEL_REQUESTED` | Customer | Cashier | Reload cancel panel |
| `CANCEL_APPROVED` | Cashier | Customer | Reload order status |
| `BILL_REQUESTED` | Customer | Cashier | Reload bills |
| `BILL_PAID` | Cashier | Customer/Manager | Reload session/revenue |

## 3. Notification Edge Cases

| Edge case | Tình huống | Xử lý |
|---|---|---|
| Client offline | Browser mất mạng | Khi reconnect, polling theo `lastNotificationId` |
| Mở hai tab cashier | Cả hai nhận notification | Chấp nhận MVP |
| Notification trùng | Polling retry | Dedupe bằng event id |
| Notification đến khi user đang nhập | Không reload toàn trang | Chỉ reload panel liên quan |
| Event quan trọng nhưng notification fail | Dữ liệu vẫn lưu, user có thể refresh | Audit vẫn có |

## 4. Audit Events Bắt Buộc

| Hành động | Vì sao audit? |
|---|---|
| Open table | Bắt đầu session |
| Merge/transfer table | Ảnh hưởng order/bill context |
| Accept/reject order | Ảnh hưởng bếp và tiền |
| Approve cancel | Ảnh hưởng bill |
| Mark task ready | Ảnh hưởng bill readiness |
| Create bill | Chốt tổng tiền |
| Confirm payment | Đóng session |
| Change availability | Ảnh hưởng bán hàng |

## 5. Điểm Cần Nhấn Khi Bảo Vệ

- Notification phục vụ vận hành realtime.
- Audit phục vụ trách nhiệm và truy vết.
- Một event nghiệp vụ có thể tạo cả notification và audit.
