# API Specification

API sử dụng JSON. Tất cả response nên theo envelope thống nhất.

## 1. Response Envelope

### Success

```json
{
  "ok": true,
  "data": {}
}
```

### Failure

```json
{
  "ok": false,
  "error": {
    "code": "BILL_BLOCKED_BY_ACTIVE_WORK",
    "message": "Cannot create bill because kitchen tasks are still preparing.",
    "requiredAction": "RESOLVE_ORDER_OR_KITCHEN_WORK",
    "context": {
      "blockingTasks": ["TASK-001"]
    }
  },
  "correlationId": "REQ-20260620-0001"
}
```

Failure response dùng deny code từ `../17-policy-governance/deny-error-codes.md`.

## 2. Health

### `GET /api/health`

```json
{
  "ok": true,
  "data": {
    "status": "UP",
    "service": "casual-dining-server"
  }
}
```

## 3. Table Session APIs

### `GET /api/tables`

Lấy danh sách bàn.

### `POST /api/tables/{code}/open`

Cashier mở bàn.

```json
{
  "actor": "cashier"
}
```

Notification tạo ra:

| Channel | Type |
|---|---|
| `customer:T01` | `TABLE_OPENED` |

### `GET /api/tables/{code}/session`

Customer kiểm tra bàn đang có session không.

### `POST /api/tables/merge`

```json
{
  "mainTableCode": "T01",
  "joinedTableCode": "T02",
  "actor": "cashier"
}
```

### `POST /api/tables/transfer`

```json
{
  "sourceTableCode": "T01",
  "targetTableCode": "T03",
  "actor": "cashier"
}
```

### `POST /api/tables/{code}/cleaned`

Đánh dấu bàn đã dọn xong.

## 4. Menu APIs

### `GET /api/menu`

Query:

| Query | Ý nghĩa |
|---|---|
| `includeHidden=true` | Manager xem cả món inactive |

### `PATCH /api/menu/{id}/availability`

```json
{
  "availabilityStatus": "SOLD_OUT",
  "actor": "manager"
}
```

## 5. Order APIs

### `POST /api/orders`

Customer submit order.

```json
{
  "tableCode": "T01",
  "sessionId": 3,
  "items": [
    {
      "menuItemId": 1,
      "quantity": 2
    }
  ],
  "idempotencyKey": "T01-1710000000-abc"
}
```

Notification tạo ra:

| Channel | Type |
|---|---|
| `cashier` | `NEW_ORDER` |

### `GET /api/sessions/{sessionId}/orders`

Customer xem trạng thái order của bàn.

### `GET /api/orders/pending`

Cashier xem order chờ duyệt.

### `POST /api/orders/{orderId}/accept`

Cashier duyệt order.

Notification tạo ra:

| Channel | Type |
|---|---|
| `customer:T01` | `ORDER_ACCEPTED` |
| `kitchen` | `TASK_CREATED` |
| `bar` | `TASK_CREATED` |

### `POST /api/orders/{orderId}/reject`

Cashier từ chối order.

### `POST /api/order-items/{orderItemId}/cancel-request`

Customer xin hủy món đặt nhầm.

### `POST /api/order-items/{orderItemId}/cancel-approve`

Cashier duyệt hủy món.

Nếu bếp đã bắt đầu làm:

```json
{
  "ok": false,
  "error": {
    "code": "CANCEL_TOO_LATE",
    "message": "Cannot cancel because kitchen already started preparing this item."
  }
}
```

## 6. Kitchen APIs

### `GET /api/kitchen/tasks?station=kitchen`

Lấy task cho bếp.

### `POST /api/kitchen/tasks/{taskId}/start`

Bếp bắt đầu làm món.

### `POST /api/kitchen/tasks/{taskId}/ready`

Bếp báo món sẵn sàng.

Notification:

| Channel | Type |
|---|---|
| `cashier` | `TASK_READY` |
| `customer:T01` | `TASK_READY` |

## 7. Billing APIs

### `POST /api/sessions/{sessionId}/bill`

Customer hoặc cashier tạo bill.

### `GET /api/bills/open`

Cashier xem bill chưa thanh toán.

### `POST /api/bills/{billId}/pay`

Cashier xác nhận thanh toán.

## 8. Recommendation APIs

### `GET /api/sessions/{sessionId}/recommendations`

Trả danh sách món đề xuất.

## 9. Notification APIs

### `GET /api/notifications?channel=cashier&after=10`

Frontend polling notification mới hơn `id=10`.

```json
{
  "ok": true,
  "data": {
    "events": [
      {
        "id": 11,
        "channel": "cashier",
        "type": "NEW_ORDER",
        "message": "Table T01 submitted order #10.",
        "createdAt": "2026-06-05_23:30:00"
      }
    ]
  }
}
```

## 10. Reporting APIs

### `GET /api/reports/summary`

Manager xem doanh thu, số order, món bán chạy.

### `GET /api/audit-events?limit=50`

Manager xem audit log.

## 11. API Edge Cases

| Case | API nên trả |
|---|---|
| Bàn chưa mở mà customer đặt món | `TABLE_NOT_ACTIVE` |
| Món hết hàng sau khi khách thêm vào cart | `ITEM_UNAVAILABLE` |
| Cashier accept order đã được accept | `ORDER_NOT_SUBMITTED` |
| Customer xin hủy món đang preparing | `CANCEL_TOO_LATE` |
| Tạo bill khi còn task pending/preparing/issue | `BILL_BLOCKED_BY_ACTIVE_WORK` |
| Cashier accept order có món vừa hết | `ITEM_UNAVAILABLE_REQUIRES_CUSTOMER_DECISION` |
| Payment bill stale | `BILL_STALE_RECALCULATE_REQUIRED` |
| Payment thiếu tiền | `PAYMENT_AMOUNT_INVALID` |
| Polling notification với channel sai | `INVALID_CHANNEL` |
| Gửi request trùng do double click | Dùng `idempotencyKey` |
