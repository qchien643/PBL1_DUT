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
| `R[session][item] = 1 + log(1 + quantity)` | Interaction weight |
| `currentSessionVector = average(cart item vectors)` | Vector cho session active |
| `score = dot(sessionVector, itemVector) + itemBias + boosts` | Hybrid score |

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
