# C++ Multi-LLM Chat

![C++ Multi-LLM Chat 项目概览（主题插画，非运行截图）](docs/images/hero.png)

一个由 C++17 SDK、HTTP 服务和原生 Web 前端组成的多模型聊天学习项目，统一封装云端与本地 Provider，并使用 SQLite 保存会话和消息历史。

代码包含完整的主要聊天调用链路。下文以当前实现为准；干净环境构建、全部模型的真实响应和部署可靠性尚未复核。

## 已实现的能力

- **统一 Provider 接口**：DeepSeek、ChatGPT、Gemini 和 Ollama 四类适配器。
- **非流式与流式调用**：上游增量响应经 Provider 回调传递，再由 HTTP 服务封装为 SSE。
- **会话管理**：创建、列出、读取历史、删除会话。
- **本地持久化**：SQLite 会话/消息表、索引和参数绑定查询。
- **Web 对话界面**：模型选择、会话切换、实时消息显示、Markdown/代码展示。
- **运行配置**：监听地址、端口、日志级别、温度、输出长度及 Ollama 参数。

Provider 已有实现不等于当前账号与模型一定可调用。云端模型名和部分端点写在源码中，需按实际服务权限与协议核对。

## 架构

![SDK、服务、存储与 Provider 架构](docs/images/architecture.svg)

```text
Web 前端
  → chatServer（cpp-httplib HTTP / SSE）
  → chat_sdk（会话与消息主接口）
  ├── SessionManager → DataManager → SQLite
  └── LLMManager → LLMProvider
                   ├── DeepSeekProvider
                   ├── ChatGPTProvider
                   ├── GeminiProvider
                   └── OllamaDeepSeekProvider
```

流式调用向上游发送 `stream=true`，用 HTTP 内容接收回调解析增量并立即向客户端转发。前端通过 Fetch ReadableStream 读取 SSE；这条链路不是收到完整回复后再逐字模拟。

## 目录

~~~text
CMakeLists.txt              根构建入口，直接连接 SDK 与应用目标
sdk/
├── include/ai_chat_sdk/    SDK、Provider、会话与存储公共头文件
├── src/                    对应实现
└── CMakeLists.txt          ai_chat_sdk 静态库与可选安装规则
apps/chat_server/
├── main.cpp                参数解析、环境变量与启动入口
├── chat_server.cpp/.h      HTTP API、SSE 与服务生命周期
├── CMakeLists.txt
└── www/                    HTML、CSS 与原生 JavaScript 前端
tests/
├── test_llm.cpp            需要真实模型与终端输入的手动集成测试
└── CMakeLists.txt
docs/images/                项目概览与架构图
local/backups/              已有私有备份，仅本地保留并排除 Git 上传
~~~

数据库、日志及旧 build 目录均不公开上传。旧构建目录保留在本地，但目录调整后应重新生成构建配置，不复用旧 CMake 缓存。

## 环境与依赖

当前构建方式面向 Linux，文档中的命令以 Ubuntu/Debian 和 `/usr/local` 安装前缀为例。

| 依赖 | 用途 |
| --- | --- |
| C++17 编译器、CMake 3.15+ | 构建 SDK、服务器与测试；根工程与本文命令均要求 CMake 3.15 或更新版本 |
| cpp-httplib 的 httplib.h | HTTP 客户端、服务器与内容接收回调 |
| OpenSSL | 上游 HTTPS |
| jsoncpp | JSON 请求与响应 |
| SQLite3 | 会话与消息持久化 |
| fmt、spdlog | 日志和格式化 |
| gflags、Threads/pthread、pkg-config | 服务参数、线程与依赖发现 |
| GoogleTest（可选） | 构建 tests/ 下的手动集成测试 |
| Ollama（使用本地 Provider 时） | 本地模型服务 |

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config libjsoncpp-dev libfmt-dev \
  libspdlog-dev libsqlite3-dev libgflags-dev libssl-dev curl
