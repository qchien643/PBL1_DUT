# Business Rules - Recommendation AI/ML

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| RECO_001 | Không cần customer account; dùng `DiningSession` | `RecommendationPolicy` | Required |
| RECO_002 | Món sold out không được recommend | `InventoryPolicy` | Required |
| RECO_003 | Món trong cart không được recommend lại | `RecommendationPolicy` | Required |
| RECO_004 | Không có active model thì fallback | `RecommendationPolicy` | Required |
| RECO_005 | Model chỉ train từ session paid/served | `RecommendationPolicy` | Required |
| RECO_006 | Recommendation không tự thêm món vào cart | N/A | Required |
| RECO_007 | Kết quả phải có `strategy` và `reason` | `RecommendationPolicy` | Recommended |

## Strategy priority

| Priority | Strategy |
| --- | --- |
| 1 | Latent factor from cart/session |
| 2 | Item pair rule |
| 3 | Best seller |
| 4 | Category fallback |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `RecommendationEligibilityPolicy` | Session có được gợi ý món không? | session status, bill lock, feature flag | `RECOMMENDATION_NOT_ALLOWED` | No audit |
| `RecommendationFilterPolicy` | Candidate nào phải loại khỏi kết quả? | menu status, availability, already ordered items | `RECOMMENDATION_CANDIDATE_FILTERED` | No audit |
| `RecommendationFallbackPolicy` | Khi model thiếu dữ liệu thì dùng fallback nào? | model status, interaction count | `RECOMMENDATION_MODEL_UNAVAILABLE` | Audit optional for model failure |
| `RecommendationTrainingPolicy` | Model mới có được activate không? | training metrics, data volume | `RECOMMENDATION_MODEL_UNAVAILABLE` | Audit manager/model activation |

Recommendation không được đề xuất món sold-out hoặc khi session đã billing.
