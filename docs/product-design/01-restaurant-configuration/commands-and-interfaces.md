# Commands And Interfaces - Restaurant Configuration

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `ViewBranchConfig` | `ConfigurationService.getActiveConfig` | Manager | `PermissionPolicy` |
| `UpdateBranchConfig` | `ConfigurationService.updateConfig` | Manager | `PermissionPolicy`, `BranchConfigPolicy` |
| `ToggleFeature` | `ConfigurationService.toggleFeature` | Manager | `FeaturePolicy` |
| `ActivateConfigVersion` | `ConfigurationService.activateVersion` | Manager | `BranchConfigPolicy`, `AuditPolicy` |
| `PreviewWorkflow` | `ConfigurationService.previewWorkflow` | Manager | `FeaturePolicy` |

## Input/output

| Interface | Input | Output |
| --- | --- | --- |
| `updateConfig` | `branchId`, `patch`, `actorId` | `configVersion`, validation errors |
| `toggleFeature` | `featureKey`, `enabled` | updated feature flag |
| `getActiveConfig` | `branchId` | active `BranchConfig` |

## Policy-aware service flow

```text
Command
→ PermissionPolicy
→ RestaurantProfilePolicy / ConfigVersionPolicy / FeatureFlagPolicy
→ Persist config version
→ Audit config_changed
→ Notify affected runtime contexts
```

Mọi command trả lỗi theo `../17-policy-governance/deny-error-codes.md`.
