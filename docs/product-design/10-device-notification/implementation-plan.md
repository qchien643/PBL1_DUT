# Implementation Plan - Device Notification

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Create notification schema | Tables exist |
| 2 | Implement domain event emitter | Events created |
| 3 | Implement `NotificationPolicy` | Recipients resolved |
| 4 | Implement CMD polling | CMD sees unread notifications |
| 5 | Add idempotency | Duplicate events safe |
| 6 | Integrate key events | Order/task/bill/cancel notifications work |

## Acceptance criteria

- Order submitted notifies staff.
- Task ready notifies staff.
- Bill requested notifies cashier.
- Duplicate notifications are prevented.
