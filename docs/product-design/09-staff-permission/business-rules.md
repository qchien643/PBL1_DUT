# Business Rules - Staff Permission

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| PERM_001 | Command nhạy cảm phải kiểm tra quyền | `PermissionPolicy` | Required |
| PERM_002 | Kitchen không được confirm payment | `PermissionPolicy` | Required |
| PERM_003 | Customer chỉ thao tác session/table của mình | `PermissionPolicy`, `DeviceBindingPolicy` | Required |
| PERM_004 | Manager override hủy món preparing phải audit | `PermissionPolicy`, `AuditPolicy` | Required |
| PERM_005 | Actor phải lưu trong audit event | `AuditPolicy` | Required |

## Permission matrix

| Permission | Manager | Cashier | Waiter | Kitchen | Customer |
| --- | --- | --- | --- | --- | --- |
| `config.update` | Yes | No | No | No | No |
| `menu.update` | Yes | No | No | No | No |
| `table.open` | Yes | Yes | Yes | No | No |
| `order.submit` | No | No | No | No | Yes |
| `order.accept` | Yes | Yes | No | No | No |
| `order.cancel.approve` | Yes | Yes | No | No | No |
| `task.update` | Yes | No | No | Yes | No |
| `payment.confirm` | Yes | Yes | No | No | No |
| `report.view` | Yes | No | No | No | No |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `PermissionPolicy` | Actor có quyền chạy command không? | actorId, roles, permission key, branch scope | `PERMISSION_DENIED` | Audit warning for sensitive deny |
| `ManagerOverridePolicy` | Action này có cần manager override không? | actor role, action type, money/kitchen impact | `MANAGER_OVERRIDE_REQUIRED` | Audit required |
| `RoleSafetyPolicy` | Role/permission change có an toàn không? | current roles, target change, manager count | `ROLE_CHANGE_NOT_SAFE` | Audit critical |
| `ActorScopePolicy` | Actor có thao tác đúng table/station/branch không? | actor context, resource scope | `PERMISSION_DENIED` | Audit if suspicious |

Permission policy luôn chạy trước business policy.
