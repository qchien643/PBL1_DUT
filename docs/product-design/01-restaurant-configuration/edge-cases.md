# Edge Cases - Restaurant Configuration

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Không có config active | Dùng seed default hoặc chặn app startup | System safety |
| Manager bật feature ngoài MVP | `FeaturePolicy` từ chối | Không đổi config |
| Config đổi khi session active | Session cũ giữ `configVersion`; session mới dùng version mới | Table/order/payment |
| Config JSON thiếu field bắt buộc | Validation fail | Không activate |
| Hai manager update cùng lúc | Version tăng tuần tự, activate version mới nhất hợp lệ | Audit |

## Audit requirements

Mỗi lần config đổi phải ghi `config_changed` với `actorId`, `oldVersion`, `newVersion`, `diff`.

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Manager đổi profile khỏi `casual_dining` | `RestaurantProfilePolicy` | Reject, giữ config cũ | Required | Manager |
| Config đổi giữa lúc cashier đang thao tác | `ConfigVersionPolicy` | Reject command stale, yêu cầu reload | Required nếu command quan trọng | Affected staff screens |
| Tắt recommendation khi customer đang xem menu | `FeatureFlagPolicy` | Ẩn recommendation, không ảnh hưởng order | Required | Customer/table refresh |

Không module nào được tự override config rule mà không đi qua policy governance.
