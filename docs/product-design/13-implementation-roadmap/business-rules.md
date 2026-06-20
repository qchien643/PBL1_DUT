# Business Rules - Implementation Roadmap

## Implementation rules

| Rule ID | Rule |
| --- | --- |
| IMPL_001 | Không implement UI logic chứa business rule |
| IMPL_002 | Mọi command nhạy cảm phải đi qua policy |
| IMPL_003 | Service phải dùng transaction khi đổi nhiều bảng |
| IMPL_004 | Audit/notification phát từ domain event |
| IMPL_005 | Order/bill luôn dùng snapshot |

## Priority rules

| Priority | Rule |
| --- | --- |
| P0 | Table/session, menu, order, kitchen, payment |
| P1 | Cancel item, notification, audit |
| P2 | Recommendation, reporting |
| P3 | Polish CMD/demo |
