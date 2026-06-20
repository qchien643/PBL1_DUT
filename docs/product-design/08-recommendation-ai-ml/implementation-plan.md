# Implementation Plan - Recommendation AI/ML

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Create recommendation schema | Tables exist |
| 2 | Seed order history | Enough interactions |
| 3 | Implement fallback strategies | Works without model |
| 4 | Build interaction matrix | Manager command creates data |
| 5 | Implement SGD training | Item vectors saved |
| 6 | Implement recommend from cart | Suggestions change by cart |
| 7 | Filter availability | Sold out excluded |
| 8 | Track events | Shown/click/add recorded |

## Acceptance criteria

- Works without customer history.
- Works without active model.
- Latent factor suggestions are filtered by availability.
