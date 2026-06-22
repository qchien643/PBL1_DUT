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
    let isHandling = false;

    function notificationUrl(afterId) {
      return `/api/notifications?channel=${encodeURIComponent(channel)}&after=${afterId}`;
    }

    async function poll() {
      try {
        if (isHandling) return;
        isHandling = true;

        let data = await Api.get(notificationUrl(lastId));
        let events = data.events || [];
        if (!events.length && Number(data.latestId || 0) < lastId) {
          lastId = 0;
          localStorage.setItem(storageKey, "0");
          data = await Api.get(notificationUrl(lastId));
          events = data.events || [];
        }
        for (const event of events) {
          lastId = Math.max(lastId, event.id);
          showToast(Api.notificationText(event));
        }
        localStorage.setItem(storageKey, String(lastId));
        if (events.length && handler) {
          await handler(events[events.length - 1], events);
        }
      } catch (error) {
        // Polling is non-blocking; the next interval retries automatically.
      } finally {
        isHandling = false;
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
