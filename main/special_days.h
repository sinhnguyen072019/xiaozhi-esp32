#ifndef SPECIAL_DAYS_H
#define SPECIAL_DAYS_H

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

#endif  // SPECIAL_DAYS_H
