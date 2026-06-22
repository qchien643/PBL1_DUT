let cashierState = {
  tables: [],
  pendingOrders: [],
  cancelRequests: [],
  openIssues: [],
  openBills: []
};

document.addEventListener("DOMContentLoaded", () => {
  loadCashierData();
  Notifications.startNotificationPolling("cashier", () => loadCashierData());
});

async function loadCashierData() {
  try {
    cashierState.tables = (await Api.get("/api/tables")).tables;
    cashierState.pendingOrders = (await Api.get("/api/orders/pending")).orders;
    cashierState.cancelRequests = (await Api.get("/api/cancel-requests")).items;
    cashierState.openIssues = (await Api.get("/api/kitchen/issues/open")).issues;
    cashierState.openBills = (await Api.get("/api/bills/open")).bills;
    renderCashier();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

function renderCashier() {
  renderTableTools();
  renderTables();
  renderPendingOrders();
  renderCancelRequests();
  renderOpenIssues();
  renderBills();
}

function tableOptions(filter, fallback = "Không có bàn phù hợp") {
  const tables = cashierState.tables.filter(filter);
  if (!tables.length) return `<option value="">${fallback}</option>`;
  return tables.map((table) => `<option value="${Api.escapeHtml(table.code)}">${Api.escapeHtml(table.code)} · ${Api.statusText(table.state)}</option>`).join("");
}

function renderTableTools() {
  const root = document.getElementById("table-tools");
  const availableOptions = tableOptions((table) => table.state === "AVAILABLE", "Không có bàn trống");
  const occupiedOptions = tableOptions((table) => table.activeSessionId, "Không có bàn đang phục vụ");
  const mergeJoinOptions = tableOptions((table) => table.state === "AVAILABLE" || table.activeSessionId, "Không có bàn có thể ghép");
  const cleaningOptions = tableOptions((table) => table.state === "CLEANING", "Không có bàn cần dọn");
  root.innerHTML = `
    <form class="control-panel" onsubmit="openTable(event)">
      <label class="field">
        <span>Mở bàn</span>
        <select name="tableCode" required>${availableOptions}</select>
      </label>
      <button class="button primary" type="submit">Mở bàn</button>
    </form>
    <form class="control-panel" onsubmit="mergeTables(event)">
      <label class="field">
        <span>Bàn chính</span>
        <select name="mainTableCode" required>${occupiedOptions}</select>
      </label>
      <label class="field">
        <span>Bàn ghép thêm</span>
        <select name="joinedTableCode" required>${mergeJoinOptions}</select>
      </label>
      <button class="button ghost" type="submit">Ghép bàn</button>
    </form>
    <form class="control-panel" onsubmit="transferTable(event)">
      <label class="field">
        <span>Chuyển từ bàn</span>
        <select name="sourceTableCode" required>${occupiedOptions}</select>
      </label>
      <label class="field">
        <span>Sang bàn</span>
        <select name="targetTableCode" required>${availableOptions}</select>
      </label>
      <button class="button ghost" type="submit">Chuyển bàn</button>
    </form>
    <form class="control-panel" onsubmit="markCleaned(event)">
      <label class="field">
        <span>Bàn đã dọn</span>
        <select name="tableCode" required>${cleaningOptions}</select>
      </label>
      <button class="button ghost" type="submit">Xác nhận đã dọn</button>
    </form>
  `;
}

function renderTables() {
  const root = document.getElementById("tables");
  root.innerHTML = cashierState.tables.map((table) => `
    <div class="row">
      <div class="row-title">
        <span>${table.code}</span>
        <span class="badge status-${table.state}">${Api.statusText(table.state)}</span>
      </div>
      <p class="muted">Phiên #${table.activeSessionId || "-"} · Bàn ghép: ${Api.tableText(table.joinedTables)}</p>
    </div>
  `).join("");
}

function renderPendingOrders() {
  const root = document.getElementById("pending-orders");
  if (!cashierState.pendingOrders.length) {
    root.innerHTML = Api.empty("Không có đơn chờ duyệt.");
    return;
  }
  root.innerHTML = cashierState.pendingOrders.map((order) => `
    <div class="row">
      <div class="row-title"><span>Đơn #${order.id}</span><span>Bàn ${Api.tableText(order.tableCodes)}</span></div>
      ${(order.items || []).map((item) => `<p class="muted">${Api.escapeHtml(Api.itemNameText(item.name))} x${item.quantity}</p>`).join("")}
      <div class="row-actions">
        <button class="button primary" onclick="acceptOrder(${order.id})">Duyệt đơn</button>
        <button class="button danger" onclick="rejectOrder(${order.id})">Từ chối</button>
      </div>
    </div>
  `).join("");
}

function renderCancelRequests() {
  const root = document.getElementById("cancel-requests");
  if (!cashierState.cancelRequests.length) {
    root.innerHTML = Api.empty("Không có yêu cầu hủy món.");
    return;
  }
  root.innerHTML = cashierState.cancelRequests.map((item) => `
    <div class="row">
      <div class="row-title"><span>Món #${item.orderItemId}</span><span>Đơn #${item.orderId}</span></div>
      <p class="muted">${Api.escapeHtml(Api.itemNameText(item.name))} x${item.quantity}</p>
      <button class="button primary full" onclick="approveCancel(${item.orderItemId})">Đồng ý hủy món</button>
    </div>
  `).join("");
}

function renderOpenIssues() {
  const root = document.getElementById("kitchen-issues");
  if (!cashierState.openIssues.length) {
    root.innerHTML = Api.empty("Không có sự cố cần xử lý.");
    return;
  }
  root.innerHTML = cashierState.openIssues.map((issue) => `
    <div class="row issue-card">
      <div class="row-title">
        <span>Sự cố #${issue.id}</span>
        <span class="badge status-ISSUE">${Api.stationText(issue.station)}</span>
      </div>
      <div class="issue-summary">
        <strong>${Api.escapeHtml(Api.itemNameText(issue.itemName))} x${issue.quantity}</strong>
        <span>Bàn ${Api.tableText(issue.tableCodes)}</span>
        <span>Mã món bếp #${issue.taskId}</span>
      </div>
      <p class="muted">Lý do: ${Api.escapeHtml(issue.reason)}</p>
      <div class="issue-actions">
        <button class="button danger" onclick="resolveKitchenIssueChoice(${issue.id}, 'CANCEL_ITEM')">Hủy món, không tính tiền</button>
        <button class="button primary" onclick="resolveKitchenIssueChoice(${issue.id}, 'REMAKE_SAME_ITEM')">Làm lại món</button>
        <button class="button secondary" onclick="resolveKitchenIssueChoice(${issue.id}, 'KEEP_AND_BILL_WITH_MANAGER_OVERRIDE')">Giữ món và tính tiền</button>
      </div>
    </div>
  `).join("");
}

function renderBills() {
  const root = document.getElementById("open-bills");
  if (!cashierState.openBills.length) {
    root.innerHTML = Api.empty("Không có hóa đơn chờ thanh toán.");
    return;
  }
  root.innerHTML = cashierState.openBills.map((bill) => `
    <div class="row">
      <div class="row-title"><span>Hóa đơn #${bill.id}</span><span>${Api.formatMoney(bill.total)}</span></div>
      <p class="muted">Phiên #${bill.sessionId} · Bàn ${Api.tableText(bill.tableCodes)} · Lần cập nhật ${bill.version}</p>
      <form class="payment-form" onsubmit="payBill(event, ${bill.id})">
        <label class="field">
          <span>Phương thức</span>
          <select name="paymentMethod">
            <option value="cash">Tiền mặt</option>
            <option value="bank_transfer">Chuyển khoản</option>
            <option value="card">Thẻ</option>
            <option value="e_wallet">Ví điện tử</option>
          </select>
        </label>
        <label class="field">
          <span>Tiền nhận</span>
          <input name="paidAmount" type="number" min="0" step="1000" value="${bill.total}" required />
        </label>
        <label class="field">
          <span>Mã giao dịch hoặc ghi chú</span>
          <input name="paymentReference" type="text" placeholder="Ví dụ: mã chuyển khoản" />
        </label>
        <div class="row-actions">
          <button class="button primary" type="submit">Xác nhận thanh toán</button>
          <button class="button ghost" type="button" onclick="reopenBill(${bill.id})">Mở lại hóa đơn</button>
        </div>
      </form>
    </div>
  `).join("");
}

async function openTable(event) {
  event.preventDefault();
  const code = new FormData(event.currentTarget).get("tableCode");
  await runAction(() => Api.post(`/api/tables/${code}/open`, { actor: "cashier" }), "Đã mở bàn.");
}

async function mergeTables(event) {
  event.preventDefault();
  const form = new FormData(event.currentTarget);
  const mainTableCode = form.get("mainTableCode");
  const joinedTableCode = form.get("joinedTableCode");
  if (!mainTableCode || !joinedTableCode) return;
  if (mainTableCode === joinedTableCode) {
    Notifications.showToast("Vui lòng chọn hai bàn khác nhau để ghép.");
    return;
  }
  await runAction(() => Api.post("/api/tables/merge", { mainTableCode, joinedTableCode, actor: "cashier" }), "Đã ghép bàn.");
}

async function transferTable(event) {
  event.preventDefault();
  const form = new FormData(event.currentTarget);
  const sourceTableCode = form.get("sourceTableCode");
  const targetTableCode = form.get("targetTableCode");
  if (!sourceTableCode || !targetTableCode) return;
  if (sourceTableCode === targetTableCode) {
    Notifications.showToast("Vui lòng chọn bàn chuyển đến khác bàn hiện tại.");
    return;
  }
  await runAction(() => Api.post("/api/tables/transfer", { sourceTableCode, targetTableCode, actor: "cashier" }), "Đã chuyển bàn.");
}

async function markCleaned(event) {
  event.preventDefault();
  const code = new FormData(event.currentTarget).get("tableCode");
  if (!code) return;
  await runAction(() => Api.post(`/api/tables/${code}/cleaned`, { actor: "cashier" }), "Đã đưa bàn về trạng thái trống.");
}

async function acceptOrder(orderId) {
  await runAction(() => Api.post(`/api/orders/${orderId}/accept`, { actor: "cashier" }), "Đã duyệt đơn món.");
}

async function rejectOrder(orderId) {
  await runAction(() => Api.post(`/api/orders/${orderId}/reject`, { actor: "cashier" }), "Đã từ chối đơn món.");
}

async function approveCancel(orderItemId) {
  await runAction(() => Api.post(`/api/order-items/${orderItemId}/cancel-approve`, { actor: "cashier" }), "Đã đồng ý hủy món.");
}

async function payBill(event, billId) {
  event.preventDefault();
  const bill = cashierState.openBills.find((candidate) => candidate.id === billId);
  const form = new FormData(event.currentTarget);
  const paymentMethod = form.get("paymentMethod") || "cash";
  const paymentReference = String(form.get("paymentReference") || "").trim();
  const methodLabel = paymentReference ? `${paymentMethod}:${paymentReference}` : paymentMethod;
  const paidAmount = Number(form.get("paidAmount")) || 0;
  await runAction(() => Api.post(`/api/bills/${billId}/pay`, {
    paymentMethod: methodLabel,
    paidAmount,
    billVersion: bill?.version || 0,
    idempotencyKey: `bill-${billId}-${Date.now()}`,
    actor: "cashier"
  }), "Đã xác nhận thanh toán.");
}

async function reopenBill(billId) {
  await runAction(() => Api.post(`/api/bills/${billId}/reopen`, { actor: "cashier" }), "Đã mở lại hóa đơn.");
}

async function resolveKitchenIssueChoice(issueId, resolution) {
  if (!issueId || !resolution) return;
  await runAction(() => Api.post(`/api/kitchen/issues/${issueId}/resolve`, { actor: "cashier", resolution }), "Đã xử lý sự cố.");
}

async function runAction(action, successMessage) {
  try {
    await action();
    Notifications.showToast(successMessage);
    await loadCashierData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}
