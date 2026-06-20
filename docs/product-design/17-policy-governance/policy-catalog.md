# Policy Catalog

## 1. Module Policy Map

| Policy | Owner | Business question | Main deny codes |
|---|---|---|---|
| `RestaurantProfilePolicy` | `01-restaurant-configuration` | Branch có đúng profile `casual_dining` không? | `UNSUPPORTED_RESTAURANT_PROFILE` |
| `ConfigVersionPolicy` | `01-restaurant-configuration` | Command đang dùng config version hợp lệ không? | `CONFIG_VERSION_STALE` |
| `FeatureFlagPolicy` | `01-restaurant-configuration` | Feature này có được bật trong branch không? | `FEATURE_DISABLED` |
| `ConsoleActorContextPolicy` | `02-console-runtime` | CMD/web context có map đúng actor/table/station không? | `INVALID_ACTOR_CONTEXT` |
| `ConsoleCommandRoutingPolicy` | `02-console-runtime` | Command có được gửi tới service tương ứng không? | `COMMAND_NOT_AVAILABLE_IN_CONTEXT` |
| `CanOpenTablePolicy` | `03-table-session` | Staff có được mở bàn không? | `TABLE_NOT_AVAILABLE`, `TABLE_ALREADY_ACTIVE` |
| `CanMergeTablePolicy` | `03-table-session` | Hai session/bàn có thể ghép không? | `TABLE_MERGE_NOT_ALLOWED` |
| `CanTransferTablePolicy` | `03-table-session` | Có chuyển session sang bàn đích không? | `TARGET_TABLE_OCCUPIED` |
| `TableCleanlinessPolicy` | `03-table-session` | Bàn cleaning có được mở lại chưa? | `TABLE_NOT_CLEANED` |
| `MenuCatalogStatusPolicy` | `04-menu-inventory` | Item có được hiển thị/order không? | `ITEM_NOT_ORDERABLE` |
| `MenuAvailabilityPolicy` | `04-menu-inventory` | Item còn phục vụ tại branch không? | `ITEM_UNAVAILABLE` |
| `ModifierAvailabilityPolicy` | `04-menu-inventory` | Modifier có hợp lệ tại submit không? | `MODIFIER_UNAVAILABLE` |
| `PriceSnapshotPolicy` | `04-menu-inventory` | Order dùng giá nào khi giá đổi? | `PRICE_CHANGED_REQUIRES_CONFIRMATION` |
| `CanSubmitOrderPolicy` | `05-order-management` | Customer có được submit cart không? | `TABLE_NOT_ACTIVE`, `EMPTY_CART`, `SESSION_LOCKED_FOR_BILLING` |
| `OrderIdempotencyPolicy` | `05-order-management` | Submit trùng có tạo order mới không? | `DUPLICATE_REQUEST` |
| `CanAcceptOrderPolicy` | `05-order-management` | Cashier có được accept order không? | `ORDER_NOT_ACCEPTABLE` |
| `UnavailableItemDecisionPolicy` | `05-order-management` | Sold-out lúc accept xử lý thế nào? | `ITEM_UNAVAILABLE_REQUIRES_CUSTOMER_DECISION` |
| `CanCancelOrderItemPolicy` | `05-order-management` | Item có được hủy theo state hiện tại không? | `CANCEL_TOO_LATE`, `CANCEL_REQUIRES_MANAGER_OVERRIDE` |
| `CanCreateKitchenTaskPolicy` | `06-kitchen-fulfillment` | Order item đã đủ điều kiện xuống bếp chưa? | `ORDER_NOT_READY_FOR_FULFILLMENT` |
| `KitchenRoutingPolicy` | `06-kitchen-fulfillment` | Item route tới station nào? | `KITCHEN_STATION_NOT_CONFIGURED` |
| `KitchenTaskStatePolicy` | `06-kitchen-fulfillment` | Task có được start/ready/served không? | `KITCHEN_TASK_INVALID_STATE` |
| `KitchenIssuePolicy` | `06-kitchen-fulfillment` | Bếp có được report issue không? | `KITCHEN_ISSUE_NOT_ALLOWED` |
| `ReadyToServedPolicy` | `06-kitchen-fulfillment` | `READY` đã đủ billable chưa? | `ITEM_NOT_SERVED_YET` |
| `CanCreateBillPolicy` | `07-payment-billing` | Session có được tạo bill không? | `BILL_BLOCKED_BY_ACTIVE_WORK`, `BILL_ALREADY_EXISTS` |
| `BillCalculationPolicy` | `07-payment-billing` | Item nào được tính tiền? | `BILL_CALCULATION_INVALID_STATE` |
| `BillLockPolicy` | `07-payment-billing` | Bill open có khóa order mới không? | `SESSION_LOCKED_FOR_BILLING` |
| `BillStalenessPolicy` | `07-payment-billing` | Bill snapshot còn hợp lệ không? | `BILL_STALE_RECALCULATE_REQUIRED` |
| `CanPayBillPolicy` | `07-payment-billing` | Cashier có được confirm payment không? | `BILL_NOT_OPEN`, `PAYMENT_AMOUNT_INVALID` |
| `RecommendationEligibilityPolicy` | `08-recommendation-ai-ml` | Có được recommend trong session này không? | `RECOMMENDATION_NOT_ALLOWED` |
| `RecommendationFilterPolicy` | `08-recommendation-ai-ml` | Candidate nào bị loại khỏi gợi ý? | `RECOMMENDATION_CANDIDATE_FILTERED` |
| `RecommendationFallbackPolicy` | `08-recommendation-ai-ml` | Khi latent factor không đủ dữ liệu thì fallback thế nào? | `RECOMMENDATION_MODEL_UNAVAILABLE` |
| `PermissionPolicy` | `09-staff-permission` | Actor có quyền chạy command không? | `PERMISSION_DENIED` |
| `ManagerOverridePolicy` | `09-staff-permission` | Hành động nhạy cảm có cần manager không? | `MANAGER_OVERRIDE_REQUIRED` |
| `RoleSafetyPolicy` | `09-staff-permission` | Có được đổi role/permission này không? | `ROLE_CHANGE_NOT_SAFE` |
| `NotificationRoutingPolicy` | `10-device-notification` | Event gửi tới recipient nào? | `NOTIFICATION_ROUTE_NOT_FOUND` |
| `NotificationDedupPolicy` | `10-device-notification` | Notification trùng có hiển thị lại không? | `DUPLICATE_NOTIFICATION` |
| `NotificationRecoveryPolicy` | `10-device-notification` | Client offline sync lại thế nào? | `NOTIFICATION_SYNC_REQUIRED` |
| `AuditRequiredPolicy` | `11-reporting-audit` | Action này có bắt buộc audit không? | `AUDIT_REQUIRED` |
| `ReportFilterPolicy` | `11-reporting-audit` | Report loại item/status nào? | `REPORT_SCOPE_INVALID` |
| `AuditRetentionPolicy` | `11-reporting-audit` | Audit có được sửa/xóa không? | `AUDIT_IMMUTABLE` |

