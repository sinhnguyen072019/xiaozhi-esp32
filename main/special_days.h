#ifndef SPECIAL_DAYS_H
#define SPECIAL_DAYS_H

#include <cstdlib>
#include <string>
#include <vector>
#include "settings.h"

struct SpecialDay {
    int month;             // Tháng (0-11, 0 là tháng 1)
    int day;               // Ngày (1-31)
    const char* greeting;  // Lời chúc
};

struct CustomSpecialDay {
    int month;             // Tháng (0-11, 0 là tháng 1)
    int day;               // Ngày (1-31)
    std::string greeting;  // Lời chúc
};

//(Nhớ lưu ý tháng luôn bắt đầu từ số 0 nhé)
inline const SpecialDay SPECIAL_DAYS[] = {
    {0, 1, "Chúc mừng Năm mới! 🎉"},             // 1/1: Tết Dương lịch (tháng 0)
    {3, 30, "Mừng Ngày Chiến thắng 30/4! 🇻🇳"},   // 30/4: Ngày Giải phóng miền Nam
    {4, 1, "Mừng Ngày Quốc tế Lao động 1/5!"},   // 1/5: Quốc tế Lao động
    {5, 1, "Mừng Ngày Quốc tế Thiếu nhi 1/6!"},  // 1/6: Quốc tế Thiếu nhi
    {6, 23, " ^ ^ Đại hải trình"},               // 23/7 (gốc)
    {6, 30, "Chúc mừng sinh nhật"},              // 30/7 (gốc)
    {8, 2, "Chúc mừng Quốc khánh 2/9! 🇻🇳"},      // 2/9: Quốc khánh
    {10, 20, "Chúc mừng Ngày Nhà giáo 20/11!"},  // 20/11: Ngày Nhà giáo Việt Nam
    {11, 24, "Chúc mừng Giáng sinh! 🎄"},        // 24/12: Giáng sinh
};

inline const int NUM_SPECIAL_DAYS = sizeof(SPECIAL_DAYS) / sizeof(SpecialDay);

inline bool IsHardcodedSpecialDay(int month_0_idx, int day) {
    for (int i = 0; i < NUM_SPECIAL_DAYS; ++i) {
        if (SPECIAL_DAYS[i].month == month_0_idx && SPECIAL_DAYS[i].day == day) {
            return true;
        }
    }
    return false;
}

inline std::vector<CustomSpecialDay> GetCustomSpecialDays() {
    std::vector<CustomSpecialDay> days;
    Settings settings("special_days", false);
    std::string list_str = settings.GetString("list", "");
    if (!list_str.empty()) {
        size_t start = 0;
        while (start < list_str.length()) {
            size_t end = list_str.find('|', start);
            if (end == std::string::npos) {
                end = list_str.length();
            }
            std::string item = list_str.substr(start, end - start);
            size_t first_comma = item.find(',');
            if (first_comma != std::string::npos) {
                size_t second_comma = item.find(',', first_comma + 1);
                if (second_comma != std::string::npos) {
                    int m = atoi(item.substr(0, first_comma).c_str());
                    int d =
                        atoi(item.substr(first_comma + 1, second_comma - first_comma - 1).c_str());
                    std::string g = item.substr(second_comma + 1);
                    days.push_back({m, d, g});
                }
            }
            start = end + 1;
        }
    } else {
        int custom_month = settings.GetInt("month", -1);
        int custom_day = settings.GetInt("day", -1);
        if (custom_month != -1 && custom_day != -1) {
            std::string g = settings.GetString("greeting", "");
            days.push_back({custom_month, custom_day, g});
        }
    }
    return days;
}

inline bool SaveCustomSpecialDays(const std::vector<CustomSpecialDay>& days) {
    Settings settings("special_days", true);
    if (days.empty()) {
        settings.EraseKey("list");
    } else {
        std::string list_str;
        for (size_t i = 0; i < days.size(); ++i) {
            if (i > 0)
                list_str += "|";
            list_str += std::to_string(days[i].month) + "," + std::to_string(days[i].day) + "," +
                        days[i].greeting;
        }
        settings.SetString("list", list_str);
    }
    settings.EraseKey("month");
    settings.EraseKey("day");
    settings.EraseKey("greeting");
    return true;
}

inline bool AddCustomSpecialDay(int month_0_idx, int day, const std::string& greeting) {
    if (month_0_idx < 0 || month_0_idx > 11 || day < 1 || day > 31)
        return false;
    if (IsHardcodedSpecialDay(month_0_idx, day))
        return false;  // Không thể sửa ngày hardcode
    std::string clean_greeting = greeting;
    for (char& c : clean_greeting) {
        if (c == '|' || c == ',' || c == '\r' || c == '\n')
            c = ' ';
    }
    auto days = GetCustomSpecialDays();
    bool found = false;
    for (auto& d : days) {
        if (d.month == month_0_idx && d.day == day) {
            d.greeting = clean_greeting;
            found = true;
            break;
        }
    }
    if (!found) {
        days.push_back({month_0_idx, day, clean_greeting});
    }
    return SaveCustomSpecialDays(days);
}

inline bool RemoveCustomSpecialDay(int month_0_idx, int day) {
    if (IsHardcodedSpecialDay(month_0_idx, day))
        return false;  // Không thể xóa ngày hardcode
    auto days = GetCustomSpecialDays();
    size_t old_size = days.size();
    for (auto it = days.begin(); it != days.end();) {
        if (it->month == month_0_idx && it->day == day) {
            it = days.erase(it);
        } else {
            ++it;
        }
    }
    if (days.size() == old_size)
        return false;
    return SaveCustomSpecialDays(days);
}

inline bool ClearCustomSpecialDays() {
    Settings settings("special_days", true);
    settings.EraseKey("list");
    settings.EraseKey("month");
    settings.EraseKey("day");
    settings.EraseKey("greeting");
    return true;
}

inline bool GetSpecialDayGreeting(int month, int day, std::string& out_greeting) {
    // Ưu tiên kiểm tra ngày hardcode trước (không thể bị ghi đè)
    for (int i = 0; i < NUM_SPECIAL_DAYS; ++i) {
        if (SPECIAL_DAYS[i].month == month && SPECIAL_DAYS[i].day == day) {
            out_greeting = SPECIAL_DAYS[i].greeting ? SPECIAL_DAYS[i].greeting : "";
            return true;
        }
    }

    auto custom_days = GetCustomSpecialDays();
    for (const auto& d : custom_days) {
        if (d.month == month && d.day == day) {
            out_greeting = d.greeting;
            return true;
        }
    }
    return false;
}

#endif  // SPECIAL_DAYS_H
