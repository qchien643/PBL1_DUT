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
