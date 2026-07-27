#ifndef SPECIAL_DAYS_H
#define SPECIAL_DAYS_H

#include <string>
#include <cstdlib>
#include "settings.h"

struct SpecialDay {
    int month;             // Tháng (0-11, 0 là tháng 1)
    int day;               // Ngày (1-31)
    const char* greeting;  // Lời chúc
};

//(Nhớ lưu ý tháng luôn bắt đầu từ số 0 nhé)
inline const SpecialDay SPECIAL_DAYS[] = {
    {6, 30, ""},  // vì tháng bắt đầu từ index 0
    {0, 1, ""},  {11, 24, ""}, {6, 23, ""}, {6, 24, " ^ ^ "},
    // Bạn có thể thêm các ngày đặc biệt khác vào đây
};

inline const int NUM_SPECIAL_DAYS = sizeof(SPECIAL_DAYS) / sizeof(SpecialDay);

inline bool GetSpecialDayGreeting(int month, int day, std::string& out_greeting) {
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
                    int d = atoi(item.substr(first_comma + 1, second_comma - first_comma - 1).c_str());
                    if (m == month && d == day) {
                        out_greeting = item.substr(second_comma + 1);
                        return true;
                    }
                }
            }
            start = end + 1;
        }
    } else {
        int custom_month = settings.GetInt("month", -1);
        int custom_day = settings.GetInt("day", -1);
        if (custom_month == month && custom_day == day) {
            out_greeting = settings.GetString("greeting", "");
            return true;
        }
    }

    for (int i = 0; i < NUM_SPECIAL_DAYS; ++i) {
        if (SPECIAL_DAYS[i].month == month && SPECIAL_DAYS[i].day == day) {
            out_greeting = SPECIAL_DAYS[i].greeting ? SPECIAL_DAYS[i].greeting : "";
            return true;
        }
    }
    return false;
}

#endif  // SPECIAL_DAYS_H
