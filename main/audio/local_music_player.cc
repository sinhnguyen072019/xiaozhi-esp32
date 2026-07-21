#include "local_music_player.h"
#include "flash_storage.h"
#include "audio_service.h"
#include "ogg_demuxer.h"
#include "board.h"
#include <esp_log.h>
#include <cstdio>
#include <cstring>

#define TAG "LocalMusicPlayer"

LocalMusicPlayer& LocalMusicPlayer::GetInstance() {
    static LocalMusicPlayer instance;
    return instance;
}

LocalMusicPlayer::~LocalMusicPlayer() {
    Stop();
}

void LocalMusicPlayer::Initialize(AudioService* audio_service) {
    audio_service_ = audio_service;
    FlashStorage::GetInstance().Initialize();
}

std::vector<std::string> LocalMusicPlayer::ListTracks() {
    return FlashStorage::GetInstance().ListFiles();
}

bool LocalMusicPlayer::PlayByName(const std::string& filename) {
    std::string base_path = FlashStorage::GetInstance().GetBasePath();
    std::string full_path = base_path + "/" + filename;
    return PlayFile(full_path);
}

bool LocalMusicPlayer::PlayFile(const std::string& filepath) {
    Stop();

    std::lock_guard<std::mutex> lock(mutex_);
    current_track_ = filepath;
    stop_requested_ = false;

    BaseType_t ret = xTaskCreate(PlaybackTask, "local_play_task", 6144, this, 5, &play_task_handle_);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create local playback task");
        return false;
    }
    return true;
}

void LocalMusicPlayer::Stop() {
    stop_requested_ = true;
    if (play_task_handle_ != nullptr) {
        for (int i = 0; i < 50 && is_playing_; i++) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    is_playing_ = false;
}

void LocalMusicPlayer::PlaybackTask(void* param) {
    auto* self = static_cast<LocalMusicPlayer*>(param);
    self->RunPlayback(self->current_track_);
    self->play_task_handle_ = nullptr;
    vTaskDelete(NULL);
}

void LocalMusicPlayer::RunPlayback(const std::string& filepath) {
    FILE* file = fopen(filepath.c_str(), "rb");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open audio file: %s", filepath.c_str());
        return;
    }

    ESP_LOGI(TAG, "Playing local music: %s", filepath.c_str());
    is_playing_ = true;

    auto codec = Board::GetInstance().GetAudioCodec();
    if (codec && !codec->output_enabled()) {
        codec->EnableOutput(true);
    }

    auto demuxer = std::make_unique<OggDemuxer>();
    demuxer->OnDemuxerFinished([this](const uint8_t* data, int sample_rate, size_t size) {
        if (stop_requested_ || !audio_service_) return;
        auto packet = std::make_unique<AudioStreamPacket>();
        packet->sample_rate = sample_rate;
        packet->frame_duration = 60;
        packet->payload.resize(size);
        std::memcpy(packet->payload.data(), data, size);
        audio_service_->PushPacketToDecodeQueue(std::move(packet), true);
    });

    uint8_t buffer[1024];
    while (!stop_requested_) {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
        if (bytes_read == 0) {
            break;
        }
        demuxer->Process(buffer, bytes_read);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    fclose(file);
    is_playing_ = false;
    ESP_LOGI(TAG, "Local music playback finished");
}
