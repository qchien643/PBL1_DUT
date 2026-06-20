# Data Design - Restaurant Configuration

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `tenants` | Chủ sở hữu dữ liệu | `id`, `name`, `status` |
| `restaurants` | Nhà hàng Casual dining | `id`, `tenantId`, `name`, `businessProfile` |
| `branches` | Chi nhánh chính | `id`, `restaurantId`, `name`, `status` |
| `branch_configs` | Config versioned | `id`, `branchId`, `version`, `configJson`, `status` |
| `feature_flags` | Feature bật/tắt | `branchId`, `featureKey`, `enabled` |
| `config_versions` | Lịch sử config | `branchId`, `version`, `createdBy`, `createdAt` |

## Indexes

| Table | Index |
| --- | --- |
| `branch_configs` | unique `branchId`, `version` |
| `branch_configs` | `branchId`, `status` |
| `feature_flags` | unique `branchId`, `featureKey` |

## Config snapshot

Session/order/bill nên lưu `configVersion` để không bị thay đổi rule khi manager update config giữa ca.

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `businessProfile` | `RestaurantProfilePolicy` | MVP chỉ cho `casual_dining` |
| `configVersion` | `ConfigVersionPolicy` | Lưu snapshot vào session/order/bill |
| `featureFlags` | `FeatureFlagPolicy` | Recommendation, waiter served flow, ready-as-served |
| `updatedBy`, `updatedAt` | `AuditRequiredPolicy` | Bắt buộc để giải thích config change |
