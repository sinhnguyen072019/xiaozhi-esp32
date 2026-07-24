#include "flash_storage.h"
#include <esp_log.h>
#include <esp_spiffs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

#define TAG "FlashStorage"

FlashStorage& FlashStorage::GetInstance() {
    static FlashStorage instance;
    return instance;
}

esp_err_t FlashStorage::Initialize(const char* base_path, const char* partition_label) {
    if (mounted_) {
        return ESP_OK;
    }

    base_path_ = base_path ? base_path : "/storage";
    partition_label_ = partition_label ? partition_label : "storage";

    esp_vfs_spiffs_conf_t conf = {.base_path = base_path_.c_str(),
                                  .partition_label = partition_label_.c_str(),
                                  .max_files = 5,
                                  .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret == ESP_OK) {
        mounted_ = true;
        size_t total = 0, used = 0;
        esp_spiffs_info(partition_label_.c_str(), &total, &used);
        ESP_LOGI(TAG,
                 "SPIFFS mounted successfully at %s (partition: %s). Total: %u KB, Used: %u KB",
                 base_path_.c_str(), partition_label_.c_str(), (unsigned)(total / 1024),
                 (unsigned)(used / 1024));
    } else {
        ESP_LOGE(TAG, "Failed to initialize SPIFFS on partition '%s' (0x%x)",
                 partition_label_.c_str(), ret);
    }

    return ret;
}

std::vector<std::string> FlashStorage::ListFiles(const std::string& extension) {
    std::vector<std::string> files;
    if (!mounted_) {
        return files;
    }

    DIR* dir = opendir(base_path_.c_str());
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory %s", base_path_.c_str());
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (extension.empty()) {
            files.push_back(filename);
        } else {
            if (filename.length() >= extension.length() &&
                filename.compare(filename.length() - extension.length(), extension.length(),
                                 extension) == 0) {
                files.push_back(filename);
            }
        }
    }
    closedir(dir);
    return files;
}
