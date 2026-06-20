# Data Design - Staff Permission

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `staff_users` | Tài khoản nhân viên | `id`, `name`, `username`, `status` |
| `roles` | Vai trò | `id`, `key`, `name` |
| `permissions` | Quyền | `key`, `description` |
| `role_permissions` | Role-permission mapping | `roleId`, `permissionKey` |
| `staff_role_assignments` | Gán role cho staff | `staffId`, `roleId`, `branchId` |

## Indexes

| Table | Index |
| --- | --- |
| `staff_users` | unique `username` |
| `staff_role_assignments` | `staffId`, `branchId` |
| `role_permissions` | unique `roleId`, `permissionKey` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `staff_role_assignments.active` | `PermissionPolicy` | Role có thể đổi giữa ca |
| `role_permissions.permissionKey` | `PermissionPolicy` | Permission key dùng trong command |
| `managerOverrideApprovals` | `ManagerOverridePolicy` | Lưu actor, reason, expiration |
| `branchId/resourceScope` | `ActorScopePolicy` | Chống thao tác sai branch/table/station |
