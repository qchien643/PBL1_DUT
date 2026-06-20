# Data Design - Implementation Roadmap

## Migration phases

| Phase | Tables |
| --- | --- |
| Foundation | `branches`, `branch_configs`, `staff_users`, `roles` |
| Table/Menu | `dining_tables`, `dining_sessions`, `menu_items`, `item_availability` |
| Order/Kitchen | `order_headers`, `order_items`, `preparation_tasks`, `task_items` |
| Payment | `bills`, `bill_lines`, `payments` |
| Notification/Audit | `notifications`, `notification_recipients`, `audit_events` |
| Recommendation | `recommendation_interactions`, `recommendation_models`, `item_latent_factors` |

## Seed phases

| Phase | Seed data |
| --- | --- |
| 1 | Branch config, roles |
| 2 | Tables, stations |
| 3 | Menu, availability |
| 4 | Historical sessions/orders |
| 5 | Item pair rules |
