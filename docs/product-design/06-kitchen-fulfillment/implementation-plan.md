# Implementation Plan - Kitchen Fulfillment

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Create station/task schema | Kitchen tables exist |
| 2 | Seed kitchen/bar stations | Kitchen CMD can select station |
| 3 | Implement routing policy | Accepted order splits tasks |
| 4 | Implement task lifecycle | Pending -> preparing -> ready |
| 5 | Integrate cancellation | Cancelled item hidden from task |
| 6 | Add issue reporting | Staff notification created |

## Acceptance criteria

- Food and drink route to correct station.
- Kitchen cannot update wrong station task.
- Ready task notifies staff.
