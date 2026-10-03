const Utils = {
  escapeHtml(str) {
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
  },

  formatTime(ts) {
    return new Date(ts * 1000).toLocaleString('zh-CN', {
      month:'2-digit', day:'2-digit', hour:'2-digit', minute:'2-digit'
    });
  }
};

const Toast = {
  show(message, type) {
    const container = document.getElementById('toast-container');
    if (!container) return;
    const el = document.createElement('div');
    el.className = `toast ${type}`;
    el.textContent = message;
    container.appendChild(el);
    setTimeout(() => { if (el.parentNode) el.remove(); }, 3000);
  },
  success(msg) { this.show(msg, 'success'); },
  error(msg) { this.show(msg, 'error'); }
};

const UIChat = {
  showChatPage(session) {
    document.getElementById('welcome-page').classList.add('hidden');
    document.getElementById('chat-page').classList.remove('hidden');
    if (session) {
      document.getElementById('chat-model-name').textContent = session.model || '';
      document.getElementById('chat-session-id').textContent = session.id || '';
    }
  },

  showWelcome() {
    document.getElementById('chat-page').classList.add('hidden');
    document.getElementById('welcome-page').classList.remove('hidden');
    document.getElementById('chat-messages').innerHTML = '';
  },

  updateCharCount() {
    const input = document.getElementById('chat-input');
    const count = document.getElementById('char-count');
    const btn = document.getElementById('btn-send');
    if (!input || !count || !btn) return;
    const len = input.value.length;
    count.textContent = `${len} / 2000`;
    btn.disabled = len === 0;
  },

  autoResizeTextarea() {
    const input = document.getElementById('chat-input');
    if (!input) return;
    input.style.height = 'auto';
    input.style.height = Math.min(input.scrollHeight, 200) + 'px';
  }
};

const Modal = {
  show(id) {
    document.getElementById(id).classList.remove('hidden');
  },
  hide(id) {
    document.getElementById(id).classList.add('hidden');
  }
};

window.Utils = Utils;
window.Toast = Toast;
window.UIChat = UIChat;
window.Modal = Modal;
