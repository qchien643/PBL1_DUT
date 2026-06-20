# Data Design - Reporting Audit

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `audit_events` | Audit log | `id`, `eventType`, `actorId`, `resourceType`, `resourceId`, `payload`, `createdAt` |
| `config_versions` | Config history | `branchId`, `version`, `configJson`, `createdBy` |
| `report_snapshots` | Optional cached report | `type`, `date`, `payload` |

## Reporting sources

| Report | Source tables |
| --- | --- |
| Daily revenue | `bills`, `payments` |
| Top selling | `order_items`, `bills` |
| Cancelled items | `cancellation_requests`, `order_item_status_history` |
| Kitchen performance | `preparation_tasks`, `task_status_history` |

## Indexes

| Table | Index |
| --- | --- |
| `audit_events` | `resourceType`, `resourceId`, `createdAt` |
| `audit_events` | `actorId`, `createdAt` |
| `config_versions` | `branchId`, `version` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `audit_events.severity` | `AuditRequiredPolicy` | LOW/MEDIUM/HIGH/CRITICAL |
| `audit_events.before/after` | audit explanation | Cần cho hủy/void/payment/config |
| immutable audit rows | `AuditRetentionPolicy` | Không update/delete |
| paid bill snapshots | `ReportFilterPolicy` | Revenue dựa trên bill paid |
