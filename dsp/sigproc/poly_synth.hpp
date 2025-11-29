#pragma once
#include <cstdint>
#include <span>

// 用于管理多个音符（notes）的播放、停止以及音频处理。它支持多复音（polyphony），即同时播放多个音符。

namespace dsp {

template<class T>
struct PolySynth final{
public:
    static constexpr uint32_t POLYONIC_MAX_COUNT = 8;

    struct Config{
        uint32_t fs;
    };

    void init(uint32_t sampleRate) {
        for (auto& note : notes_) {
            note.init(sampleRate);
        }
        numUsedNotes_ = 0;
        numUnusedNotes_ = POLYONIC_MAX_COUNT;
        for (uint32_t i = 0; i < POLYONIC_MAX_COUNT; ++i) {
            unusedNotes_[i] = &notes_[i];
        }
    }

    void process(std::span<float> buffer, std::span<float> auxBuffer) {
        if (numUsedNotes_ == 0) {
            std::fill_n(buffer.begin(), buffer.size(), 0);
            return;
        }

        // Process first note
        bool shouldRemove = usedNotes_[0]->process(buffer, auxBuffer);
        if (!shouldRemove) {
            // Process remaining notes if first note wasn't removed
            for (uint32_t i = 1; i < numUsedNotes_;) {
                if (usedNotes_[i]->add_to(buffer, auxBuffer)) {
                    remove_note(i);
                } else {
                    ++i;
                }
            }
            return;
        }

        // First note was removed
        remove_note(0);
        
        // Process all remaining notes
        for (uint32_t i = 0; i < numUsedNotes_;) {
            if (usedNotes_[i]->add_to(buffer, auxBuffer)) {
                remove_note(i);
            } else {
                ++i;
            }
        }
    }

    // Helper function to avoid code duplication
    void remove_note(uint32_t index) {
        unusedNotes_[numUnusedNotes_++] = usedNotes_[index];
        std::swap(usedNotes_[index], usedNotes_[numUsedNotes_ - 1]);
        --numUsedNotes_;
    }

    void note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
        if (numUnusedNotes_ > 0) {
            auto* used = unusedNotes_[--numUnusedNotes_];
            used->note_on(channel, note, velocity / 127.0f);
            usedNotes_[numUsedNotes_++] = used;
        }
        else {
            notes_[roundrobinPos_].note_on(channel, note, velocity / 127.0f);
            ++roundrobinPos_;
            roundrobinPos_ &= (POLYONIC_MAX_COUNT - 1);
        }
    }

    void note_off(uint8_t note) {
        for (uint32_t i = 0; i < numUsedNotes_; ++i) {
            if (usedNotes_[i]->is_playing(note)) {
                usedNotes_[i]->note_off();
            }
        }
    }

    void force_stop_all() {
        for (uint32_t i = 0; i < numUsedNotes_; ++i) {
            usedNotes_[i]->panic();
        }
        numUsedNotes_ = 0;
        numUnusedNotes_ = POLYONIC_MAX_COUNT;
        for (uint32_t i = 0; i < POLYONIC_MAX_COUNT; ++i) {
            unusedNotes_[i] = &notes_[i];
        }
    }

    std::span<T*> get_used_notes() { return std::span<T*>(usedNotes_, numUsedNotes_); }
    std::span<T> get_notes() { return std::span<T>(notes_, POLYONIC_MAX_COUNT); }
private:
    T notes_[POLYONIC_MAX_COUNT];
    T* usedNotes_[POLYONIC_MAX_COUNT]{};
    T* unusedNotes_[POLYONIC_MAX_COUNT]{};
    uint32_t numUsedNotes_{};
    uint32_t numUnusedNotes_{};
    uint32_t roundrobinPos_{};
};

}