const kitchenState = {
  station: new URLSearchParams(location.search).get("station") || "kitchen",
  tasks: []
};

document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("station-title").textContent = Api.stationText(kitchenState.station);
  loadKitchenData();
  Notifications.startNotificationPolling(kitchenState.station, () => loadKitchenData());
});

async function loadKitchenData() {
  try {
    kitchenState.tasks = (await Api.get(`/api/kitchen/tasks?station=${encodeURIComponent(kitchenState.station)}`)).tasks;
    renderKitchen();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}

function renderKitchen() {
  renderTaskColumn("tasks-pending", "PENDING");
  renderTaskColumn("tasks-preparing", "PREPARING");
  renderTaskColumn("tasks-ready", "READY");
  renderTaskColumn("tasks-issue", "ISSUE");
}

function renderTaskColumn(elementId, status) {
  const root = document.getElementById(elementId);
  const tasks = kitchenState.tasks.filter((task) => task.status === status);
  if (!tasks.length) {
    root.innerHTML = Api.empty("Không có món.");
    return;
  }
  root.innerHTML = tasks.map((task) => `
    <div class="row">
      <div class="row-title"><span>Món bếp #${task.id}</span><span class="badge status-${task.status}">${Api.statusText(task.status)}</span></div>
      <p class="muted">${Api.escapeHtml(Api.itemNameText(task.itemName))} x${task.quantity}</p>
      <p class="muted">Bàn ${Api.tableText(task.tableCodes)}</p>
      ${task.issue ? `<p class="muted">Sự cố #${task.issueId}: ${Api.escapeHtml(task.issue)}</p>` : ""}
      <div class="row-actions">
        ${task.status === "PENDING" ? `<button class="button primary" onclick="startTask(${task.id})">Bắt đầu làm</button>` : ""}
        ${task.status === "PREPARING" ? `<button class="button secondary" onclick="readyTask(${task.id})">Báo xong</button>` : ""}
        ${task.status === "READY" ? `<button class="button primary" onclick="servedTask(${task.id})">Đã phục vụ</button>` : ""}
      </div>
      ${["PENDING", "PREPARING", "READY"].includes(task.status) ? `
        <form class="issue-form" onsubmit="reportIssue(event, ${task.id})">
          <label class="field">
            <span>Lý do sự cố</span>
            <input name="reason" type="text" placeholder="Hết nguyên liệu, sai món, lỗi chất lượng..." required />
          </label>
          <button class="button danger" type="submit">Báo sự cố</button>
        </form>
      ` : ""}
    </div>
  `).join("");
}

async function startTask(taskId) {
  await runKitchenAction(() => Api.post(`/api/kitchen/tasks/${taskId}/start`, { station: kitchenState.station }), "Đã bắt đầu làm món.");
}

async function readyTask(taskId) {
  await runKitchenAction(() => Api.post(`/api/kitchen/tasks/${taskId}/ready`, { station: kitchenState.station }), "Đã báo món xong.");
}

async function servedTask(taskId) {
  await runKitchenAction(() => Api.post(`/api/kitchen/tasks/${taskId}/served`, { actor: "waiter" }), "Đã ghi nhận món được phục vụ.");
}

async function reportIssue(event, taskId) {
  event.preventDefault();
  const reason = String(new FormData(event.currentTarget).get("reason") || "").trim();
  if (!reason) return;
  await runKitchenAction(() => Api.post(`/api/kitchen/tasks/${taskId}/issue`, { station: kitchenState.station, reason }), "Đã báo sự cố.");
  event.currentTarget.reset();
}

async function runKitchenAction(action, successMessage) {
  try {
    await action();
    Notifications.showToast(successMessage);
    await loadKitchenData();
  } catch (error) {
    Notifications.showToast(Api.errorText(error));
  }
}
