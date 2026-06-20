# Implementation Plan - Restaurant Configuration

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Tạo schema config | Có bảng `branch_configs`, `feature_flags` |
| 2 | Seed default Casual dining config | App load được config active |
| 3 | Implement repository | CRUD config/version/feature |
| 4 | Implement `BranchConfigPolicy` | Validate profile và required fields |
| 5 | Implement `ConfigurationService` | CMD gọi được view/update/activate |
| 6 | Add audit event | Config change được log |

## Acceptance criteria

- Không thể activate config không phải `casual_dining`.
- Không thể bật reservation/payment gateway trong MVP.
- Config change không làm sai session đang active.
