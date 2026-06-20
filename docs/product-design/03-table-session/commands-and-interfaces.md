# Commands And Interfaces - Table Session

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `OpenTable` | `TableService.openTable` | Cashier/Staff | `PermissionPolicy`, `TablePolicy` |
| `MergeTable` | `TableService.mergeTable` | Cashier/Staff | `TablePolicy` |
| `TransferTable` | `TableService.transferTable` | Cashier/Staff | `TablePolicy` |
| `RequestService` | `TableService.requestService` | Customer | `TablePolicy` |
| `RequestBill` | `PaymentService.requestBill` | Customer/Staff | `PaymentPolicy` |
| `MarkTableCleaned` | `TableService.markCleaned` | Staff | `TablePolicy` |

## Key inputs

| Command | Required input |
| --- | --- |
| `OpenTable` | `tableId`, `guestCount`, `actorId` |
| `MergeTable` | `sessionId`, `targetTableId`, `reason` |
| `TransferTable` | `sessionId`, `fromTableId`, `toTableId`, `reason` |

## Policy-aware service flow

```text
Open/Merge/Transfer command
→ PermissionPolicy
→ Table policy contract
→ State transition on session/table mapping
→ Audit table/session event
→ Notify affected table/cashier screens
```

Denied commands phải trả deny code từ `../17-policy-governance/deny-error-codes.md`.
