#include "lan_chat_server.h"

#include "application.h"
#include "settings.h"

#include <cJSON.h>
#include <esp_log.h>

#include <cstdlib>
#include <cstring>

#define TAG "LanChatServer"

static const char LAN_CHAT_HTML[] = R"rawhtml(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
    <title>XiaoZhi Silent LAN Chat</title>
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: #1e293b;
            --user-msg: #2563eb;
            --bot-msg: #334155;
            --text-color: #f8fafc;
            --text-dim: #94a3b8;
            --accent: #38bdf8;
            --danger: #ef4444;
            --success: #22c55e;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
        body { background-color: var(--bg-color); color: var(--text-color); height: 100dvh; display: flex; flex-direction: column; }
        header { background: var(--card-bg); padding: 12px 20px; display: flex; align-items: center; justify-content: space-between; border-bottom: 1px solid #334155; }
        .title { font-size: 1.1rem; font-weight: 600; display: flex; align-items: center; gap: 8px; }
        .status-badge { font-size: 0.75rem; padding: 4px 10px; border-radius: 12px; background: #475569; color: #fff; font-weight: 500; }
        .status-badge.online { background: var(--success); }
        .status-badge.http { background: #0284c7; }
        .status-badge.error { background: var(--danger); }
        .nav-tabs { display: flex; background: var(--card-bg); border-bottom: 1px solid #334155; }
        .tab-btn { flex: 1; padding: 12px; border: none; background: transparent; color: var(--text-dim); font-weight: 600; cursor: pointer; border-bottom: 2px solid transparent; transition: all 0.2s; }
        .tab-btn.active { color: var(--accent); border-bottom-color: var(--accent); background: rgba(56, 189, 248, 0.05); }
        .content-area { flex: 1; display: flex; flex-direction: column; overflow: hidden; position: relative; }
        .tab-content { display: none; flex: 1; flex-direction: column; height: 100%; }
        .tab-content.active { display: flex; }
        #chat-messages { flex: 1; overflow-y: auto; padding: 16px; display: flex; flex-direction: column; gap: 12px; }
        .msg { max-width: 85%; padding: 10px 14px; border-radius: 14px; font-size: 0.95rem; line-height: 1.4; word-wrap: break-word; }
        .msg.user { align-self: flex-end; background: var(--user-msg); color: #fff; border-bottom-right-radius: 2px; }
        .msg.assistant { align-self: flex-start; background: var(--bot-msg); color: var(--text-color); border-bottom-left-radius: 2px; }
        .msg.system { align-self: center; background: rgba(148, 163, 184, 0.1); color: var(--text-dim); font-size: 0.8rem; border-radius: 8px; text-align: center; }
        .chat-input-bar { padding: 12px 16px; padding-bottom: calc(12px + env(safe-area-inset-bottom)); background: var(--card-bg); border-top: 1px solid #334155; display: flex; gap: 8px; }
        .chat-input-bar input { flex: 1; padding: 10px 14px; border-radius: 20px; border: 1px solid #475569; background: #0f172a; color: #fff; outline: none; font-size: 0.95rem; }
        .chat-input-bar input:focus { border-color: var(--accent); }
        .send-btn { padding: 10px 18px; border-radius: 20px; border: none; background: var(--accent); color: #0f172a; font-weight: 600; cursor: pointer; }
        .config-container { padding: 24px 20px; display: flex; flex-direction: column; gap: 20px; max-width: 500px; margin: 0 auto; width: 100%; }
        .config-card { background: var(--card-bg); border-radius: 12px; padding: 18px; border: 1px solid #334155; display: flex; flex-direction: column; gap: 14px; }
        .config-card h3 { font-size: 1rem; color: var(--accent); }
        .form-group { display: flex; flex-direction: column; gap: 6px; }
        .form-group label { font-size: 0.85rem; color: var(--text-dim); }
        .form-group input { padding: 10px; border-radius: 8px; border: 1px solid #475569; background: #0f172a; color: #fff; outline: none; }
        .save-btn { padding: 10px 16px; border-radius: 8px; border: none; background: var(--success); color: #fff; font-weight: 600; cursor: pointer; }
        .notice { font-size: 0.8rem; color: var(--text-dim); line-height: 1.4; }
    </style>
</head>
<body>
    <header>
        <div class="title">🤖 XiaoZhi LAN Chat</div>
        <span id="status-badge" class="status-badge">Đang kết nối...</span>
    </header>
    <div class="nav-tabs">
        <button class="tab-btn active" onclick="switchTab('chat')">💬 Chatbot</button>
        <button class="tab-btn" onclick="switchTab('config')">⚙️ Cấu hình</button>
    </div>
    <div class="content-area">
        <div id="tab-chat" class="tab-content active">
            <div id="chat-messages">
                <div class="msg system">Đang kết nối tới XiaoZhi LAN Chatbot...</div>
            </div>
            <div class="chat-input-bar">
                <input type="text" id="msg-input" placeholder="Nhập tin nhắn im lặng..." onkeypress="handleKey(event)">
                <button class="send-btn" onclick="sendMessage()">Gửi</button>
            </div>
        </div>
        <div id="tab-config" class="tab-content">
            <div class="config-container">
                <div class="config-card">
                    <h3>Cài đặt Lịch sử Chat</h3>
                    <div class="form-group">
                        <label for="history-limit">Số câu tin nhắn giữ lại (2 - 20):</label>
                        <input type="number" id="history-limit" min="2" max="20" value="6">
                    </div>
                    <button class="save-btn" onclick="saveConfig()">Lưu Cấu Hình</button>
                    <p class="notice">💡 Hệ thống tự động lưu trữ N câu chat gần nhất vào NVS. Màn hình chat hỗ trợ trò chuyện im lặng hoàn toàn khi tắt volume.</p>
                </div>
            </div>
        </div>
    </div>
    <script>
        let ws = null;
        let useHttpFallback = false;
        let pollTimer = null;
        let lastMsgCount = 0;

        function switchTab(tabName) {
            document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));
            if (tabName === 'chat') {
                document.querySelectorAll('.tab-btn')[0].classList.add('active');
                document.getElementById('tab-chat').classList.add('active');
            } else {
                document.querySelectorAll('.tab-btn')[1].classList.add('active');
                document.getElementById('tab-config').classList.add('active');
            }
        }

        function renderHistory(data) {
            if (!data) return;
            if (data.max_history) {
                document.getElementById('history-limit').value = data.max_history;
            }
            const container = document.getElementById('chat-messages');
            container.innerHTML = '';
            if (data.messages && data.messages.length > 0) {
                data.messages.forEach(m => appendMsg(m.role, m.text));
            } else {
                appendMsg('system', 'Sẵn sàng trò chuyện im lặng với XiaoZhi!');
            }
        }

        function appendMsg(role, text) {
            const container = document.getElementById('chat-messages');
            const div = document.createElement('div');
            div.className = 'msg ' + role;
            div.innerText = text;
            container.appendChild(div);
            container.scrollTop = container.scrollHeight;
        }

        function startHttpPolling() {
            useHttpFallback = true;
            const badge = document.getElementById('status-badge');
            badge.innerText = 'Đã kết nối (HTTP API)';
            badge.className = 'status-badge http';
            
            fetchHistory();
            if (!pollTimer) {
                pollTimer = setInterval(fetchHistory, 1500);
            }
        }

        function fetchHistory() {
            fetch('/api/history')
                .then(res => res.json())
                .then(data => {
                    const count = data.messages ? data.messages.length : 0;
                    if (count !== lastMsgCount) {
                        lastMsgCount = count;
                        renderHistory(data);
                    }
                })
                .catch(err => console.error(err));
        }

        function connectWS() {
            try {
                const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
                ws = new WebSocket(`${protocol}//${location.host}/ws`);
                const badge = document.getElementById('status-badge');

                ws.onopen = () => {
                    useHttpFallback = false;
                    if (pollTimer) clearInterval(pollTimer);
                    badge.innerText = 'Đã kết nối (WebSocket)';
                    badge.className = 'status-badge online';
                };
                ws.onmessage = (event) => {
                    try {
                        const data = JSON.parse(event.data);
                        if (data.type === 'history') {
                            renderHistory(data);
                        } else if (data.type === 'chat') {
                            appendMsg(data.role, data.text);
                        } else if (data.type === 'error') {
                            badge.innerText = 'Bị từ chối';
                            badge.className = 'status-badge error';
                            appendMsg('system', '⚠️ ' + data.message);
                        } else if (data.type === 'config_saved') {
                            alert('Đã lưu cấu hình lịch sử chat thành công!');
                        }
                    } catch(e) { console.error(e); }
                };
                ws.onerror = () => { startHttpPolling(); };
                ws.onclose = () => { startHttpPolling(); };
            } catch(e) {
                startHttpPolling();
            }
        }

        function sendMessage() {
            const input = document.getElementById('msg-input');
            const text = input.value.trim();
            if (!text) return;

            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ type: 'chat', text: text }));
                input.value = '';
            } else {
                fetch('/api/chat', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ text: text })
                })
                .then(res => res.json())
                .then(res => {
                    input.value = '';
                    fetchHistory();
                })
                .catch(err => alert('Lỗi gửi tin nhắn HTTP!'));
            }
        }

        function handleKey(e) { if (e.key === 'Enter') sendMessage(); }

        function saveConfig() {
            const val = parseInt(document.getElementById('history-limit').value);
            if (isNaN(val) || val < 2 || val > 20) {
                alert('Vui lòng nhập số từ 2 đến 20!');
                return;
            }
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ type: 'config', max_history: val }));
            } else {
                fetch('/api/config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ max_history: val })
                })
                .then(res => res.json())
                .then(res => {
                    alert('Đã lưu cấu hình lịch sử chat!');
                    fetchHistory();
                })
                .catch(err => alert('Lỗi lưu cấu hình!'));
            }
        }

        connectWS();
    </script>
