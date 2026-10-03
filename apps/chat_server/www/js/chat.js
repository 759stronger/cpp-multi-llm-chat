const Chat = {
  messages: [],
  streamAbortController: null,
  activeSessionId: null,
  _pendingTyping: null,

  setupMarked() {
    if (typeof marked === 'undefined') return;
    if (typeof hljs !== 'undefined') {
      marked.setOptions({
        highlight: (code, lang) => {
          if (lang && hljs.getLanguage(lang)) {
            return hljs.highlight(code, { language: lang }).value;
          }
          return code;
        }
      });
    }
  },

  async loadHistory(sessionId) {
    this._cleanupStream();
    this.activeSessionId = sessionId;
    this.messages = [];
    const container = document.getElementById('chat-messages');
    if (container) container.innerHTML = '';
    const loadingEl = document.getElementById('chat-history-loading');
    if (loadingEl) loadingEl.classList.remove('hidden');
    try {
      const data = await API.getSessionHistory(sessionId);
      if (loadingEl) loadingEl.classList.add('hidden');
      if (data.success) {
        this.messages = data.data || [];
        this.renderMessages();
        this.scrollToBottom();
      }
    } catch (e) {
      if (loadingEl) loadingEl.classList.add('hidden');
      Toast.error('加载历史失败');
    }
  },

  renderMessages() {
    const container = document.getElementById('chat-messages');
    if (!container) return;
    container.innerHTML = '';
    this.messages.forEach(msg => {
      container.appendChild(this.createMessageElement(msg));
    });
    this.highlightAllCode();
    this.attachCopyButtons();
  },

  createMessageElement(msg) {
    const div = document.createElement('div');
    div.className = `message ${msg.role}`;

    const bubble = document.createElement('div');
    bubble.className = 'message-bubble';

    const content = document.createElement('div');
    content.className = 'message-content';
    if (msg.role === 'assistant') {
      content.innerHTML = this.renderMarkdown(msg.content || '');
    } else {
      content.textContent = msg.content || '';
    }
    bubble.appendChild(content);

    const time = document.createElement('div');
    time.className = 'message-time';
    time.textContent = msg.timestamp
      ? new Date(msg.timestamp * 1000).toLocaleString('zh-CN', { month:'2-digit', day:'2-digit', hour:'2-digit', minute:'2-digit' })
      : '';
    bubble.appendChild(time);

    div.appendChild(bubble);
    return div;
  },

  renderMarkdown(text) {
    if (!text) return '';
    if (typeof marked === 'undefined') return Utils.escapeHtml(text).replace(/\n/g, '<br>');
    return marked.parse(text);
  },

  addUserMessage(text) {
    const msg = { role: 'user', content: text, timestamp: Math.floor(Date.now() / 1000) };
    this.messages.push(msg);
    const el = this.createMessageElement(msg);
    const container = document.getElementById('chat-messages');
    if (container) container.appendChild(el);
    this.scrollToBottom();
  },

  addAssistantPlaceholder() {
    const container = document.getElementById('chat-messages');
    if (!container) return null;
    const div = document.createElement('div');
    div.className = 'message assistant typing';
    const bubble = document.createElement('div');
    bubble.className = 'message-bubble';
    const indicator = document.createElement('div');
    indicator.innerHTML = '<div class="typing-indicator"><span></span><span></span><span></span></div>';
    bubble.appendChild(indicator);
    div.appendChild(bubble);
    container.appendChild(div);
    this.scrollToBottom();
    this._pendingTyping = { div, bubble, indicator };
    return this._pendingTyping;
  },

  _cleanupStream() {
    if (this._pendingTyping) {
      const el = this._pendingTyping.div;
      if (el && el.parentNode) el.remove();
      this._pendingTyping = null;
    }
  },

  replaceTypingWithContent(typingEls, text) {
    const { div, bubble, indicator } = typingEls;
    if (indicator && indicator.parentNode) indicator.remove();
    let contentEl = bubble.querySelector('.message-content');
    if (!contentEl) {
      contentEl = document.createElement('div');
      contentEl.className = 'message-content';
      bubble.appendChild(contentEl);
      const timeEl = document.createElement('div');
      timeEl.className = 'message-time';
      timeEl.textContent = new Date().toLocaleString('zh-CN', { month:'2-digit', day:'2-digit', hour:'2-digit', minute:'2-digit' });
      bubble.appendChild(timeEl);
      div.classList.remove('typing');
    }
    contentEl.innerHTML = this.renderMarkdown(text);
    this.highlightAllCode();
    this.attachCopyButtons();
    this.scrollToBottom();
  },

  async sendMessage(text) {
    if (!text.trim()) return;
    const sessionId = SessionManager.currentSessionId;
    if (!sessionId) return;

    this._cleanupStream();
    this.activeSessionId = sessionId;
    this.addUserMessage(text);

    const typingEls = this.addAssistantPlaceholder();
    let fullText = '';

    try {
      await API.sendStreamMessage(sessionId, text,
        (chunk) => {
          if (this.activeSessionId !== sessionId) return;
          fullText += chunk;
          if (fullText.length > 0 && typingEls) this.replaceTypingWithContent(typingEls, fullText);
        },
        () => {
          if (this.activeSessionId !== sessionId) return;
          this._pendingTyping = null;
          this.messages.push({
            role: 'assistant',
            content: fullText,
            timestamp: Math.floor(Date.now() / 1000)
          });
          SessionManager.loadSessions();
        },
        (err) => {
          console.error('Stream error:', err);
          if (this.activeSessionId === sessionId) Toast.error('发送失败');
          this._cleanupStream();
        }
      );
    } catch (e) {
      console.error('Error:', e);
      if (this.activeSessionId === sessionId) Toast.error('网络异常');
      this._cleanupStream();
    }
  },

  highlightAllCode() {
    if (typeof hljs === 'undefined') return;
    document.querySelectorAll('.message-content pre code').forEach(block => {
      hljs.highlightElement(block);
    });
  },

  attachCopyButtons() {
    document.querySelectorAll('.message-content pre').forEach(pre => {
      if (pre.parentElement.classList.contains('code-block-wrapper')) return;
      const wrapper = document.createElement('div');
      wrapper.className = 'code-block-wrapper';
      const header = document.createElement('div');
      header.className = 'code-block-header';
      const lang = pre.querySelector('code')?.className.match(/language-(\w+)/)?.[1] || '';
      const langSpan = document.createElement('span');
      langSpan.className = 'code-block-lang';
      langSpan.textContent = lang;
      const copyBtn = document.createElement('button');
      copyBtn.className = 'code-block-copy';
      copyBtn.textContent = '复制';
      copyBtn.addEventListener('click', () => {
        const code = pre.textContent || '';
        navigator.clipboard.writeText(code).then(() => {
          copyBtn.textContent = '已复制';
          copyBtn.classList.add('copied');
          setTimeout(() => { copyBtn.textContent = '复制'; copyBtn.classList.remove('copied'); }, 2000);
        });
      });
      header.appendChild(langSpan);
      header.appendChild(copyBtn);
      pre.parentNode.insertBefore(wrapper, pre);
      wrapper.appendChild(header);
      wrapper.appendChild(pre);
    });
  },

  scrollToBottom() {
    const container = document.getElementById('chat-messages');
    if (container) {
      requestAnimationFrame(() => {
        container.scrollTop = container.scrollHeight;
      });
    }
  }
};

window.Chat = Chat;
