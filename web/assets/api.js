(function () {
  async function request(path, options = {}) {
    const response = await fetch(path, {
      headers: {
        "Content-Type": "application/json",
        ...(options.headers || {})
      },
      ...options
    });
    const payload = await response.json();
    if (!payload.ok) {
      const error = new Error(payload.error?.message || "Thao tác không thành công");
      error.code = payload.error?.code || "REQUEST_FAILED";
      error.requiredAction = payload.error?.requiredAction || "";
      error.context = payload.error?.context || {};
      error.correlationId = payload.correlationId || "";
      throw error;
    }
    return payload.data;
  }

  function formatMoney(value) {
    return `${Number(value || 0).toLocaleString("vi-VN")} đ`;
  }

  function escapeHtml(value) {
    return String(value ?? "")
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;");
  }

  function tableText(codes) {
    return Array.isArray(codes) && codes.length ? codes.join("+") : "-";
  }

  function empty(message) {
    return `<div class="empty">${escapeHtml(message)}</div>`;
  }

  const statusLabels = {
    ACTIVE: "Đang phục vụ",
    AVAILABLE: "Trống",
    ACCEPTED: "Đã nhận",
    BILL_REQUESTED: "Chờ thanh toán",
    CANCELLED: "Đã hủy",
    CANCEL_REQUESTED: "Chờ duyệt hủy",
    CLEANING: "Chờ dọn",
    CLOSED: "Đã đóng",
    COMPLETED: "Hoàn tất",
    ISSUE: "Có sự cố",
    ISSUE_PENDING_DECISION: "Chờ xử lý sự cố",
    NEEDS_CUSTOMER_CONFIRMATION: "Chờ khách xác nhận",
    OCCUPIED: "Đang dùng",
    OPEN: "Đang mở",
    PAID: "Đã thanh toán",
    PENDING: "Chờ làm",
    PREPARING: "Đang làm",
    READY: "Sẵn sàng",
    REJECTED: "Đã từ chối",
    SERVED: "Đã phục vụ",
    SOLD_OUT: "Hết món",
    STALE: "Cần tính lại",
    SUBMITTED: "Chờ duyệt"
  };

  const stationLabels = {
    kitchen: "Bếp",
    bar: "Quầy nước"
  };

  const categoryLabels = {
    food: "Món ăn",
    drink: "Đồ uống",
    dessert: "Tráng miệng"
  };

  const roleLabels = {
    manager: "Quản lý",
    cashier: "Thu ngân",
    waiter: "Phục vụ",
    kitchen: "Bếp",
    bar: "Quầy nước",
    customer: "Khách",
    system: "Hệ thống"
  };

  const itemNameLabels = {
    "Grilled Chicken Rice": "Cơm gà nướng",
    "Beef Noodle Bowl": "Bún bò",
    "Seafood Fried Rice": "Cơm chiên hải sản",
    "Vegetable Spring Rolls": "Gỏi cuốn rau củ",
    "Iced Tea": "Trà đá",
    "Orange Juice": "Nước cam",
    "Caramel Flan": "Bánh flan",
    "Fruit Yogurt": "Sữa chua trái cây",
    "Unknown item": "Món không xác định",
    "Item": "Món"
  };

  const errorLabels = {
    BILL_BLOCKED_BY_ACTIVE_WORK: "Chưa thể lập hóa đơn vì còn món hoặc yêu cầu cần xử lý.",
    BILL_REQUIRES_ORDER: "Cần gửi ít nhất một đơn món trước khi yêu cầu thanh toán.",
    BILL_STALE_RECALCULATE_REQUIRED: "Hóa đơn đã thay đổi, vui lòng tải lại hoặc lập hóa đơn mới.",
    ORDER_CART_EMPTY: "Giỏ món đang trống.",
    ORDER_ITEM_CANCEL_NOT_ALLOWED: "Món này không thể yêu cầu hủy ở trạng thái hiện tại.",
    ORDER_ITEM_SESSION_MISMATCH: "Món không thuộc phiên bàn hiện tại.",
    ORDER_NO_ORDERABLE_ITEM: "Không có món nào có thể đặt.",
    PAYMENT_AMOUNT_INVALID: "Số tiền nhận chưa đủ để thanh toán hóa đơn.",
    PERMISSION_DENIED: "Tài khoản hiện tại không có quyền thực hiện thao tác này.",
    SESSION_NOT_ACTIVE: "Bàn chưa được mở hoặc phiên bàn không còn hoạt động.",
    UNAVAILABLE_ITEM_REQUIRES_CUSTOMER_CONFIRMATION: "Có món đã hết, cần khách xác nhận cách xử lý."
  };

  const actionLabels = {
    ADD_ITEMS_TO_CART: "Thêm món vào giỏ",
    ASK_CUSTOMER_TO_MODIFY_ORDER: "Khách cần chọn cách xử lý đơn",
    CHOOSE_ANOTHER_ITEM: "Chọn món khác",
    CHOOSE_VALID_DECISION: "Chọn cách xử lý hợp lệ",
    CHOOSE_VALID_RESOLUTION: "Chọn cách xử lý hợp lệ",
    CONTACT_MANAGER: "Liên hệ quản lý",
    CONTACT_STAFF: "Liên hệ nhân viên",
    ENTER_REASON: "Nhập lý do",
    ENTER_SUFFICIENT_AMOUNT: "Nhập đủ số tiền",
    OPEN_TABLE_FIRST: "Mở bàn trước",
    RECALCULATE_BILL: "Tính lại hóa đơn",
    RELOAD_BILL: "Tải lại hóa đơn",
    RELOAD_MENU: "Tải lại thực đơn",
    RELOAD_ORDER: "Tải lại đơn",
    RELOAD_SESSION: "Tải lại bàn",
    RELOAD_TASKS: "Tải lại danh sách món",
    REOPEN_BILL_FIRST: "Mở lại hóa đơn trước",
    RESOLVE_ORDER_OR_KITCHEN_WORK: "Xử lý hết đơn và món đang làm",
    SUBMIT_ORDER_FIRST: "Gửi đơn món trước"
  };

  function statusText(value) {
    return statusLabels[value] || String(value ?? "-");
  }

  function stationText(value) {
    return stationLabels[value] || String(value ?? "-");
  }

  function categoryText(value) {
    return categoryLabels[value] || String(value ?? "-");
  }

  function itemNameText(value) {
    return itemNameLabels[value] || String(value ?? "-");
  }

  function noteText(value) {
    const text = String(value ?? "");
    if (!text) return "";
    if (text.includes("Sold out when cashier tried to accept")) return "Món đã hết khi thu ngân duyệt đơn.";
    if (text.includes("Customer resolved sold-out item")) return "Khách đã xử lý món hết.";
    if (text.includes("Replacement for sold-out item")) return "Món thay thế cho món đã hết.";
    if (text.includes("Customer cancelled after sold-out confirmation")) return "Khách hủy sau khi xác nhận món hết.";
    if (text.includes("Cancelled after kitchen issue")) return "Đã hủy sau sự cố.";
    if (text.includes("Remake after kitchen issue")) return "Làm lại sau sự cố.";
    if (text.includes("Manager override after issue")) return "Quản lý cho phép giữ món sau sự cố.";
    return text;
  }

  function roleText(value) {
    return roleLabels[value] || String(value ?? "-");
  }

  function auditText(event) {
    const actionLabels = {
      AcceptOrder: "Duyệt đơn món",
      ApproveCancelItem: "Đồng ý hủy món",
      CompleteKitchenTask: "Báo món đã xong",
      ConfirmPayment: "Xác nhận thanh toán",
      CreateBill: "Lập hóa đơn",
      CustomerCancelOrder: "Khách hủy đơn",
      CustomerResolveSoldOut: "Khách xử lý món đã hết",
      MarkTaskServed: "Ghi nhận món đã phục vụ",
      OpenedSession: "Mở bàn",
      RejectOrder: "Từ chối đơn món",
      ReportKitchenIssue: "Báo sự cố bếp",
      RequestCancelItem: "Khách yêu cầu hủy món",
      ResolveKitchenIssue: "Xử lý sự cố bếp",
      StartKitchenTask: "Bắt đầu làm món",
      SubmitOrder: "Khách gửi đơn món"
    };
    if (event?.action && actionLabels[event.action]) {
      const entity = event.entityId ? ` #${event.entityId}` : "";
      return `${actionLabels[event.action]}${entity}`;
    }
    return "Có thao tác mới trong hệ thống.";
  }

  function notificationText(event) {
    const labels = {
      BILL_PAID: "Hóa đơn đã được thanh toán.",
      BILL_REOPENED: "Hóa đơn đã được mở lại.",
      BILL_REQUESTED: "Khách vừa yêu cầu thanh toán.",
      CANCEL_APPROVED: "Yêu cầu hủy món đã được đồng ý.",
      CANCEL_REQUESTED: "Khách vừa yêu cầu hủy món.",
      CUSTOMER_DECISION_APPLIED: "Khách đã cập nhật lựa chọn cho đơn món.",
      KITCHEN_ISSUE: "Bếp hoặc quầy nước vừa báo sự cố.",
      KITCHEN_ISSUE_RESOLVED: "Sự cố đã được xử lý.",
      MENU_CHANGED: "Tình trạng món vừa được cập nhật.",
      NEW_ORDER: "Có đơn món mới cần duyệt.",
      ORDER_ACCEPTED: "Đơn món đã được thu ngân duyệt.",
      ORDER_ITEM_CANCELLED: "Một món đã được hủy.",
      ORDER_NEEDS_CUSTOMER_CONFIRMATION: "Có món đã hết, cần khách xác nhận.",
      ORDER_REJECTED: "Đơn món đã bị từ chối.",
      TABLE_OPENED: "Bàn đã được mở.",
      TASK_CREATED: "Có món mới cần chuẩn bị.",
      TASK_READY: "Có món đã làm xong.",
      TASK_SERVED: "Món đã được phục vụ.",
      TASK_STARTED: "Có món đang được chuẩn bị."
    };
    return labels[event?.type] || "Có cập nhật mới.";
  }

  function errorText(error) {
    const message = errorLabels[error.code] || "Thao tác không thành công. Vui lòng tải lại màn hình rồi thử lại.";
    const action = error.requiredAction ? ` Cần làm: ${actionLabels[error.requiredAction] || error.requiredAction}.` : "";
    return `${message}${action}`;
  }

  window.Api = {
    get: (path) => request(path),
    post: (path, body = {}) => request(path, { method: "POST", body: JSON.stringify(body) }),
    patch: (path, body = {}) => request(path, { method: "PATCH", body: JSON.stringify(body) }),
    formatMoney,
    escapeHtml,
    tableText,
    statusText,
    stationText,
    categoryText,
    itemNameText,
    noteText,
    roleText,
    auditText,
    notificationText,
    empty,
    errorText
  };
})();
