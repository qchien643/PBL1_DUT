const customerState = {
  tableCode: new URLSearchParams(location.search).get("table") || "T01",
  session: null,
  menuItems: [],
  cart: [],
  orders: [],
  recommendations: []
};

document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("page-title").textContent = `Bàn ${customerState.tableCode}`;
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
    ? `Phiên #${customerState.session.id} · ${Api.statusText(customerState.session.status)}`
    : "Bàn này chưa được thu ngân mở.";
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
        <span>${Api.escapeHtml(Api.itemNameText(item.name))}</span>
        <span class="badge status-${item.availabilityStatus}">${Api.statusText(item.availabilityStatus)}</span>
      </div>
      <p class="muted">${Api.categoryText(item.category)} · ${Api.formatMoney(item.price)} · ${Api.stationText(item.station)}</p>
      <button class="button primary full" ${!customerState.session || item.availabilityStatus !== "AVAILABLE" ? "disabled" : ""}
        onclick="addToCart(${item.id})">Thêm món</button>
    </article>
  `).join("");
}

function addToCart(itemId) {
  const item = customerState.menuItems.find((candidate) => candidate.id === itemId);
  if (!item) return;
  const existing = customerState.cart.find((line) => line.menuItemId === itemId);
  if (existing) existing.quantity += 1;
  else customerState.cart.push({ menuItemId: itemId, quantity: 1 });
  Notifications.showToast(`Đã thêm ${Api.itemNameText(item.name)}.`);
  renderCart();
}

function renderCart() {
  const lines = document.getElementById("cart-lines");
  if (!customerState.cart.length) {
    lines.innerHTML = Api.empty("Chưa chọn món nào.");
  } else {
    lines.innerHTML = customerState.cart.map((line) => {
      const item = customerState.menuItems.find((candidate) => candidate.id === line.menuItemId);
      return `<div class="row">
        <div class="row-title"><span>${Api.escapeHtml(Api.itemNameText(item?.name || "Món không xác định"))}</span><span>x${line.quantity}</span></div>
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
  if (!customerState.session) return Notifications.showToast("Vui lòng nhờ thu ngân mở bàn trước.");
  if (!customerState.cart.length) return Notifications.showToast("Chưa chọn món nào.");
  try {
    await Api.post("/api/orders", {
      tableCode: customerState.tableCode,
      sessionId: customerState.session.id,
      items: customerState.cart,
      idempotencyKey: `${customerState.tableCode}-${Date.now()}`
    });
    customerState.cart = [];
    Notifications.showToast("Đã gửi đơn món. Vui lòng chờ thu ngân duyệt.");
    await loadCustomerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

function renderOrders() {
  const orders = document.getElementById("orders");
  if (!customerState.orders.length) {
    orders.innerHTML = Api.empty("Chưa có đơn món.");
    return;
  }
  orders.innerHTML = customerState.orders.map((order) => `
    <div class="row">
      <div class="row-title"><span>Đơn #${order.id}</span><span class="badge status-${order.status}">${Api.statusText(order.status)}</span></div>
      ${(order.items || []).map((item) => `
        <div class="order-item-line">
          <p class="muted">Món #${item.id} · ${Api.escapeHtml(Api.itemNameText(item.name))} x${item.quantity} · ${Api.statusText(item.status)}${item.note ? ` · ${Api.escapeHtml(Api.noteText(item.note))}` : ""} · ${Api.formatMoney(item.lineTotal)}</p>
          ${item.canRequestCancel ? `<button class="button danger small" onclick="requestCancel(${item.id})">Yêu cầu hủy</button>` : ""}
        </div>
        ${item.status === "ISSUE_PENDING_DECISION" ? `
          <div class="notice compact">
            <strong>Món đang có sự cố.</strong>
            <p class="muted">Nhân viên sẽ xử lý trước khi lập hóa đơn.</p>
          </div>
        ` : ""}
      `).join("")}
      ${order.status === "NEEDS_CUSTOMER_CONFIRMATION" ? `
        <div class="notice">
          <strong>Có món đã hết.</strong>
          <p class="muted">Vui lòng chọn cách xử lý trước khi thu ngân gửi đơn xuống bếp.</p>
          <form class="replacement-form" onsubmit="replaceSoldOut(event, ${order.id})">
            <label class="field">
              <span>Món thay thế</span>
              <select name="menuItemId" required>
                ${customerState.menuItems
                  .filter((item) => item.availabilityStatus === "AVAILABLE")
                  .map((item) => `<option value="${item.id}">${Api.escapeHtml(Api.itemNameText(item.name))} · ${Api.formatMoney(item.price)}</option>`)
                  .join("")}
              </select>
            </label>
            <label class="field compact">
              <span>Số lượng</span>
              <input name="quantity" type="number" min="1" value="1" required />
            </label>
            <button class="button primary" type="submit">Đổi món</button>
          </form>
          <div class="row-actions">
            <button class="button danger" onclick="resolveSoldOut(${order.id}, 'CANCEL_ORDER')">Hủy đơn</button>
            <button class="button secondary" onclick="resolveSoldOut(${order.id}, 'REMOVE_UNAVAILABLE')">Bỏ món đã hết</button>
          </div>
        </div>
      ` : ""}
    </div>
  `).join("");
}

async function resolveSoldOut(orderId, decision, replacements = []) {
  try {
    await Api.post(`/api/orders/${orderId}/customer-decision`, { decision, replacements, actor: "customer" });
    Notifications.showToast("Đã lưu lựa chọn. Thu ngân sẽ kiểm tra lại.");
    await loadCustomerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
    await loadCustomerData();
  }
}

async function replaceSoldOut(event, orderId) {
  event.preventDefault();
  const form = new FormData(event.currentTarget);
  const menuItemId = Number(form.get("menuItemId"));
  if (!menuItemId) return;
  const quantity = Number(form.get("quantity")) || 1;
  await resolveSoldOut(orderId, "REPLACE_ITEMS", [{ menuItemId, quantity }]);
}

function renderRecommendations() {
  const recommendations = document.getElementById("recommendations");
  if (!customerState.recommendations.length) {
    recommendations.innerHTML = Api.empty("Gợi ý sẽ xuất hiện sau khi bàn được mở.");
    return;
  }
  recommendations.innerHTML = customerState.recommendations.map((item) => `
    <div class="row">
      <div class="row-title"><span>${Api.escapeHtml(Api.itemNameText(item.name))}</span><span>Phù hợp ${item.matchPercent}%</span></div>
      <p class="muted">${Api.formatMoney(item.price)} · Phù hợp với các món của bàn</p>
      <button class="button secondary full" onclick="addToCart(${item.menuItemId})">Thêm món gợi ý</button>
    </div>
  `).join("");
}

async function requestCancel(orderItemId) {
  if (!customerState.session) return;
  if (!orderItemId) return;
  try {
    await Api.post(`/api/order-items/${orderItemId}/cancel-request`, { sessionId: customerState.session.id });
    Notifications.showToast("Đã gửi yêu cầu hủy món cho thu ngân.");
    await loadCustomerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

async function requestBill() {
  if (!customerState.session) return Notifications.showToast("Bàn chưa được mở.");
  try {
    const bill = await Api.post(`/api/sessions/${customerState.session.id}/bill`, { actor: "customer" });
    Notifications.showToast(`Đã yêu cầu thanh toán: ${Api.formatMoney(bill.total)}.`);
    await loadCustomerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}
