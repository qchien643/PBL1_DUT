# Edge Cases - Recommendation AI/ML

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Chưa có model | Fallback best seller/item pair | Customer still sees suggestions |
| Cart rỗng | Best seller/category fallback | Recommendation |
| Món mới không có vector | Category fallback | Recommendation |
| Món gợi ý sold out | `InventoryPolicy` loại bỏ | Menu/Inventory |
| Session billing | Không recommend thêm món | Payment |
| Model train lỗi | Không activate, giữ model cũ/fallback | Manager |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Session mới chưa có lịch sử | `RecommendationFallbackPolicy` | Dùng best-seller/category fallback | No | No |
| Candidate sold-out | `RecommendationFilterPolicy`, `MenuAvailabilityPolicy` | Loại khỏi danh sách trước khi hiển thị | No | No |
| Session bill locked | `RecommendationEligibilityPolicy` | Ẩn gợi ý để tránh gọi thêm | No | Customer |
| Model train lỗi | `RecommendationTrainingPolicy` | Giữ model active cũ, fallback rule-based | Optional | Manager |
