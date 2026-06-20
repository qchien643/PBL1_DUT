# Notification UX

Tài liệu này chỉ mô tả **cách frontend hiển thị notification**. Thiết kế lưu trữ, channel và API polling nằm ở [15-cpp-server-api/notification-polling.md](../15-cpp-server-api/notification-polling.md).

## 1. UX Mục Tiêu

Khi server phát sinh event, frontend cần:

```text
1. Hiện toast/badge cho người dùng biết
2. Reload đúng panel liên quan
3. Không làm mất input người dùng đang nhập
```

## 2. Toast Pattern

Ví dụ cashier nhận order mới:

```text
[NEW ORDER] Table T01 submitted order #3
```

UI nên:

- Hiện toast ở góc phải.
- Tăng badge notification.
- Reload `PendingOrderQueue`.

## 3. Badge Pattern

| Actor | Badge nên hiển thị |
|---|---|
| Customer | Order accepted, item ready, bill paid |
| Cashier | New order, cancel request, bill request |
| Kitchen/Bar | Task created |
| Manager | Payment completed, important audit event |

## 4. Không Reload Toàn Trang

Không nên:

```js
window.location.reload();
```

Vì:

- Mất cart hoặc form đang nhập.
- Màn hình nhấp nháy.
- Khó dùng khi nhà hàng đông khách.

Nên:

```js
showToast(event.message);
reloadPanelByEventType(event.type);
```

## 5. Mapping Event Sang UI Action

| Event type | Page | UI action |
|---|---|---|
| `TABLE_OPENED` | Customer | Reload session banner |
| `NEW_ORDER` | Cashier | Reload pending order panel |
| `ORDER_ACCEPTED` | Customer | Reload order status |
| `TASK_CREATED` | Kitchen/Bar | Reload task board |
| `TASK_READY` | Customer/Cashier | Reload order status/table board |
| `CANCEL_REQUESTED` | Cashier | Reload cancel request panel |
| `BILL_REQUESTED` | Cashier | Reload open bill panel |
| `BILL_PAID` | Customer/Manager | Reload session/revenue |

## 6. Trạng Thái Mất Kết Nối

Nếu polling lỗi:

```text
Server disconnected. Retrying...
```

UI không nên xóa dữ liệu đang có. Khi kết nối lại, polling tiếp tục từ `lastNotificationId`.
