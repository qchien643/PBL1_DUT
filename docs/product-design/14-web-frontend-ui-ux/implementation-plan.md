# Frontend Implementation Plan

## 1. Folder Đề Xuất

```text
web/
  customer.html
  cashier.html
  kitchen.html
  manager.html
  assets/
    app.css
    api.js
    notifications.js
    customer.js
    cashier.js
    kitchen.js
    manager.js
```

## 2. Phase 1 - Static Layout

- Tạo HTML layout cho 4 màn hình.
- Tạo CSS chung: card, table, button, toast, badge, modal.
- Chưa gọi API, dùng mock data tạm.

## 3. Phase 2 - API Client

- Viết `api.js`.
- Chuẩn hóa:

```js
async function apiGet(path) {}
async function apiPost(path, body) {}
async function apiPatch(path, body) {}
```

- Tất cả function đọc response envelope `{ ok, data, error }`.

## 4. Phase 3 - Customer UI

- Load session theo table code.
- Load menu.
- Add/remove cart item.
- Submit order.
- View order status.
- Request bill.

## 5. Phase 4 - Cashier UI

- Table board.
- Open/merge/transfer table.
- Pending order queue.
- Accept/reject order.
- Cancel request panel.
- Open bill panel.

## 6. Phase 5 - Kitchen/Bar UI

- Load tasks theo station.
- Start task.
- Mark ready.
- Chia task theo cột.

## 7. Phase 6 - Manager UI

- Menu availability.
- Revenue summary.
- Audit log.

## 8. Phase 7 - Notification UX

- Viết `notifications.js`.
- Polling theo channel.
- Toast + badge.
- Reload đúng panel.

## 9. Tiêu Chí Hoàn Thành

- Demo được toàn bộ luồng casual dining bằng browser.
- Không cần mở CMD cho từng actor.
- Khi có order mới, cashier thấy notification.
- Khi cashier accept, kitchen/bar thấy notification.
- Khi task ready, customer/cashier thấy notification.