```

**还需单独准备 cpp-httplib。** 仓库未提交 `httplib.h`，也未锁定兼容版本。请从 [cpp-httplib 官方仓库](https://github.com/yhirose/cpp-httplib) 准备头文件，放到编译器可搜索目录，例如 `/usr/local/include/httplib.h`。所选版本必须兼容源码中 `Request::content_receiver` 的四参数回调、`set_chunked_content_provider` 和 `Client::send`；本项目尚未记录已验证的依赖版本组合。若使用把 httplib 拆为头文件和共享库的系统包，需要额外核对其链接要求。

## 获取与构建

~~~bash
git clone https://github.com/759stronger/cpp-multi-llm-chat.git
cd cpp-multi-llm-chat
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
~~~

根 CMake 默认构建 `ai_chat_sdk` 和 `AIChatServer`，服务器直接链接源码树中的 SDK 目标，无需先用管理员权限安装 SDK，也不依赖固定的 `/usr/local/lib` 搜索路径。SDK 的头文件与静态库安装规则仍保留，可按需要设置安装前缀后单独安装。

SDK、服务器和可选测试共用 C++17、OpenSSL、cpp-httplib 和目标式依赖配置。OpenSSL 通过 `OpenSSL::SSL`、`OpenSSL::Crypto` 连接；其他开发库通过 pkg-config 查找。cpp-httplib 没有自动下载逻辑，头文件不在默认路径时，可配置：

~~~bash
cmake -S . -B build -DCPPHTTPLIB_INCLUDE_DIR=/path/to/headers
~~~

该目录应直接包含 `httplib.h`。依赖版本组合与真实模型协议仍需按实际环境验证，文档不把静态配置核对当作构建通过记录。

### 可选手动集成测试

真实模型集成测试默认不构建，也未注册为自动 CTest 用例。如需手动准备该可执行文件，可显式启用：

~~~bash
cmake -S . -B build -DBUILD_INTERACTIVE_TESTS=ON
cmake --build build --target LLMTest
~~~

这里只构建，不自动执行。现有测试需要实际模型配置与终端输入，运行可能调用付费上游；不能将它视为离线单元测试。

## 配置与运行

实际密钥入口是以下**小写环境变量**：

- `deepseek_api_key`
- `chatgpt_api_key`
- `gemini_api_key`

**当前 main 要求三项云端密钥全部非空。** 缺少任意一项会退出，即使你只打算使用 Ollama。上游密钥配置与访问本服务的用户鉴权是不同功能；当前没有下游用户鉴权。

在准备好凭据的终端中设置环境变量。下面仅为占位示例：

```bash
export deepseek_api_key="<your-deepseek-key>"
export chatgpt_api_key="<your-chatgpt-key>"
export gemini_api_key="<your-gemini-key>"
```

请勿把真实密钥提交到仓库。随后从 `apps/chat_server` 目录启动，以保证 `./www` 静态资源路径正确：

```bash
cd apps/chat_server
../../build/bin/AIChatServer --host=127.0.0.1 --port=8080
```

打开 [http://127.0.0.1:8080](http://127.0.0.1:8080)。运行目录下会创建或读取 `chatDB.db`。 为继续使用已有本地会话数据，请保持该启动目录；从其他目录运行会使用其他位置的数据库。构建成功后还会将 `www` 复制到可执行文件旁。单配置构建的程序位于 `build/bin/`；Windows 使用 `.exe`，多配置生成器可能再增加配置名子目录。

| 参数 | 默认值 | 说明 |
| --- | --- | --- |
| --host | 0.0.0.0 | 建议本地演示显式设置为 127.0.0.1 |
| --port | 8080 | HTTP 监听端口 |
| --log_level | DEBUG | DEBUG / INFO / WARN / ERROR |
| --temperature | 0.7 | 入口检查范围为 0–1 |
| --max_tokens | 2048 | 入口要求大于 0 |
| --ollama_endpoint | http://127.0.0.1:11434 | Ollama 地址 |
| --ollama_model_name | deepseek-r1:1.5b | 本地模型名 |
| --config_file | chatServer.conf | 非密钥参数的配置文件路径 |

`chatServer.conf` 可写入 gflags 风格参数，例如：

```text
--host=127.0.0.1
--port=8080
--ollama_endpoint=http://127.0.0.1:11434
--ollama_model_name=deepseek-r1:1.5b
```

该文件不会替代 main 中从环境变量读取密钥的逻辑。服务中的 ChatGPT/Gemini Provider 固定使用本机 HTTP 代理 `127.0.0.1:7890`；ChatGPT 默认端点是第三方服务。运行前应核对代理、上游归属、授权与协议，必要时调整源码配置后重新构建。

## HTTP API

| 方法与路径 | 功能 |
| --- | --- |
| GET /api/models | 返回代码标记为可用的模型列表 |
| POST /api/sessions | 创建会话，请求体含 model |
| GET /api/sessions | 获取所有会话 |
| GET /api/sessions/{id}/history | 获取会话历史 |
| DELETE /api/sessions/{id} | 删除会话 |
| POST /api/message | 非流式消息，请求体含 session_id、message |
| POST /api/message/async | SSE 流式消息，结束标记为 data: [DONE] |

这些是项目自有 API；当前并非一个对外提供 `/v1/chat/completions` 的通用 OpenAI 兼容网关。

## 最小验证

服务启动后，先验证静态页面和模型列表；这些请求不会发送聊天消息给上游：

```bash
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/api/models
```

预期：页面请求返回 HTML；模型接口返回 JSON。模型列表中的“可用”来自初始化标记，不能作为真实连通性或账号权限验证。

如需验证持久化链路，可用模型列表中的一个名称创建会话，再查询和删除该会话：

```bash
curl -i -X POST http://127.0.0.1:8080/api/sessions \
  -H 'Content-Type: application/json' \
  -d '{"model":"deepseek-r1:1.5b"}'
