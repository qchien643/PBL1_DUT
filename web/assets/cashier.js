let cashierState = {
  tables: [],
  pendingOrders: [],
  cancelRequests: [],
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
    cashierState.openBills = (await Api.get("/api/bills/open")).bills;
    renderCashier();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

function renderCashier() {
  renderTables();
  renderPendingOrders();
  renderCancelRequests();
  renderBills();
}

function renderTables() {
  const root = document.getElementById("tables");
  root.innerHTML = cashierState.tables.map((table) => `
    <div class="row">
      <div class="row-title">
        <span>${table.code}</span>
        <span class="badge status-${table.state}">${table.state}</span>
      </div>
      <p class="muted">Session #${table.activeSessionId || "-"} · Joined: ${Api.tableText(table.joinedTables)}</p>
    </div>
  `).join("");
}

function renderPendingOrders() {
  const root = document.getElementById("pending-orders");
  if (!cashierState.pendingOrders.length) {
    root.innerHTML = Api.empty("No pending order.");
    return;
  }
  root.innerHTML = cashierState.pendingOrders.map((order) => `
    <div class="row">
      <div class="row-title"><span>Order #${order.id}</span><span>${Api.tableText(order.tableCodes)}</span></div>
      ${(order.items || []).map((item) => `<p class="muted">${Api.escapeHtml(item.name)} x${item.quantity}</p>`).join("")}
      <div class="row-actions">
        <button class="button primary" onclick="acceptOrder(${order.id})">Accept</button>
        <button class="button danger" onclick="rejectOrder(${order.id})">Reject</button>
      </div>
    </div>
  `).join("");
}

function renderCancelRequests() {
  const root = document.getElementById("cancel-requests");
  if (!cashierState.cancelRequests.length) {
    root.innerHTML = Api.empty("No cancel request.");
    return;
  }
  root.innerHTML = cashierState.cancelRequests.map((item) => `
    <div class="row">
      <div class="row-title"><span>Item #${item.orderItemId}</span><span>Order #${item.orderId}</span></div>
      <p class="muted">${Api.escapeHtml(item.name)} x${item.quantity}</p>
      <button class="button primary full" onclick="approveCancel(${item.orderItemId})">Approve cancel</button>
    </div>
  `).join("");
}

function renderBills() {
  const root = document.getElementById("open-bills");
  if (!cashierState.openBills.length) {
    root.innerHTML = Api.empty("No open bill.");
    return;
  }
  root.innerHTML = cashierState.openBills.map((bill) => `
    <div class="row">
      <div class="row-title"><span>Bill #${bill.id}</span><span>${Api.formatMoney(bill.total)}</span></div>
      <p class="muted">Session #${bill.sessionId} · Tables ${Api.tableText(bill.tableCodes)} · Version ${bill.version}</p>
      <div class="row-actions">
        <button class="button primary" onclick="payBill(${bill.id})">Confirm payment</button>
        <button class="button ghost" onclick="reopenBill(${bill.id})">Reopen</button>
      </div>
    </div>
  `).join("");
}

async function openTable() {
  const code = prompt("Table code to open:", "T01");
  if (!code) return;
  await runAction(() => Api.post(`/api/tables/${code}/open`, { actor: "cashier" }), "Table opened.");
}

async function mergeTables() {
  const mainTableCode = prompt("Main table code:", "T01");
  const joinedTableCode = prompt("Table to merge:", "T02");
  if (!mainTableCode || !joinedTableCode) return;
  await runAction(() => Api.post("/api/tables/merge", { mainTableCode, joinedTableCode, actor: "cashier" }), "Tables merged.");
}

async function transferTable() {
  const sourceTableCode = prompt("Current table:", "T01");
  const targetTableCode = prompt("Target table:", "T03");
  if (!sourceTableCode || !targetTableCode) return;
  await runAction(() => Api.post("/api/tables/transfer", { sourceTableCode, targetTableCode, actor: "cashier" }), "Table transferred.");
}

async function markCleaned() {
  const code = prompt("Cleaned table code:", "T01");
  if (!code) return;
  await runAction(() => Api.post(`/api/tables/${code}/cleaned`, { actor: "cashier" }), "Table marked available.");
}

async function acceptOrder(orderId) {
  await runAction(() => Api.post(`/api/orders/${orderId}/accept`, { actor: "cashier" }), "Order accepted.");
}

async function rejectOrder(orderId) {
  await runAction(() => Api.post(`/api/orders/${orderId}/reject`, { actor: "cashier" }), "Order rejected.");
}

async function approveCancel(orderItemId) {
  await runAction(() => Api.post(`/api/order-items/${orderItemId}/cancel-approve`, { actor: "cashier" }), "Cancel approved.");
}

async function payBill(billId) {
  const bill = cashierState.openBills.find((candidate) => candidate.id === billId);
  const paymentMethod = prompt("Payment method:", "cash") || "cash";
  const paidAmount = Number(prompt("Paid amount:", String(bill?.total || 0))) || 0;
  await runAction(() => Api.post(`/api/bills/${billId}/pay`, {
    paymentMethod,
    paidAmount,
    billVersion: bill?.version || 0,
    idempotencyKey: `bill-${billId}-${Date.now()}`,
    actor: "cashier"
  }), "Payment confirmed.");
}

async function reopenBill(billId) {
  await runAction(() => Api.post(`/api/bills/${billId}/reopen`, { actor: "cashier" }), "Bill reopened.");
}

async function resolveKitchenIssue() {
  const issueId = Number(prompt("Kitchen issue id:"));
  if (!issueId) return;
  const resolution = prompt("Resolution: CANCEL_ITEM, REMAKE_SAME_ITEM, KEEP_AND_BILL_WITH_MANAGER_OVERRIDE", "CANCEL_ITEM");
  if (!resolution) return;
  await runAction(() => Api.post(`/api/kitchen/issues/${issueId}/resolve`, { actor: "cashier", resolution }), "Kitchen issue resolved.");
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
