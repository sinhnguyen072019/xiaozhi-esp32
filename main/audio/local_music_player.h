#ifndef LOCAL_MUSIC_PLAYER_H
#define LOCAL_MUSIC_PLAYER_H

#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class AudioService;

class LocalMusicPlayer {
public:
    static LocalMusicPlayer& GetInstance();

    void Initialize(AudioService* audio_service);

    bool PlayFile(const std::string& filepath);
    bool PlayByName(const std::string& filename);
    void Stop();
    bool IsPlaying() const { return is_playing_; }
    void TogglePause() { is_paused_ = !is_paused_; }
    std::string GetCurrentTrack() const { return current_track_; }

    std::vector<std::string> ListTracks();

private:
    LocalMusicPlayer() = default;
    ~LocalMusicPlayer();

    static void PlaybackTask(void* param);
    void RunPlayback(const std::string& filepath);

    AudioService* audio_service_ = nullptr;
    std::atomic<bool> is_playing_{false};
    std::atomic<bool> is_paused_{false};
    std::atomic<bool> stop_requested_{false};
    TaskHandle_t play_task_handle_ = nullptr;
    std::string current_track_;
    std::mutex mutex_;
};

#endif // LOCAL_MUSIC_PLAYER_H