</body>
</html>
)rawhtml";

LanChatServer::LanChatServer() {
    Settings settings("lan_chat");
    max_history_ = settings.GetInt("max_history", 6);
    if (max_history_ < 2) max_history_ = 6;
}

LanChatServer::~LanChatServer() {
    Stop();
}

void LanChatServer::SetMaxHistory(int max_history) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_history_ = max_history;
    Settings settings("lan_chat", true);
    settings.SetInt("max_history", max_history_);
    while (history_.size() > static_cast<size_t>(max_history_)) {
        history_.pop_front();
    }
}

std::string LanChatServer::BuildHistoryJsonStr() {
    std::lock_guard<std::mutex> lock(mutex_);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "history");
    cJSON_AddNumberToObject(root, "max_history", max_history_);
    cJSON* msgs = cJSON_CreateArray();
    for (const auto& msg : history_) {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "role", msg.role.c_str());
        cJSON_AddStringToObject(item, "text", msg.text.c_str());
        cJSON_AddItemToArray(msgs, item);
    }
    cJSON_AddItemToObject(root, "messages", msgs);

    char* json_str = cJSON_PrintUnformatted(root);
    std::string result = json_str ? json_str : "{}";
    if (json_str) free(json_str);
    cJSON_Delete(root);
    return result;
}

