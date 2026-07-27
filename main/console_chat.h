#ifndef MAIN_CONSOLE_CHAT_H_
#define MAIN_CONSOLE_CHAT_H_

#include <string>

class ConsoleChat {
public:
    static ConsoleChat& GetInstance() {
        static ConsoleChat instance;
        return instance;
    }

    void Start();

private:
    ConsoleChat() = default;
    ~ConsoleChat() = default;

    bool started_ = false;
    void RegisterCommands();
};

#endif  // MAIN_CONSOLE_CHAT_H_
