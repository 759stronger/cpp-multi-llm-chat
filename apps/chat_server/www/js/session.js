const SessionManager = {
  sessions: [],
  currentSessionId: null,
  currentModel: null,

  async loadSessions() {
    try {
      const data = await API.getSessions();
      if (data.success) {
        this.sessions = data.data || [];
        this.render();
      }
    } catch (e) {
      console.error('加载会话列表失败:', e);
    }
  },

  getCurrentSession() {
    if (!this.currentSessionId) return null;
    return this.sessions.find(s => s.id === this.currentSessionId) || null;
  },

  selectSession(sessionId) {
    this.currentSessionId = sessionId;
    const s = this.getCurrentSession();
    this.currentModel = s ? s.model : null;
    this.render();
    UIChat.showChatPage(s);
    if (s) Chat.loadHistory(sessionId);
  },

  async createSession(model) {
    try {
      const data = await API.createSession(model);
      if (data.success) {
        const info = typeof data.data === 'string' ? JSON.parse(data.data) : data.data;
        const newSession = {
          id: info.session_id,
          model: info.model,
          created_at: Date.now() / 1000,
          updated_at: Date.now() / 1000,
          message_count: 0
        };
        this.sessions.unshift(newSession);
        this.selectSession(newSession.id);
        this.render();
        Toast.success('创建成功');
        return newSession;
      }
      Toast.error('创建失败');
      return null;
    } catch (e) {
      Toast.error('网络异常');
      return null;
    }
  },

  async deleteSession(sessionId) {
    try {
      await API.deleteSession(sessionId);
      this.sessions = this.sessions.filter(s => s.id !== sessionId);
      if (this.currentSessionId === sessionId) {
        this.currentSessionId = null;
        this.currentModel = null;
        UIChat.showWelcome();
      }
      this.render();
      Toast.success('删除成功');
    } catch (e) {
      Toast.error('删除失败');
    }
  },

  render() {
    const container = document.getElementById('session-list');
    if (!container) return;
    if (this.sessions.length === 0) {
      container.innerHTML = '<div style="text-align:center;color:var(--text-muted);padding:32px 16px;font-size:13px;">暂无对话</div>';
      return;
    }
    container.innerHTML = this.sessions.map(s => {
      const preview = s.first_user_message_content || '新对话';
      const time = s.updated_at ? new Date(s.updated_at * 1000).toLocaleString('zh-CN', { month:'2-digit', day:'2-digit', hour:'2-digit', minute:'2-digit' }) : '';
      const model = s.model || '未知模型';
      const active = s.id === this.currentSessionId ? ' active' : '';
      return `<div class="session-item${active}" data-id="${s.id}">
        <div class="session-item-info">
          <div class="session-item-preview">${Utils.escapeHtml(preview)}</div>
          <div class="session-item-meta">${Utils.escapeHtml(model)} · ${time}</div>
        </div>
        <button class="session-item-menu-btn" data-action="menu" data-id="${s.id}" title="更多">⋯</button>
        <div class="session-item-dropdown" data-dropdown="${s.id}">
          <button data-action="delete" data-id="${s.id}">删除会话</button>
        </div>
      </div>`;
    }).join('');
  }
};

window.SessionManager = SessionManager;
