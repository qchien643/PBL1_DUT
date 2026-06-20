# Edge Cases - Reporting Audit

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Bill unpaid bị tính revenue | Query chỉ lấy paid bill | Reporting |
| Cancelled item tính top selling | Filter cancelled | Reporting |
| Audit fail sau action quan trọng | Same transaction for critical actions | Data integrity |
| Staff phủ nhận hủy món | Audit has actor/reason/timestamp | Governance |
| Session ghép bàn | Report theo session, không nhân đôi bàn | Reporting |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Payment không có audit | `AuditRequiredPolicy` | Reject/rollback critical action | Required | Manager |
| Audit bị sửa/xóa | `AuditRetentionPolicy` | Deny, ghi tamper attempt | Critical | Manager |
| Top-selling tính item cancelled | `ReportFilterPolicy` | Exclude non-billable item | No | No |
| Session merge bị đếm doanh thu hai lần | `ReportFilterPolicy` | Report theo final session/bill | No | No |
