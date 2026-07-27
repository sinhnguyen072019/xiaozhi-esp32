#include "console_chat.h"

#include <esp_console.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <cstring>

#include "application.h"
#include "board.h"
#include "sdkconfig.h"

#define TAG "ConsoleChat"

static int ChatCommandCallback(int argc, char** argv) {
    if (argc < 2) {
        printf("Sử dụng: %s <nội dung tin nhắn>\n", argv[0]);
        return 1;
    }
    std::string message;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) {
            message += " ";
        }
        message += argv[i];
    }
    printf("Gửi tin nhắn tới AI: \"%s\"\n", message.c_str());
    Application::GetInstance().SendTextMessage(message);
    return 0;
}

static int StatusCommandCallback(int argc, char** argv) {
    auto state = Application::GetInstance().GetDeviceState();
    const char* state_str = "Unknown";
    switch (state) {
        case kDeviceStateIdle:
            state_str = "Idle";
            break;
        case kDeviceStateConnecting:
            state_str = "Connecting";
            break;
        case kDeviceStateListening:
            state_str = "Listening";
            break;
        case kDeviceStateSpeaking:
            state_str = "Speaking";
            break;
        case kDeviceStateStarting:
            state_str = "Starting";
            break;
        case kDeviceStateWifiConfiguring:
            state_str = "WifiConfiguring";
            break;
        case kDeviceStateActivating:
            state_str = "Activating";
            break;
        case kDeviceStateUpgrading:
            state_str = "Upgrading";
            break;
        default:
            break;
    }
    printf("Trạng thái thiết bị: %s\n", state_str);
    return 0;
}

void ConsoleChat::RegisterCommands() {
    const esp_console_cmd_t chat_cmd = {.command = "chat",
                                        .help = "Gửi tin nhắn văn bản tới AI (Texting với AI)",
                                        .hint = "<nội dung tin nhắn>",
                                        .func = ChatCommandCallback,
                                        .argtable = nullptr};
    ESP_ERROR_CHECK(esp_console_cmd_register(&chat_cmd));

    const esp_console_cmd_t send_cmd = {
        .command = "send",
        .help = "Gửi tin nhắn văn bản tới AI (cùng chức năng với lệnh chat)",
        .hint = "<nội dung tin nhắn>",
        .func = ChatCommandCallback,
        .argtable = nullptr};
    ESP_ERROR_CHECK(esp_console_cmd_register(&send_cmd));

    const esp_console_cmd_t status_cmd = {.command = "status",
                                          .help = "Kiểm tra trạng thái thiết bị hiện tại",
                                          .hint = nullptr,
                                          .func = StatusCommandCallback,
                                          .argtable = nullptr};
    ESP_ERROR_CHECK(esp_console_cmd_register(&status_cmd));
}

void ConsoleChat::Start() {
    if (started_) {
        return;
    }
    started_ = true;

    ESP_LOGI(TAG, "Khởi tạo Console Chat qua USB Type-C / UART");

    esp_console_repl_t* repl = nullptr;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.max_cmdline_length = 1024;
    repl_config.prompt = "XiaoZhi>";

    RegisterCommands();

    esp_err_t err = ESP_OK;
#if defined(CONFIG_ESP_CONSOLE_USB_CDC)
    esp_console_dev_usb_cdc_config_t hw_config = ESP_CONSOLE_DEV_CDC_CONFIG_DEFAULT();
    err = esp_console_new_repl_usb_cdc(&hw_config, &repl_config, &repl);
#elif defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    esp_console_dev_usb_serial_jtag_config_t hw_config =
        ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    err = esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl);
#else
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    err = esp_console_new_repl_uart(&hw_config, &repl_config, &repl);
#endif

    if (err == ESP_OK && repl != nullptr) {
        esp_console_start_repl(repl);
        ESP_LOGI(TAG, "Console REPL started thành công");
    } else {
        ESP_LOGI(TAG, "Console REPL init status: %d (có thể đã được khởi tạo trước đó bởi board)",
                 err);
    }
}
