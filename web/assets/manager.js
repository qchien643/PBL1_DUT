const managerState = {
  menuItems: [],
  summary: null,
  auditEvents: []
};

document.addEventListener("DOMContentLoaded", () => {
  loadManagerData();
  Notifications.startNotificationPolling("manager", () => loadManagerData());
});

async function loadManagerData() {
  try {
    managerState.menuItems = (await Api.get("/api/menu?includeHidden=true")).items;
    managerState.summary = await Api.get("/api/reports/summary");
    managerState.auditEvents = (await Api.get("/api/audit-events?limit=30")).events;
    renderManager();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

function renderManager() {
  renderManagerMenu();
  renderSummary();
  renderAudit();
}

function renderManagerMenu() {
  const root = document.getElementById("manager-menu");
  root.innerHTML = managerState.menuItems.map((item) => `
    <div class="row">
      <div class="row-title">
        <span>${Api.escapeHtml(Api.itemNameText(item.name))}</span>
        <span class="badge status-${item.availabilityStatus}">${Api.statusText(item.availabilityStatus)}</span>
      </div>
      <p class="muted">${Api.categoryText(item.category)} · ${Api.formatMoney(item.price)}</p>
      <button class="button ${item.availabilityStatus === "AVAILABLE" ? "danger" : "primary"} full"
        onclick="setAvailability(${item.id}, '${item.availabilityStatus === "AVAILABLE" ? "SOLD_OUT" : "AVAILABLE"}')">
        ${item.availabilityStatus === "AVAILABLE" ? "Báo hết món" : "Mở bán lại"}
      </button>
    </div>
  `).join("");
}

function renderSummary() {
  const summary = managerState.summary || {};
  document.getElementById("summary").innerHTML = `
    <div class="metric"><span>Doanh thu đã thanh toán</span><strong>${Api.formatMoney(summary.paidRevenue)}</strong></div>
    <div class="metric"><span>Đơn món</span><strong>${summary.orderCount || 0}</strong></div>
    <div class="metric"><span>Bàn</span><strong>${summary.tableCount || 0}</strong></div>
  `;
}

function renderAudit() {
  const root = document.getElementById("audit-log");
  if (!managerState.auditEvents.length) {
    root.innerHTML = Api.empty("Chưa có nhật ký thao tác.");
    return;
  }
  root.innerHTML = managerState.auditEvents.map((event) => `
    <div class="row">
      <div class="row-title"><span>#${event.id} ${Api.roleText(event.role)}</span><span>${Api.escapeHtml(event.createdAt)}</span></div>
      <p class="muted">${Api.auditText(event)}</p>
    </div>
  `).join("");
}

async function setAvailability(itemId, availabilityStatus) {
  try {
    await Api.patch(`/api/menu/${itemId}/availability`, { availabilityStatus, actor: "manager" });
    Notifications.showToast("Đã cập nhật tình trạng món.");
    await loadManagerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}