curl -i http://127.0.0.1:8080/api/sessions
```

创建接口返回的 `data.session_id` 可用于历史查询和删除：

```bash
curl -i "http://127.0.0.1:8080/api/sessions/<session-id>/history"
curl -i -X DELETE "http://127.0.0.1:8080/api/sessions/<session-id>"
```

将 `<session-id>` 替换为返回的实际 ID。聊天和 SSE 验证会调用真实上游并可能产生费用；准备好模型、代理与账号权限后，再通过页面或消息 API 验证。本 README 未记录这些步骤已经通过。

## 测试与当前边界

- `tests/` 使用 GoogleTest，但大部分 Provider 用例处于注释状态；启用的聊天测试需要真实上游和终端输入，不是独立离线回归测试。
- 四类适配器有实际调用实现，但上游模型名称、请求/响应兼容性与完整流式错误处理尚未逐一验证。
- 有连接和读取超时，尚未实现自动重试、跨 Provider fallback、调用限额或用量计费。
- API 没有用户身份、会话归属和下游鉴权。默认监听所有地址；公网或共享部署前需要补齐访问控制及部署保护。
- `--enable_ssl` 虽存在于参数定义中，服务器当前仍创建普通 `httplib::Server`，不能据此宣称启用了 HTTPS。
- 初始化主要设置模型可用标记；服务器启动状态也没有完整反映后台监听失败。日志显示成功不能替代 HTTP 探测。
- 没有已提交的 CI、固定依赖版本组合或完整可重复的构建与运行验证记录。

## 后续完善

1. 固定 cpp-httplib 与其他依赖版本，验证当前构建配置并补离线 Mock 测试。
2. 校验各 Provider 的非流式/流式协议、失败结束信号和模型配置。
3. 加入下游鉴权、会话隔离、重试策略和服务生命周期验证。
4. 再逐步扩展为具有路由、账号池、限流和用量记录的 AI 网关。

## 许可证

原 README 声明 MIT；当前未附独立 LICENSE 文件。建议补齐授权文本后再分发或复用。