void LanChatServer::OnChatMessage(const std::string& role, const std::string& text) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (text.empty()) return;

    // Append to history
    history_.push_back({role, text});
    while (history_.size() > static_cast<size_t>(max_history_)) {
        history_.pop_front();
    }

#if CONFIG_HTTPD_WS_SUPPORT
    // Broadcast to active client if connected via WebSocket
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "chat");
    cJSON_AddStringToObject(root, "role", role.c_str());
    cJSON_AddStringToObject(root, "text", text.c_str());
    char* json_str = cJSON_PrintUnformatted(root);
    if (json_str != nullptr) {
        BroadcastMessage(json_str);
        free(json_str);
    }
    cJSON_Delete(root);
#endif
}

esp_err_t LanChatServer::IndexHandler(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, LAN_CHAT_HTML, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t LanChatServer::ApiHistoryHandler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    std::string json_str = GetInstance().BuildHistoryJsonStr();
    httpd_resp_send(req, json_str.c_str(), json_str.length());
    return ESP_OK;
}

esp_err_t LanChatServer::ApiChatHandler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char buf[512] = {0};
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid body");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    cJSON* root = cJSON_Parse(buf);
    if (root != nullptr) {
        auto text = cJSON_GetObjectItem(root, "text");
        if (cJSON_IsString(text) && text->valuestring != nullptr) {
            std::string msg(text->valuestring);
            ESP_LOGI(TAG, "Received HTTP LAN chat message: %s", msg.c_str());
            Application::GetInstance().SendTextMessage(msg);
        }
        cJSON_Delete(root);
    }

    const char* resp = "{\"status\":\"ok\"}";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t LanChatServer::ApiConfigHandler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char buf[256] = {0};
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret > 0) {
        buf[ret] = '\0';
        cJSON* root = cJSON_Parse(buf);
        if (root != nullptr) {
            auto max_hist = cJSON_GetObjectItem(root, "max_history");
            if (cJSON_IsNumber(max_hist)) {
                int limit = max_hist->valueint;
                if (limit >= 2 && limit <= 20) {
                    GetInstance().SetMaxHistory(limit);
                }
            }
            cJSON_Delete(root);
        }
    }

    const char* resp = "{\"status\":\"ok\"}";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

