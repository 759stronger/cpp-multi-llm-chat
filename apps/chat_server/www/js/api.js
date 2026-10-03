const API_BASE = '';

const API = {
  async getSessions() {
    const resp = await fetch(`${API_BASE}/api/sessions`);
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    return resp.json();
  },

  async getModels() {
    const resp = await fetch(`${API_BASE}/api/models`);
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    return resp.json();
  },

  async createSession(model) {
    const resp = await fetch(`${API_BASE}/api/sessions`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ model })
    });
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    return resp.json();
  },

  async getSessionHistory(sessionId) {
    const resp = await fetch(`${API_BASE}/api/sessions/${sessionId}/history`);
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    return resp.json();
  },

  async deleteSession(sessionId) {
    const resp = await fetch(`${API_BASE}/api/sessions/${sessionId}`, {
      method: 'DELETE'
    });
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    return resp.json();
  },

  async sendMessage(sessionId, message) {
    const resp = await fetch(`${API_BASE}/api/message`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ session_id: sessionId, message })
    });
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    return resp.json();
  },

  sendStreamMessage(sessionId, message, onChunk, onDone, onError) {
    return fetch(`${API_BASE}/api/message/async`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ session_id: sessionId, message })
    }).then(async (resp) => {
      if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
      const reader = resp.body.getReader();
      const decoder = new TextDecoder();
      let buffer = '';
      while (true) {
        const { done, value } = await reader.read();
        if (done) break;
        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split('\n');
        buffer = lines.pop() || '';
        for (const line of lines) {
          const trimmed = line.trim();
          if (!trimmed || !trimmed.startsWith('data:')) continue;
          const payload = trimmed.slice(5).trim();
          if (payload === '[DONE]') { onDone(); return; }
          if (payload.length > 0) onChunk(payload);
        }
      }
      onDone();
    }).catch(err => {
      if (onError) onError(err);
    });
  }
};

window.API = API;
