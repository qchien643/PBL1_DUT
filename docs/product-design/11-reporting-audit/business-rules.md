# Business Rules - Reporting Audit

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| RPT_001 | Chỉ manager xem báo cáo tổng | `ReportAccessPolicy` | Required |
| RPT_002 | Chỉ bill paid tính doanh thu | `ReportingService` | Required |
| RPT_003 | Món cancelled không tính top selling | `ReportingService` | Required |
| AUD_001 | Payment confirmed phải audit | `AuditPolicy` | Required |
| AUD_002 | Hủy món phải audit reason/actor | `AuditPolicy` | Required |
| AUD_003 | Manager override là audit high | `AuditPolicy` | Required |
| AUD_004 | Config/menu price change phải audit | `AuditPolicy` | Required |

## Audit severity

| Severity | Events |
| --- | --- |
| Normal | table opened, order submitted |
| Important | order accepted, item sold out |
| High | payment confirmed, manager override, config changed |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `AuditRequiredPolicy` | Action này có bắt buộc audit không? | action type, actor, money/kitchen impact | `AUDIT_REQUIRED` | Audit must be same transaction if critical |
| `AuditRetentionPolicy` | Audit có được sửa/xóa không? | audit event, actor role | `AUDIT_IMMUTABLE` | Critical audit if tamper attempted |
| `ReportFilterPolicy` | Report tính dữ liệu nào? | bill/order/item status, date range | `REPORT_SCOPE_INVALID` | No notification |
| `AuditForMoneyImpactPolicy` | Action ảnh hưởng tiền đã có audit chưa? | bill/order adjustment, payment | `AUDIT_REQUIRED` | Required |

Audit/reporting dùng dữ liệu đã commit, không tự thay đổi nghiệp vụ.