## 2. Cross-Module Gate Policies

| Gate | From | To | Rule |
|---|---|---|---|
| `OrderReadyForFulfillmentPolicy` | Order | Kitchen | Chỉ item accepted và không cần khách xác nhận mới tạo task |
| `KitchenCompletionGatePolicy` | Kitchen | Billing | Bill bị chặn nếu còn task `PENDING`, `PREPARING`, `ISSUE` |
| `CustomerDecisionGatePolicy` | Order/Menu | Billing | Bill bị chặn nếu còn `NEEDS_CUSTOMER_CONFIRMATION` |
| `CancellationResolutionGatePolicy` | Order | Billing | Bill bị chặn nếu cancel request chưa xử lý |
| `AuditForMoneyImpactPolicy` | All | Audit | Mọi action ảnh hưởng tiền phải audit |
| `NotificationAfterStateChangePolicy` | All | Notification | Notification chỉ gửi sau khi state commit thành công |

## 3. Default Policy Values

| Policy | Default |
|---|---|
| `UnavailableItemDecisionPolicy` | `REQUIRE_CUSTOMER_CONFIRMATION` |
| `ReadyToServedPolicy` | `READY` chưa đủ billable nếu có waiter flow |
| `ReadyCountsAsServedPolicy` | `false`, chỉ bật cho MVP nếu không triển khai served flow |
| `BillLockPolicy` | Bill `OPEN` khóa order mới |
| `BillStalenessPolicy` | Nếu session version đổi sau bill thì bill không payable |
| `PaymentMethodPolicy` | Manual cash/card/bank transfer only |

