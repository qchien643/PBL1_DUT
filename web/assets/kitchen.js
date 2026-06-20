const kitchenState = {
  station: new URLSearchParams(location.search).get("station") || "kitchen",
  tasks: []
};

document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("station-title").textContent = `${kitchenState.station.toUpperCase()} Board`;
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
    root.innerHTML = Api.empty("No task.");
    return;
  }
  root.innerHTML = tasks.map((task) => `
    <div class="row">
      <div class="row-title"><span>Task #${task.id}</span><span class="badge status-${task.status}">${task.status}</span></div>
      <p class="muted">${Api.escapeHtml(task.itemName)} x${task.quantity}</p>
      <p class="muted">Tables ${Api.tableText(task.tableCodes)}</p>
      ${task.issue ? `<p class="muted">Issue #${task.issueId}: ${Api.escapeHtml(task.issue)}</p>` : ""}
      <div class="row-actions">
        ${task.status === "PENDING" ? `<button class="button primary" onclick="startTask(${task.id})">Start</button>` : ""}
        ${task.status === "PREPARING" ? `<button class="button secondary" onclick="readyTask(${task.id})">Ready</button>` : ""}
        ${task.status === "READY" ? `<button class="button primary" onclick="servedTask(${task.id})">Served</button>` : ""}
        ${["PENDING", "PREPARING", "READY"].includes(task.status) ? `<button class="button danger" onclick="reportIssue(${task.id})">Report issue</button>` : ""}
      </div>
    </div>
  `).join("");
}

async function startTask(taskId) {
  await runKitchenAction(() => Api.post(`/api/kitchen/tasks/${taskId}/start`, { station: kitchenState.station }), "Task started.");
}

async function readyTask(taskId) {
  await runKitchenAction(() => Api.post(`/api/kitchen/tasks/${taskId}/ready`, { station: kitchenState.station }), "Task is ready.");
}

async function servedTask(taskId) {
  await runKitchenAction(() => Api.post(`/api/kitchen/tasks/${taskId}/served`, { actor: "waiter" }), "Task marked served.");
}

async function reportIssue(taskId) {
  const reason = prompt("Issue reason:", "Item unavailable or quality issue");
  if (!reason) return;
  await runKitchenAction(() => Api.post(`/api/kitchen/tasks/${taskId}/issue`, { station: kitchenState.station, reason }), "Issue reported.");
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
