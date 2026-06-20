# Commands And Interfaces - Staff Permission

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `Login` | `AuthService.login` | Staff | N/A |
| `CreateStaffUser` | `StaffService.createUser` | Manager | `PermissionPolicy` |
| `AssignRole` | `StaffService.assignRole` | Manager | `PermissionPolicy`, `AuditPolicy` |
| `GetMyPermissions` | `StaffService.getMyPermissions` | Staff | N/A |
| `CheckPermission` | `PermissionPolicy.evaluate` | Internal | N/A |

## ActorContext

| Field | Meaning |
| --- | --- |
| `actorType` | `customer`, `staff`, `system` |
| `actorId` | Staff id or console customer id |
| `roles` | Active role list |
| `branchId` | Branch scope |
| `tableId/sessionId` | Customer scope |

## Policy-aware service flow

```text
authorize(command, actorContext, resource)
→ ActorScopePolicy
→ PermissionPolicy
→ ManagerOverridePolicy if action sensitive
→ return allowed/denied PolicyDecision
```

Service phải đọc role mới nhất từ DB tại thời điểm command, không tin role cached trong CMD.
