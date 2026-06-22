# Data Design - Recommendation AI/ML

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `recommendation_interactions` | Session-item matrix | `sessionId`, `itemId`, `weight`, `source` |
| `recommendation_models` | Model metadata | `id`, `version`, `algorithm`, `factorSize`, `status` |
| `item_latent_factors` | Vector món | `modelId`, `itemId`, `vector`, `bias` |
| `session_latent_factors` | Vector session train | `modelId`, `sessionId`, `vector` |
| `recommendation_training_runs` | Lịch sử train | `modelId`, `startedAt`, `finishedAt`, `metrics` |
| `recommendation_events` | Shown/click/add | `sessionId`, `itemId`, `eventType`, `strategy` |
| `item_pair_rules` | Fallback ăn kèm | `sourceItemId`, `targetItemId`, `weight` |

## Interaction fields

`recommendation_interactions.weight` nên được tính từ order history bằng công thức:

```text
weight = log(1 + quantity) * statusWeight * recencyDecay + eventWeight
```

| Field | Nguồn dữ liệu | Cách dùng |
| --- | --- | --- |
| `quantity` | `order_items.quantity` | Món gọi nhiều hơn tạo tín hiệu mạnh hơn nhưng dùng log để không áp đảo |
| `statusWeight` | `order_items.status` | `READY/SERVED = 1`, `CANCELLED/REJECTED/ISSUE_PENDING_DECISION = 0` |
| `recencyDecay` | thời điểm order/bill | Đơn gần đây ảnh hưởng mạnh hơn đơn quá cũ |
| `eventWeight` | `recommendation_events` | Bấm hoặc thêm món gợi ý tạo tín hiệu bổ sung |
| `source` | `ORDER`, `RECOMMENDATION_ADD`, `RECOMMENDATION_CLICK` | Biết tương tác đến từ order thật hay phản hồi gợi ý |

Các bảng có thể bổ sung sau MVP:

| Table | Key fields | Lý do |
| --- | --- | --- |
| `ingredient_stock` | `ingredientId`, `stockQty`, `expiresAt` | Tăng điểm món cần đẩy tồn kho |
| `menu_item_costs` | `itemId`, `cost`, `margin` | Ưu tiên món có biên lợi nhuận tốt |
| `station_load_snapshots` | `station`, `pendingCount`, `avgPrepDelay` | Giảm điểm món làm lâu khi bếp/quầy quá tải |
| `manual_combo_rules` | `sourceItemId`, `targetItemId`, `priority` | Cho quản lý cấu hình combo thủ công |

## Indexes

| Table | Index |
| --- | --- |
| `item_latent_factors` | unique `modelId`, `itemId` |
| `recommendation_interactions` | `sessionId`, `itemId` |
| `recommendation_models` | `status`, `version` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `recommendation_models.status` | `RecommendationFallbackPolicy` | Chỉ dùng model `ACTIVE` |
| `recommendation_interactions` | latent factor training | Lấy từ order history đã thanh toán/served |
| `item_latent_factors` | scoring | Không thay thế menu availability filter |
| `modelMetrics` | `RecommendationTrainingPolicy` | Chỉ activate nếu đạt threshold MVP |
