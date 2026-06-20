const customerState = {
  tableCode: new URLSearchParams(location.search).get("table") || "T01",
  session: null,
  menuItems: [],
  cart: [],
  orders: [],
  recommendations: []
};

document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("page-title").textContent = `Table ${customerState.tableCode}`;
  loadCustomerData();
  Notifications.startNotificationPolling(`customer:${customerState.tableCode}`, () => loadCustomerData());
});

async function loadCustomerData() {
  try {
    const sessionData = await Api.get(`/api/tables/${customerState.tableCode}/session`);
    customerState.session = sessionData.hasActiveSession ? sessionData.session : null;
    customerState.menuItems = (await Api.get("/api/menu")).items;
    customerState.orders = customerState.session ? (await Api.get(`/api/sessions/${customerState.session.id}/orders`)).orders : [];
    customerState.recommendations = customerState.session ? (await Api.get(`/api/sessions/${customerState.session.id}/recommendations`)).items : [];
    renderCustomer();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

function renderCustomer() {
  document.getElementById("session-banner").textContent = customerState.session
    ? `Session #${customerState.session.id} · ${customerState.session.status}`
    : "This table has not been opened by cashier yet.";
  renderMenu();
  renderCart();
  renderOrders();
  renderRecommendations();
}

function renderMenu() {
  const grid = document.getElementById("menu-grid");
  grid.innerHTML = customerState.menuItems.map((item) => `
    <article class="card">
      <div class="row-title">
        <span>${Api.escapeHtml(item.name)}</span>
        <span class="badge status-${item.availabilityStatus}">${item.availabilityStatus}</span>
      </div>
      <p class="muted">${Api.escapeHtml(item.category)} · ${Api.formatMoney(item.price)} · ${Api.escapeHtml(item.station)}</p>
      <button class="button primary full" ${!customerState.session || item.availabilityStatus !== "AVAILABLE" ? "disabled" : ""}
        onclick="addToCart(${item.id})">Add</button>
    </article>
  `).join("");
}

function addToCart(itemId) {
  const item = customerState.menuItems.find((candidate) => candidate.id === itemId);
  if (!item) return;
  const existing = customerState.cart.find((line) => line.menuItemId === itemId);
  if (existing) existing.quantity += 1;
  else customerState.cart.push({ menuItemId: itemId, quantity: 1 });
  Notifications.showToast(`Added ${item.name}`);
  renderCart();
}

function renderCart() {
  const lines = document.getElementById("cart-lines");
  if (!customerState.cart.length) {
    lines.innerHTML = Api.empty("Cart is empty.");
  } else {
    lines.innerHTML = customerState.cart.map((line) => {
      const item = customerState.menuItems.find((candidate) => candidate.id === line.menuItemId);
      return `<div class="row">
        <div class="row-title"><span>${Api.escapeHtml(item?.name || "Unknown item")}</span><span>x${line.quantity}</span></div>
        <div class="row-actions">
          <button class="button ghost" onclick="changeQty(${line.menuItemId}, 1)">+</button>
          <button class="button ghost" onclick="changeQty(${line.menuItemId}, -1)">-</button>
        </div>
      </div>`;
    }).join("");
  }
  const total = customerState.cart.reduce((sum, line) => {
    const item = customerState.menuItems.find((candidate) => candidate.id === line.menuItemId);
    return sum + (item ? item.price * line.quantity : 0);
  }, 0);
  document.getElementById("cart-total").textContent = Api.formatMoney(total);
}

function changeQty(itemId, delta) {
  const line = customerState.cart.find((candidate) => candidate.menuItemId === itemId);
  if (!line) return;
  line.quantity += delta;
  if (line.quantity <= 0) {
    customerState.cart = customerState.cart.filter((candidate) => candidate.menuItemId !== itemId);
  }
  renderCart();
}

async function submitOrder() {
  if (!customerState.session) return Notifications.showToast("Ask cashier to open this table first.");
  if (!customerState.cart.length) return Notifications.showToast("Cart is empty.");
  try {
    await Api.post("/api/orders", {
      tableCode: customerState.tableCode,
      sessionId: customerState.session.id,
      items: customerState.cart,
      idempotencyKey: `${customerState.tableCode}-${Date.now()}`
    });
    customerState.cart = [];
    Notifications.showToast("Order submitted. Waiting for cashier approval.");
    await loadCustomerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

function renderOrders() {
  const orders = document.getElementById("orders");
  if (!customerState.orders.length) {
    orders.innerHTML = Api.empty("No order yet.");
    return;
  }
  orders.innerHTML = customerState.orders.map((order) => `
    <div class="row">
      <div class="row-title"><span>Order #${order.id}</span><span class="badge status-${order.status}">${order.status}</span></div>
      ${(order.items || []).map((item) => `
        <p class="muted">Item #${item.id} · ${Api.escapeHtml(item.name)} x${item.quantity} · ${item.status} · ${Api.escapeHtml(item.note || "")} · ${Api.formatMoney(item.lineTotal)}</p>
      `).join("")}
      ${order.status === "NEEDS_CUSTOMER_CONFIRMATION" ? `
        <div class="notice">
          <strong>Some item is sold out.</strong>
          <p class="muted">Choose how to update this order before cashier sends it to kitchen.</p>
          <div class="row-actions">
            <button class="button danger" onclick="resolveSoldOut(${order.id}, 'CANCEL_ORDER')">Cancel order</button>
            <button class="button secondary" onclick="resolveSoldOut(${order.id}, 'REMOVE_UNAVAILABLE')">Remove sold-out</button>
            <button class="button primary" onclick="replaceSoldOut(${order.id})">Replace item</button>
          </div>
        </div>
      ` : ""}
    </div>
  `).join("");
}

async function resolveSoldOut(orderId, decision, replacements = []) {
  try {
    await Api.post(`/api/orders/${orderId}/customer-decision`, { decision, replacements, actor: "customer" });
    Notifications.showToast("Order decision saved. Cashier will review again.");
    await loadCustomerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

async function replaceSoldOut(orderId) {
  const menuItemId = Number(prompt("Replacement menu item id:"));
  if (!menuItemId) return;
  const quantity = Number(prompt("Quantity:", "1")) || 1;
  await resolveSoldOut(orderId, "REPLACE_ITEMS", [{ menuItemId, quantity }]);
}

function renderRecommendations() {
  const recommendations = document.getElementById("recommendations");
  if (!customerState.recommendations.length) {
    recommendations.innerHTML = Api.empty("Recommendations appear after table is active.");
    return;
  }
  recommendations.innerHTML = customerState.recommendations.map((item) => `
    <div class="row">
      <div class="row-title"><span>${Api.escapeHtml(item.name)}</span><span>${item.matchPercent}% match</span></div>
      <p class="muted">${Api.formatMoney(item.price)} · ${Api.escapeHtml(item.reason)}</p>
      <button class="button secondary full" onclick="addToCart(${item.menuItemId})">Add suggestion</button>
    </div>
  `).join("");
}

async function requestCancel() {
  if (!customerState.session) return;
  const orderItemId = Number(prompt("Order item id to cancel:"));
  if (!orderItemId) return;
  try {
    await Api.post(`/api/order-items/${orderItemId}/cancel-request`, { sessionId: customerState.session.id });
    Notifications.showToast("Cancel request sent to cashier.");
    await loadCustomerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

async function requestBill() {
  if (!customerState.session) return Notifications.showToast("No active session.");
  try {
    const bill = await Api.post(`/api/sessions/${customerState.session.id}/bill`, { actor: "customer" });
    Notifications.showToast(`Bill requested: ${Api.formatMoney(bill.total)}`);
    await loadCustomerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}