#if CONFIG_HTTPD_WS_SUPPORT
esp_err_t LanChatServer::WsHandler(httpd_req_t* req) {
    auto& instance = GetInstance();
    int sock_fd = httpd_req_to_sockfd(req);

    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "WebSocket handshake from fd=%d", sock_fd);
        instance.AddClient(req);
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt = {};
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        return ret;
    }

    uint8_t* buf = nullptr;
    if (ws_pkt.len > 0) {
        buf = static_cast<uint8_t*>(calloc(1, ws_pkt.len + 1));
        if (buf == nullptr) {
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            free(buf);
            return ret;
        }
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_CLOSE) {
        instance.RemoveClient(req);
        free(buf);
        return ESP_OK;
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && buf != nullptr) {
        buf[ws_pkt.len] = '\0';
        instance.HandleWsMessage(req, reinterpret_cast<const char*>(buf), ws_pkt.len);
    }

    free(buf);
    return ESP_OK;
}

void LanChatServer::AddClient(httpd_req_t* req) {
    int sock_fd = httpd_req_to_sockfd(req);
    std::lock_guard<std::mutex> lock(mutex_);

    if (active_client_fd_ != -1 && active_client_fd_ != sock_fd) {
        ESP_LOGW(TAG, "Closing old client fd=%d, new client fd=%d", active_client_fd_, sock_fd);
        // Overwrite the old client with the new one to allow page reloads
    }

    active_client_fd_ = sock_fd;
    ESP_LOGI(TAG, "Active client set to fd=%d", active_client_fd_);
    SendHistoryToClient(sock_fd);
}

void LanChatServer::RemoveClient(httpd_req_t* req) {
    int sock_fd = httpd_req_to_sockfd(req);
    std::lock_guard<std::mutex> lock(mutex_);
    if (active_client_fd_ == sock_fd) {
        active_client_fd_ = -1;
        ESP_LOGI(TAG, "Active client fd=%d disconnected", sock_fd);
    }
}

void LanChatServer::SendHistoryToClient(int sock_fd) {
    std::string json_str = BuildHistoryJsonStr();
    httpd_ws_frame_t ws_pkt = {};
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t*)json_str.c_str();
    ws_pkt.len = json_str.length();
    httpd_ws_send_frame_async(server_handle_, sock_fd, &ws_pkt);
}

