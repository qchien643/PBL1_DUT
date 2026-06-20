# Business Rules - Restaurant Configuration

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| CONFIG_001 | `businessProfile` phải là `casual_dining` | `BranchConfigPolicy` | Required |
| CONFIG_002 | Mỗi branch có đúng một config active | `BranchConfigPolicy` | Required |
| CONFIG_003 | Config thay đổi phải tạo version mới | `AuditPolicy` | Required |
| CONFIG_004 | Feature ngoài Casual dining MVP không được bật | `FeaturePolicy` | Required |
| CONFIG_005 | Session active giữ `configVersion` lúc mở bàn | `TablePolicy` | Required |

## Casual dining defaults

| Nhóm | Giá trị |
| --- | --- |
| `table.openingMode` | `staff_manual` |
| `table.allowMerge` | `true` |
| `table.allowTransfer` | `true` |
| `ordering.approvalMode` | `staff_required` |
| `ordering.allowCustomerCancel` | `true` |
| `payment.timing` | `after_meal` |
| `payment.confirmation` | `staff_manual` |
| `kitchen.routingMode` | `category_based` |
| `features.recommendation` | `true` |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `RestaurantProfilePolicy` | Branch có đúng mô hình `casual_dining` không? | `branchId`, active config | `UNSUPPORTED_RESTAURANT_PROFILE` | Audit khi config sai profile được phát hiện |
| `ConfigVersionPolicy` | Command đang dùng config version còn hợp lệ không? | command config version, active config version | `CONFIG_VERSION_STALE` | Notify staff reload nếu ảnh hưởng màn hình đang mở |
| `FeatureFlagPolicy` | Feature có được bật cho branch không? | `featureKey`, active config | `FEATURE_DISABLED` | Audit khi manager đổi feature |

Chi tiết contract, deny output và test coverage dùng chuẩn tại `../17-policy-governance/`.
