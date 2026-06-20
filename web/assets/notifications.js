(function () {
  function showToast(message) {
    const root = document.getElementById("toast-root");
    if (!root) return;
    const toast = document.createElement("div");
    toast.className = "toast";
    toast.textContent = message;
    root.appendChild(toast);
    setTimeout(() => toast.remove(), 4500);
  }

  function startNotificationPolling(channel, handler) {
    const storageKey = `notification:last:${channel}`;
    let lastId = Number(localStorage.getItem(storageKey) || 0);

    async function poll() {
      try {
        const data = await Api.get(`/api/notifications?channel=${encodeURIComponent(channel)}&after=${lastId}`);
        for (const event of data.events || []) {
          lastId = Math.max(lastId, event.id);
          showToast(event.message);
          if (handler) {
            handler(event);
          }
        }
        localStorage.setItem(storageKey, String(lastId));
      } catch (error) {
        console.warn(error.message);
      }
    }

    poll();
    return setInterval(poll, 1500);
  }

  window.Notifications = {
    showToast,
    startNotificationPolling
  };
})();
