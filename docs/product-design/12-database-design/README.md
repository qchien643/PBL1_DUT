# 12 - Database Design

## 1. Mục tiêu

Tổng hợp thiết kế database cho toàn bộ MVP Casual dining. Module này dùng để triển khai migration/DDL và seed data.

## 2. Tài liệu con

| File | Nội dung |
| --- | --- |
| [schema-overview.md](schema-overview.md) | Nhóm schema và nguyên tắc thiết kế |
| [table-groups.md](table-groups.md) | Danh sách bảng theo module |
| [relationships.md](relationships.md) | ERD và quan hệ chính |
| [seed-data.md](seed-data.md) | Dữ liệu seed phục vụ demo |
| [file-storage-strategy.md](file-storage-strategy.md) | Thiết kế lưu nhiều `.txt` theo bảng cho C++ MVP |
| [repository-design.md](repository-design.md) | Repository ownership, contracts và service mapping |

## 3. Nguyên tắc

| Nguyên tắc | Ý nghĩa |
| --- | --- |
| Snapshot order/bill | Không phụ thuộc menu hiện tại |
| State history | Theo dõi thay đổi quan trọng |
| Config version | Session dùng đúng config lúc mở |
| Idempotency | Chống submit trùng |
| Audit | Truy vết actor/action/resource |
| Table-per-file storage | Không gom toàn bộ dữ liệu vào một file tổng |