void LanChatServer::HandleWsMessage(httpd_req_t* req, const char* data, size_t len) {
    int sock_fd = httpd_req_to_sockfd(req);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (active_client_fd_ != sock_fd) {
            return;
        }
    }

    cJSON* root = cJSON_ParseWithLength(data, len);
    if (root == nullptr) return;

    auto type = cJSON_GetObjectItem(root, "type");
    if (cJSON_IsString(type)) {
        if (strcmp(type->valuestring, "chat") == 0) {
            auto text = cJSON_GetObjectItem(root, "text");
            if (cJSON_IsString(text) && text->valuestring != nullptr) {
                std::string msg(text->valuestring);
                ESP_LOGI(TAG, "Received WS LAN chat message: %s", msg.c_str());
                Application::GetInstance().SendTextMessage(msg);
            }
        } else if (strcmp(type->valuestring, "config") == 0) {
            auto max_hist = cJSON_GetObjectItem(root, "max_history");
            if (cJSON_IsNumber(max_hist)) {
                int limit = max_hist->valueint;
                if (limit >= 2 && limit <= 20) {
                    SetMaxHistory(limit);
                    cJSON* resp = cJSON_CreateObject();
                    cJSON_AddStringToObject(resp, "type", "config_saved");
                    char* resp_str = cJSON_PrintUnformatted(resp);
                    if (resp_str != nullptr) {
                        httpd_ws_frame_t ws_pkt = {};
                        ws_pkt.type = HTTPD_WS_TYPE_TEXT;
                        ws_pkt.payload = (uint8_t*)resp_str;
                        ws_pkt.len = strlen(resp_str);
                        httpd_ws_send_frame_async(server_handle_, sock_fd, &ws_pkt);
                        free(resp_str);
                    }
                    cJSON_Delete(resp);
                }
            }
        }
    }
    cJSON_Delete(root);
}

struct WsSendJob {
    httpd_handle_t server;
    int fd;
    char* data;
};

static void WsSendJobTask(void* arg) {
    WsSendJob* job = static_cast<WsSendJob*>(arg);
    httpd_ws_frame_t ws_pkt = {};
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t*)job->data;
    ws_pkt.len = strlen(job->data);
    httpd_ws_send_frame_async(job->server, job->fd, &ws_pkt);
    free(job->data);
    delete job;
}

void LanChatServer::BroadcastMessage(const std::string& json_str) {
    if (server_handle_ == nullptr || active_client_fd_ == -1) {
        return;
    }
    WsSendJob* job = new WsSendJob();
    job->server = server_handle_;
    job->fd = active_client_fd_;
    job->data = strdup(json_str.c_str());
    httpd_queue_work(server_handle_, WsSendJobTask, job);
}
#endif

void LanChatServer::Start(int port) {
    if (server_handle_ != nullptr) return;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.ctrl_port = 32770;
    config.max_open_sockets = 5;
    config.stack_size = 8192;
    config.lru_purge_enable = true;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = IndexHandler,
        .user_ctx = nullptr,
        .is_websocket = false,
    };

    httpd_uri_t api_history_uri = {
        .uri = "/api/history",
        .method = HTTP_GET,
        .handler = ApiHistoryHandler,
        .user_ctx = nullptr,
        .is_websocket = false,
    };

    httpd_uri_t api_chat_uri = {
        .uri = "/api/chat",
        .method = HTTP_POST,
        .handler = ApiChatHandler,
        .user_ctx = nullptr,
        .is_websocket = false,
    };

    httpd_uri_t api_config_uri = {
        .uri = "/api/config",
        .method = HTTP_POST,
        .handler = ApiConfigHandler,
        .user_ctx = nullptr,
        .is_websocket = false,
    };

#if CONFIG_HTTPD_WS_SUPPORT
    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = WsHandler,
        .user_ctx = nullptr,
        .is_websocket = true,
    };
#endif

    if (httpd_start(&server_handle_, &config) == ESP_OK) {
        httpd_register_uri_handler(server_handle_, &index_uri);
        httpd_register_uri_handler(server_handle_, &api_history_uri);
        httpd_register_uri_handler(server_handle_, &api_chat_uri);
        httpd_register_uri_handler(server_handle_, &api_config_uri);
#if CONFIG_HTTPD_WS_SUPPORT
        httpd_register_uri_handler(server_handle_, &ws_uri);
#endif
        ESP_LOGI(TAG, "LanChatServer started on port %d (WS + REST Dual Mode)", port);
    } else {
        ESP_LOGE(TAG, "Failed to start LanChatServer on port %d", port);
    }
}

void LanChatServer::Stop() {
    if (server_handle_ != nullptr) {
        httpd_stop(server_handle_);
        server_handle_ = nullptr;
        active_client_fd_ = -1;
        ESP_LOGI(TAG, "LanChatServer stopped");
    }
}
