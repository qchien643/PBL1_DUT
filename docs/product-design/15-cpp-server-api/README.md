# C++ Server, API And Notification Design

Module này chỉ tập trung vào **backend/server/API/notification** cho hệ thống đặt món casual dining.

Frontend UI/UX được tách riêng ở module [14-web-frontend-ui-ux](../14-web-frontend-ui-ux/README.md).

## 1. Mục Tiêu

- Thêm server trung tâm để đồng bộ trạng thái giữa các màn hình.
- Cung cấp REST API cho frontend HTML/CSS/JS.
- Giữ business service và policy hiện có.
- Thêm notification store và polling API.

## 2. Phạm Vi Server

| Nhóm | Quyết định |
|---|---|
| Runtime | `restaurant_mvp server` |
| Port | `localhost:8080` |
| Server lib đề xuất | `cpp-httplib` |
| API format | JSON |
| Static file serving | Có, phục vụ folder `web/` |
| Realtime MVP | Polling API |
| Database MVP | File database hiện tại hoặc SQLite nâng cấp |

## 3. Tài Liệu Trong Module

| File | Nội dung |
|---|---|
| [server-architecture.md](server-architecture.md) | Layer server, request flow, mapping code |
| [api-specification.md](api-specification.md) | REST API request/response |
| [notification-polling.md](notification-polling.md) | Notification record, channel, event type, polling |
| [implementation-plan.md](implementation-plan.md) | Kế hoạch triển khai server/API |

## 4. Server Không Làm Gì

- Không chứa layout UI.
- Không render HTML động phức tạp.
- Không thay frontend quyết định UX.
- Không dùng WebSocket trong MVP nếu polling đủ dùng.

Server là nơi xử lý nghiệp vụ và dữ liệu thật.
