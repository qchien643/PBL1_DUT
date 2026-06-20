(function () {
  async function request(path, options = {}) {
    const response = await fetch(path, {
      headers: {
        "Content-Type": "application/json",
        ...(options.headers || {})
      },
      ...options
    });
    const payload = await response.json();
    if (!payload.ok) {
      const error = new Error(payload.error?.message || "Request failed");
      error.code = payload.error?.code || "REQUEST_FAILED";
      error.requiredAction = payload.error?.requiredAction || "";
      error.context = payload.error?.context || {};
      error.correlationId = payload.correlationId || "";
      throw error;
    }
    return payload.data;
  }

  function formatMoney(value) {
    return `${Number(value || 0).toLocaleString("en-US")} VND`;
  }

  function escapeHtml(value) {
    return String(value ?? "")
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;");
  }

  function tableText(codes) {
    return Array.isArray(codes) && codes.length ? codes.join("+") : "-";
  }

  function empty(message) {
    return `<div class="empty">${escapeHtml(message)}</div>`;
  }

  function errorText(error) {
    const action = error.requiredAction ? ` · Action: ${error.requiredAction}` : "";
    const code = error.code ? `[${error.code}] ` : "";
    return `${code}${error.message}${action}`;
  }

  window.Api = {
    get: (path) => request(path),
    post: (path, body = {}) => request(path, { method: "POST", body: JSON.stringify(body) }),
    patch: (path, body = {}) => request(path, { method: "PATCH", body: JSON.stringify(body) }),
    formatMoney,
    escapeHtml,
    tableText,
    empty,
    errorText
  };
})();
