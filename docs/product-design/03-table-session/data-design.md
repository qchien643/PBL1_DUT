# Data Design - Table Session

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `dining_tables` | Bàn vật lý | `id`, `code`, `area`, `capacity`, `status` |
| `dining_sessions` | Phiên phục vụ | `id`, `primaryTableId`, `status`, `guestCount`, `configVersion`, `openedAt`, `closedAt` |
| `dining_session_tables` | Mapping session-bàn | `sessionId`, `tableId`, `role`, `joinedAt`, `leftAt`, `status` |
| `table_status_history` | Lịch sử trạng thái | `tableId`, `fromStatus`, `toStatus`, `actorId`, `createdAt` |
| `table_merge_history` | Lịch sử ghép bàn | `sessionId`, `targetTableId`, `actorId`, `reason` |
| `table_transfer_history` | Lịch sử chuyển bàn | `sessionId`, `fromTableId`, `toTableId`, `actorId`, `reason` |

## Indexes

| Table | Index |
| --- | --- |
| `dining_tables` | `status`, `area` |
| `dining_sessions` | `status`, `primaryTableId` |
| `dining_session_tables` | unique active `tableId` |
| `dining_session_tables` | `sessionId`, `status` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `dining_tables.status` | `CanOpenTablePolicy`, `TableCleanlinessPolicy` | `AVAILABLE/OCCUPIED/BILLING/CLEANING` |
| `dining_sessions.status` | `BillLockPolicy`, order gates | `ACTIVE/BILL_REQUESTED/CLOSED/MERGED` |
| `dining_session_tables.role` | `CanMergeTablePolicy` | primary/secondary table |
| unique active `tableId` | `CanOpenTablePolicy` | Chống double active session |
