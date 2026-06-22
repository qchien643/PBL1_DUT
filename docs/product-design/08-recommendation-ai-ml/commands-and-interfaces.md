# Commands And Interfaces - Recommendation AI/ML

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `ViewRecommendations` | `RecommendationService.recommend` | Customer | `RecommendationPolicy`, `InventoryPolicy` |
| `BuildRecommendationInteractions` | `RecommendationService.buildInteractions` | Manager | `PermissionPolicy` |
| `TrainRecommendationModel` | `RecommendationService.trainModel` | Manager | `PermissionPolicy` |
| `ActivateRecommendationModel` | `RecommendationService.activateModel` | Manager | `PermissionPolicy`, `AuditPolicy` |
| `TrackRecommendationEvent` | `RecommendationService.trackEvent` | System | N/A |

## Scoring

| Formula | Meaning |
| --- | --- |
| `r_si = log(1 + q_si) * w_status * decay(t_s) + w_event` | Trọng số tương tác giữa phiên `s` và món `i` |
| `p_si = 1 if r_si > 0 else 0` | Tín hiệu implicit feedback |
| `c_si = 1 + alpha * r_si` | Độ tin cậy của tương tác |
| `score_lf(s,i) = mu + b_s + b_i + dot(x_s, y_i)` | Điểm latent factor |
| `x_current = normalize(sum_i a_i * y_i / sum_i a_i)` | Vector cho phiên đang ngồi |
| `score = w_lf*latent + w_pop*popular + w_pair*pair + w_cat*category + w_time*time - w_prep*prep` | Điểm hybrid cuối cùng |

Xem giải thích biến, hàm mất mát và fallback tại [algorithm-design.md](algorithm-design.md).

## Policy-aware service flow

```text
getRecommendations
→ FeatureFlagPolicy
→ RecommendationEligibilityPolicy
→ latent factor scoring if model active
→ RecommendationFallbackPolicy if model weak/unavailable
→ RecommendationFilterPolicy + MenuAvailabilityPolicy
→ return ranked display list
```

Gợi ý chỉ là UI assist; submit order vẫn phải đi qua order/menu policy.
