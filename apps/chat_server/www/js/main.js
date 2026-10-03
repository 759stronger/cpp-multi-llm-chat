(function () {
  'use strict';

  let selectedModel = null;

  function init() {
    Chat.setupMarked();

    document.getElementById('chat-input').addEventListener('input', onInputChange);
    document.getElementById('chat-input').addEventListener('keydown', onInputKeydown);
    document.getElementById('btn-send').addEventListener('click', onSendClick);
    document.getElementById('btn-new-session').addEventListener('click', onNewSessionClick);
    document.getElementById('btn-welcome-new').addEventListener('click', onNewSessionClick);
    document.getElementById('btn-model-cancel').addEventListener('click', () => Modal.hide('modal-model'));
    document.getElementById('btn-model-confirm').addEventListener('click', onModelConfirm);

    document.getElementById('session-list').addEventListener('click', onSessionListClick);

    document.addEventListener('click', onDocumentClick);

    SessionManager.loadSessions();

    UIChat.updateCharCount();
  }

  function onInputChange() {
    UIChat.updateCharCount();
    UIChat.autoResizeTextarea();
  }

  function onInputKeydown(e) {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      onSendClick();
    }
  }

  async function onSendClick() {
    const input = document.getElementById('chat-input');
    const text = input.value.trim();
    if (!text) return;
    input.value = '';
    UIChat.updateCharCount();
    UIChat.autoResizeTextarea();
    await Chat.sendMessage(text);
  }

  async function onNewSessionClick() {
    try {
      const data = await API.getModels();
      if (!data.success) {
        Toast.error('获取模型列表失败');
        return;
      }
      renderModelList(data.data || []);
      selectedModel = null;
      document.getElementById('btn-model-confirm').disabled = true;
      Modal.show('modal-model');
    } catch (e) {
      Toast.error('网络异常');
    }
  }

  function renderModelList(models) {
    const container = document.getElementById('model-list');
    if (!container) return;
    container.innerHTML = models.map(m => `<div class="model-option" data-model="${Utils.escapeHtml(m.name)}">
      <div class="model-option-radio"></div>
      <div>
        <div class="model-option-name">${Utils.escapeHtml(m.name)}</div>
        <div class="model-option-desc">${Utils.escapeHtml(m.desc || '')}</div>
      </div>
    </div>`).join('');

    container.querySelectorAll('.model-option').forEach(opt => {
      opt.addEventListener('click', () => {
        container.querySelectorAll('.model-option').forEach(o => o.classList.remove('selected'));
        opt.classList.add('selected');
        selectedModel = opt.dataset.model;
        document.getElementById('btn-model-confirm').disabled = false;
      });
    });
  }

  async function onModelConfirm() {
    if (!selectedModel) return;
    Modal.hide('modal-model');
    await SessionManager.createSession(selectedModel);
  }

  function onSessionListClick(e) {
    const item = e.target.closest('.session-item');
    if (!item) return;

    const action = e.target.dataset.action;
    const id = e.target.dataset.id || item.dataset.id;
    if (!id) return;

    if (action === 'menu') {
      const dropdown = item.querySelector(`[data-dropdown="${id}"]`);
      closeAllDropdowns();
      if (dropdown) dropdown.classList.toggle('show');
      return;
    }

    if (action === 'delete') {
      closeAllDropdowns();
      SessionManager.deleteSession(id);
      return;
    }

    closeAllDropdowns();
    SessionManager.selectSession(id);
  }

  function onDocumentClick(e) {
    if (!e.target.closest('.session-item-menu-btn') && !e.target.closest('.session-item-dropdown')) {
      closeAllDropdowns();
    }
  }

  function closeAllDropdowns() {
    document.querySelectorAll('.session-item-dropdown.show').forEach(d => d.classList.remove('show'));
  }

  document.addEventListener('DOMContentLoaded', init);

})();
