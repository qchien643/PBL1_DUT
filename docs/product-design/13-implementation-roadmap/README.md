# 13 - Implementation Roadmap

## 1. Mục tiêu

Roadmap triển khai code theo thứ tự phụ thuộc, đủ để chia task cho từng phase.

## 2. Phase map

| Phase | Mục tiêu |
| --- | --- |
| 1 | Foundation architecture |
| 2 | Database and seed |
| 3 | Policy layer |
| 4 | Core business workflow |
| 5 | Recommendation AI/ML |
| 6 | Reporting and audit |
| 7 | Demo hardening |

Phase 3 phải triển khai theo `../17-policy-governance/README.md` trước khi hoàn thiện service workflow, để tránh rule bị viết rải rác trong CMD/web/API.

## 3. Tài liệu refactor quan trọng

| File | Nội dung |
| --- | --- |
| [persistence-refactor-plan.md](persistence-refactor-plan.md) | Kế hoạch chuyển persistence từ một file tổng sang nhiều `.txt` theo bảng và repository |

## 4. Definition of done

- Chạy được 4 CMD.
- Demo mở bàn -> order -> bếp -> thanh toán.
- Demo ghép/chuyển bàn.
- Demo hủy món đặt nhầm.
- Demo recommendation fallback/model.
- Demo report/audit.
- Demo policy deny code và audit/notification cho edge case trọng yếu.
