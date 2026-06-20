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
        <span>${Api.escapeHtml(item.name)}</span>
        <span class="badge status-${item.availabilityStatus}">${item.availabilityStatus}</span>
      </div>
      <p class="muted">${Api.escapeHtml(item.category)} · ${Api.formatMoney(item.price)}</p>
      <button class="button ${item.availabilityStatus === "AVAILABLE" ? "danger" : "primary"} full"
        onclick="setAvailability(${item.id}, '${item.availabilityStatus === "AVAILABLE" ? "SOLD_OUT" : "AVAILABLE"}')">
        Mark ${item.availabilityStatus === "AVAILABLE" ? "sold out" : "available"}
      </button>
    </div>
  `).join("");
}

function renderSummary() {
  const summary = managerState.summary || {};
  document.getElementById("summary").innerHTML = `
    <div class="metric"><span>Paid revenue</span><strong>${Api.formatMoney(summary.paidRevenue)}</strong></div>
    <div class="metric"><span>Orders</span><strong>${summary.orderCount || 0}</strong></div>
    <div class="metric"><span>Tables</span><strong>${summary.tableCount || 0}</strong></div>
  `;
}

function renderAudit() {
  const root = document.getElementById("audit-log");
  if (!managerState.auditEvents.length) {
    root.innerHTML = Api.empty("No audit event.");
    return;
  }
  root.innerHTML = managerState.auditEvents.map((event) => `
    <div class="row">
      <div class="row-title"><span>#${event.id} ${Api.escapeHtml(event.role)}</span><span>${Api.escapeHtml(event.createdAt)}</span></div>
      <p class="muted">${Api.escapeHtml(event.message)}</p>
    </div>
  `).join("");
}

async function setAvailability(itemId, availabilityStatus) {
  try {
    await Api.patch(`/api/menu/${itemId}/availability`, { availabilityStatus, actor: "manager" });
    Notifications.showToast("Menu availability updated.");
    await loadManagerData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}
