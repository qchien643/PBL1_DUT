# Implementation Plan - Table Session

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Create table/session schema | Tables and indexes exist |
| 2 | Seed dining tables | Staff CMD sees tables |
| 3 | Implement `TablePolicy` | Invalid transitions blocked |
| 4 | Implement `openTable` | Creates session and mapping |
| 5 | Implement `mergeTable` | Adds table to session |
| 6 | Implement `transferTable` | Moves primary table safely |
| 7 | Add history/audit | State changes traceable |

## Acceptance criteria

- Một bàn không có hai active session.
- Ghép bàn dùng chung bill.
- Chuyển bàn giữ nguyên order/bill.
