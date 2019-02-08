#pragma once

#include "types.hpp"

#include <memory>

namespace Game
{
    struct SoundOutputBuffer;
}

namespace Windows
{

    class Window;

    struct Cursors
    {
        uint32 PlayCursor;
        uint32 WriteCursor;
    };

    struct SoundEngine
    {
        using value_type                          = int16;
        static constexpr int32 BitsPerSample      = sizeof(value_type) * 8;
        static constexpr int32 SamplesPerSecond   = 48000;
        static constexpr int32 ChannelCount       = 2;
        static constexpr int32 BytesPerSample     = sizeof(value_type) * ChannelCount;
        static constexpr int32 WorkBufferSize     = SamplesPerSecond * BytesPerSample; // 1 second
        static constexpr int32 LatencySampleCount = SamplesPerSecond / 12;
        static constexpr int32 BlockAlign         = (ChannelCount * BitsPerSample) / 8;

#if DEBUG_SOUND
        unsigned long lastPlayCursor   = 0;
        unsigned long lastWriteCursor  = 0;
        unsigned long lastByteToLock   = 0;
        unsigned long lastBytesToWrite = 0;
#endif // DEBUG_SOUND

        SoundEngine(const Window&);
        ~SoundEngine();

        Game::SoundOutputBuffer PrepareUpdate();

        void FillSoundBuffer(Game::SoundOutputBuffer& SourceBuffer);

        Cursors GetCursors() const;

    private:
        void Init(const Window&);
        void ClearBuffer() const;
        void Play() const;

        struct SoundBuffer;
        uint32                       RunningSampleIndex = 0;
        std::unique_ptr<SoundBuffer> WorkBuffer;
        int16*                       Samples = nullptr;
        uint32                       ByteToLock;
        uint32                       BytesToWrite;
    };

} // namespace Windows