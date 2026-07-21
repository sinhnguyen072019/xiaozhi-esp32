#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <string>
#include <vector>
#include <esp_err.h>

class FlashStorage {
public:
    static FlashStorage& GetInstance();

    esp_err_t Initialize(const char* base_path = "/storage", const char* partition_label = "storage");
    bool IsMounted() const { return mounted_; }
    const std::string& GetBasePath() const { return base_path_; }

    std::vector<std::string> ListFiles(const std::string& extension = "");

private:
    FlashStorage() = default;
    ~FlashStorage() = default;

    bool mounted_ = false;
    std::string base_path_ = "/storage";
    std::string partition_label_ = "storage";
};

#endif // FLASH_STORAGE_H
