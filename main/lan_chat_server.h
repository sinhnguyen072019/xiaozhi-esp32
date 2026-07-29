#ifndef LAN_CHAT_SERVER_H_
#define LAN_CHAT_SERVER_H_

#include <esp_http_server.h>
#include <esp_log.h>

#include <deque>
#include <mutex>
#include <string>

struct ChatMessage {
    std::string role;  // "user" or "assistant" or "system"
    std::string text;
};

class LanChatServer {
public:
    static LanChatServer& GetInstance() {
        static LanChatServer instance;
        return instance;
    }

    LanChatServer(const LanChatServer&) = delete;
    LanChatServer& operator=(const LanChatServer&) = delete;

    void Start(int port = 8888);
    void Stop();

    void OnChatMessage(const std::string& role, const std::string& text);

    int GetMaxHistory() const { return max_history_; }
    void SetMaxHistory(int max_history);

private:
    LanChatServer();
    ~LanChatServer();

    static esp_err_t IndexHandler(httpd_req_t* req);
    static esp_err_t ApiHistoryHandler(httpd_req_t* req);
    static esp_err_t ApiChatHandler(httpd_req_t* req);
    static esp_err_t ApiConfigHandler(httpd_req_t* req);
#if CONFIG_HTTPD_WS_SUPPORT
    static esp_err_t WsHandler(httpd_req_t* req);
    void HandleWsMessage(httpd_req_t* req, const char* data, size_t len);
    void AddClient(httpd_req_t* req);
    void RemoveClient(httpd_req_t* req);
    void BroadcastMessage(const std::string& json_str);
    void SendHistoryToClient(int sock_fd);
#endif

    std::string BuildHistoryJsonStr();

    httpd_handle_t server_handle_ = nullptr;
    int active_client_fd_ = -1;
    std::mutex mutex_;

    int max_history_ = 6;
    std::deque<ChatMessage> history_;
};

#endif  // LAN_CHAT_SERVER_H_
