# Table Groups

## 1. Configuration

| Table | Key fields |
| --- | --- |
| `tenants` | `id`, `name`, `status` |
| `restaurants` | `id`, `tenantId`, `name`, `businessProfile` |
| `branches` | `id`, `restaurantId`, `name`, `status` |
| `branch_configs` | `id`, `branchId`, `version`, `configJson`, `status` |
| `feature_flags` | `branchId`, `featureKey`, `enabled` |

## 2. Core operation

| Table | Key fields |
| --- | --- |
| `dining_tables` | `id`, `code`, `area`, `capacity`, `status` |
| `dining_sessions` | `id`, `primaryTableId`, `status`, `guestCount`, `configVersion` |
| `dining_session_tables` | `sessionId`, `tableId`, `role`, `status` |
| `menu_items` | `id`, `categoryId`, `name`, `basePrice`, `catalogStatus` |
| `item_availability` | `branchId`, `itemId`, `availabilityStatus`, `isOrderable` |

## 3. Order to kitchen

| Table | Key fields |
| --- | --- |
| `order_headers` | `id`, `sessionId`, `status`, `clientRequestId` |
| `order_items` | `id`, `orderId`, `menuItemId`, `quantity`, `status` |
| `cancellation_requests` | `id`, `orderItemId`, `reason`, `status` |
| `preparation_tasks` | `id`, `orderId`, `stationId`, `status` |
| `task_items` | `taskId`, `orderItemId`, `quantity`, `status` |

## 4. Payment and governance

| Table | Key fields |
| --- | --- |
| `bills` | `id`, `sessionId`, `status`, `subtotal`, `total`, `version` |
| `bill_lines` | `billId`, `orderItemId`, `amount` |
| `payments` | `billId`, `method`, `amount`, `status`, `confirmedBy` |
| `notifications` | `id`, `eventId`, `eventType`, `message` |
| `audit_events` | `eventType`, `actorId`, `resourceType`, `resourceId`, `payload` |

## 5. Recommendation

| Table | Key fields |
| --- | --- |
| `recommendation_interactions` | `sessionId`, `itemId`, `weight` |
| `recommendation_models` | `id`, `version`, `algorithm`, `status` |
| `item_latent_factors` | `modelId`, `itemId`, `vector`, `bias` |
| `recommendation_events` | `sessionId`, `itemId`, `eventType`, `strategy` |

## 6. Policy governance support

| Table/field | Policy cần hỗ trợ | Ghi chú |
| --- | --- | --- |
| `*_status_history` | State transition policy | Dùng khi cần giải thích trạng thái trước/sau |
| `idempotency_keys` hoặc unique request key | `OrderIdempotencyPolicy`, `PaymentIdempotencyPolicy` | MVP có thể dùng unique field trong order/payment |
| `bills.sessionVersion` | `BillStalenessPolicy` | Chặn payment bill cũ |
| `order_headers.status=NEEDS_CUSTOMER_CONFIRMATION` | `UnavailableItemDecisionPolicy` | Không tạo kitchen task/bill |
| `preparation_tasks.status=ISSUE` | `KitchenIssuePolicy`, `CanCreateBillPolicy` | Chặn bill đến khi resolved |
| `audit_events.correlationId` | `AuditRequiredPolicy` | Trace command xuyên module |
| `notifications.resourceVersion` | `NotificationRecoveryPolicy` | Client reload state khi stale |
