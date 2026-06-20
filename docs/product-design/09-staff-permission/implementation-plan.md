# Implementation Plan - Staff Permission

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Create staff/role schema | Tables exist |
| 2 | Seed roles/permissions | Manager/cashier/waiter/kitchen ready |
| 3 | Implement `PermissionPolicy` | Role checks centralized |
| 4 | Integrate policy into services | Commands blocked when unauthorized |
| 5 | Add actor to audit | Audit includes actor/role |

## Acceptance criteria

- Wrong role cannot execute sensitive command.
- Customer cannot access another table/session.
- Manager override is audited.
