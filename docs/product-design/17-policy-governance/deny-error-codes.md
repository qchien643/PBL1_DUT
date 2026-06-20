# Deny And Error Codes

## 1. API/Business Error Format

Mọi lỗi nghiệp vụ do policy deny nên trả về cùng format:

```json
{
  "success": false,
  "error": {
    "code": "BILL_BLOCKED_BY_ACTIVE_WORK",
    "message": "Chưa thể thanh toán vì còn món chưa xử lý xong.",
    "requiredAction": "RESOLVE_ORDER_OR_KITCHEN_WORK",
    "context": {
      "sessionId": "S-001",
      "blockingItems": ["OI-001", "TASK-002"]
    }
  },
  "correlationId": "REQ-20260620-0001"
}
```

## 2. Code Categories

| Prefix | Nhóm |
|---|---|
| `CONFIG_` | Restaurant configuration |
| `CONTEXT_` | Console/web actor context |
| `PERMISSION_` | Staff/role permission |
| `TABLE_` | Table/session |
| `MENU_`, `ITEM_`, `MODIFIER_` | Menu/inventory |
| `ORDER_`, `CANCEL_` | Order/cancellation |
| `KITCHEN_`, `TASK_` | Kitchen fulfillment |
| `BILL_`, `PAYMENT_` | Billing/payment |
| `RECOMMENDATION_` | AI/ML recommendation |
| `NOTIFICATION_` | Device notification |
| `AUDIT_`, `REPORT_` | Audit/reporting |

## 3. Core Deny Codes

| Code | Required action | User-facing message |
|---|---|---|
| `UNSUPPORTED_RESTAURANT_PROFILE` | Use casual dining config | Hệ thống chỉ hỗ trợ Casual dining trong MVP. |
| `CONFIG_VERSION_STALE` | Reload config | Cấu hình đã thay đổi, vui lòng tải lại. |
| `FEATURE_DISABLED` | Hide/disable feature | Tính năng này đang tắt cho chi nhánh. |
| `INVALID_ACTOR_CONTEXT` | Rebind console/device | Ngữ cảnh màn hình không hợp lệ. |
| `PERMISSION_DENIED` | Login/switch actor | Bạn không có quyền thực hiện thao tác này. |
| `MANAGER_OVERRIDE_REQUIRED` | Ask manager approval | Thao tác này cần quản lý xác nhận. |
| `TABLE_NOT_AVAILABLE` | Choose available table | Bàn chưa sẵn sàng để mở. |
| `TABLE_ALREADY_ACTIVE` | Use current session | Bàn đang có khách. |
| `TARGET_TABLE_OCCUPIED` | Choose another table or merge | Bàn đích đang có khách. |
| `TABLE_NOT_CLEANED` | Mark cleaned first | Bàn cần được dọn trước khi mở lại. |
| `SESSION_LOCKED_FOR_BILLING` | Reopen/void bill | Bàn đang ở trạng thái thanh toán. |
| `ITEM_NOT_ORDERABLE` | Remove item | Món hiện không được bán. |
| `ITEM_UNAVAILABLE` | Choose another item | Món hiện không thể phục vụ. |
| `ITEM_UNAVAILABLE_REQUIRES_CUSTOMER_DECISION` | Ask customer to modify order | Món vừa hết, cần khách xác nhận lại. |
| `MODIFIER_UNAVAILABLE` | Remove modifier | Tùy chọn món không còn hợp lệ. |
| `PRICE_CHANGED_REQUIRES_CONFIRMATION` | Confirm new price | Giá món đã thay đổi, cần xác nhận lại. |
| `TABLE_NOT_ACTIVE` | Open table first | Bàn chưa được kích hoạt. |
| `EMPTY_CART` | Add item | Giỏ món đang trống. |
| `DUPLICATE_REQUEST` | Return existing result | Yêu cầu đã được xử lý trước đó. |
| `ORDER_NOT_ACCEPTABLE` | Reload order | Đơn không thể duyệt ở trạng thái hiện tại. |
| `ORDER_NOT_READY_FOR_FULFILLMENT` | Resolve order decision | Đơn chưa đủ điều kiện gửi xuống bếp. |
| `CANCEL_TOO_LATE` | Call staff/manager | Món đang làm hoặc đã xong nên không thể hủy tự động. |
| `KITCHEN_STATION_NOT_CONFIGURED` | Fix menu routing | Món chưa được cấu hình station bếp/bar. |
| `KITCHEN_TASK_INVALID_STATE` | Reload task | Task bếp không ở trạng thái hợp lệ. |
| `KITCHEN_ISSUE_NOT_ALLOWED` | Reload task | Không thể báo issue cho task này. |
| `ITEM_NOT_SERVED_YET` | Wait served confirmation | Món đã xong nhưng chưa xác nhận phục vụ. |
| `BILL_BLOCKED_BY_ACTIVE_WORK` | Resolve blockers | Chưa thể thanh toán vì còn món/order chưa xử lý. |
| `BILL_ALREADY_EXISTS` | Use existing bill | Bill của session này đã tồn tại. |
| `BILL_STALE_RECALCULATE_REQUIRED` | Recalculate bill | Bill đã thay đổi, vui lòng tính lại. |
| `BILL_NOT_OPEN` | Reload bill | Bill không ở trạng thái có thể thanh toán. |
| `PAYMENT_AMOUNT_INVALID` | Correct amount | Số tiền thanh toán chưa hợp lệ. |
| `PAYMENT_ALREADY_COMPLETED` | Show paid bill | Bill đã được thanh toán. |
| `RECOMMENDATION_NOT_ALLOWED` | Hide recommendations | Không gợi ý món ở trạng thái hiện tại. |
| `RECOMMENDATION_MODEL_UNAVAILABLE` | Use fallback | Model chưa sẵn sàng, dùng gợi ý dự phòng. |
| `NOTIFICATION_SYNC_REQUIRED` | Reload state | Cần đồng bộ lại trạng thái mới nhất. |
| `AUDIT_REQUIRED` | Provide actor/reason | Cần ghi nhận lý do và người thực hiện. |
| `AUDIT_IMMUTABLE` | Do not edit audit | Audit log không được chỉnh sửa. |

## 4. Required Action Values

| Value | Ý nghĩa |
|---|---|
| `RELOAD_STATE` | UI/CMD gọi lại API/state mới nhất |
| `OPEN_TABLE_FIRST` | Cashier/waiter mở bàn |
| `ASK_CUSTOMER_TO_MODIFY_ORDER` | Khách chọn bỏ/thay/hủy order |
| `CALL_STAFF` | Khách cần nhân viên hỗ trợ |
| `ASK_MANAGER_OVERRIDE` | Cần manager xác nhận |
| `RESOLVE_KITCHEN_ISSUE` | Bếp/cashier xử lý issue |
| `RESOLVE_ORDER_OR_KITCHEN_WORK` | Xử lý order/task/cancel blocker trước bill |
| `RECALCULATE_BILL` | Tạo lại bill snapshot |
| `FIX_CONFIGURATION` | Manager sửa cấu hình |

