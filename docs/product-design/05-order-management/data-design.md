# Data Design - Order Management

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `order_headers` | Order theo session | `id`, `sessionId`, `status`, `clientRequestId`, `submittedAt`, `acceptedBy` |
| `order_items` | Món trong order | `id`, `orderId`, `menuItemId`, `quantity`, `unitPriceSnapshot`, `status` |
| `order_item_modifiers` | Modifier snapshot | `orderItemId`, `modifierName`, `priceDeltaSnapshot` |
| `order_status_history` | Lịch sử order | `orderId`, `fromStatus`, `toStatus`, `actorId`, `reason` |
| `order_item_status_history` | Lịch sử item | `orderItemId`, `fromStatus`, `toStatus`, `actorId`, `reason` |
| `cancellation_requests` | Yêu cầu hủy món | `id`, `orderItemId`, `requestedBy`, `reason`, `status`, `resolvedBy` |

## Indexes

| Table | Index |
| --- | --- |
| `order_headers` | `sessionId`, `status` |
| `order_headers` | unique `sessionId`, `clientRequestId` |
| `order_items` | `orderId`, `status` |
| `cancellation_requests` | `orderItemId`, `status` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `order_headers.clientRequestId` | `OrderIdempotencyPolicy` | Unique trong session |
| `order_headers.status` | `CanAcceptOrderPolicy`, billing gates | Thêm `NEEDS_CUSTOMER_CONFIRMATION` |
| `order_items.status` | cancel/bill/kitchen gates | Không delete item đã hủy |
| `order_items.priceSnapshot` | `PriceSnapshotPolicy`, `BillCalculationPolicy` | Tránh sai giá |
| `cancellation_requests.reason` | `CanCancelOrderItemPolicy`, `AuditRequiredPolicy` | Required cho audit |
