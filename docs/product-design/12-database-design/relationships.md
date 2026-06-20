# Relationships

## 1. ERD tổng hợp

```mermaid
erDiagram
    TENANT ||--o{ RESTAURANT : owns
    RESTAURANT ||--o{ BRANCH : has
    BRANCH ||--o{ BRANCH_CONFIG : configures
    BRANCH ||--o{ DINING_TABLE : contains
    DINING_TABLE ||--o{ DINING_SESSION_TABLE : assigned
    DINING_SESSION ||--o{ DINING_SESSION_TABLE : has
    DINING_SESSION ||--o{ ORDER_HEADER : has
    ORDER_HEADER ||--o{ ORDER_ITEM : contains
    ORDER_ITEM ||--o{ ORDER_ITEM_MODIFIER : has
    ORDER_ITEM ||--o{ CANCELLATION_REQUEST : may_have
    BRANCH ||--o{ MENU : owns
    MENU ||--o{ MENU_CATEGORY : contains
    MENU_CATEGORY ||--o{ MENU_ITEM : contains
    MENU_ITEM ||--o{ ITEM_AVAILABILITY : has
    ORDER_HEADER ||--o{ PREPARATION_TASK : creates
    PREPARATION_STATION ||--o{ PREPARATION_TASK : receives
    PREPARATION_TASK ||--o{ TASK_ITEM : contains
    DINING_SESSION ||--o{ BILL : produces
    BILL ||--o{ BILL_LINE : contains
    BILL ||--o{ PAYMENT : has
    DINING_SESSION ||--o{ RECOMMENDATION_INTERACTION : creates
    MENU_ITEM ||--o{ RECOMMENDATION_INTERACTION : appears_in
    RECOMMENDATION_MODEL ||--o{ ITEM_LATENT_FACTOR : has
    STAFF_USER ||--o{ AUDIT_EVENT : performs
```

## 2. Important constraints

| Constraint | Reason |
| --- | --- |
| One active session per table | Tránh order sai bàn |
| Unique `clientRequestId` per session | Chống submit order trùng |
| Unique availability per branch/item | Một nguồn trạng thái món |
| One active bill per session | MVP không split bill |
| One active recommendation model | Gợi ý nhất quán |

## 3. Index checklist

| Table | Index |
| --- | --- |
| `dining_session_tables` | active `tableId` |
| `order_headers` | `sessionId`, `status` |
| `preparation_tasks` | `stationId`, `status` |
| `bills` | `sessionId`, `status` |
| `notifications` | `eventId`, `eventType` |
| `audit_events` | `resourceType`, `resourceId`, `createdAt` |

## 4. Policy-related relationship rules

| Relationship | Policy impact |
| --- | --- |
| `dining_sessions` → `order_headers` | Billing gates phải xem tất cả order trong session |
| `order_items` → `task_items` | Cancellation policy cần biết task status của từng item |
| `preparation_tasks` → `kitchen_issue` | Kitchen issue chặn bill và cần audit |
| `bills` → `bill_lines` → `order_items` | Bill calculation phải trace item nào bị tính/loại |
| `bills.sessionVersion` → `dining_sessions.version` | Bill stale nếu session đổi sau khi bill tạo |
| `audit_events.correlationId` → command request | Giải thích toàn bộ command xuyên service |

Các relationship này là bắt buộc để policy deny có bằng chứng dữ liệu, không chỉ là message chung chung.
