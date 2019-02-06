#include "win_backbuffer.hpp"

namespace Windows
{
    // Pixels are always 32-bits wide, Memory Order BB GG RR XX
    static constexpr int BytesPerPixel = 4;

    BackBuffer::BackBuffer(int width, int height) { Resize(width, height); }

    void BackBuffer::Resize(int _Width, int _Height)
    {
        if (Memory)
        {
            VirtualFree(Memory, 0, MEM_RELEASE);
        }
        Width  = _Width;
        Height = _Height;

        Info.bmiHeader.biSize        = sizeof(Info.bmiHeader);
        Info.bmiHeader.biWidth       = Width;
        Info.bmiHeader.biHeight      = -Height;
        Info.bmiHeader.biPlanes      = 1;
        Info.bmiHeader.biBitCount    = 32;
        Info.bmiHeader.biCompression = BI_RGB;

        BytesPerPixel        = Windows::BytesPerPixel;
        int BitmapMemorySize = Width * Height * BytesPerPixel;
        Memory               = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        Pitch                = Width * BytesPerPixel;

        // Clear this to black: Memory allocated by VirtualAlloc is automatically initialized to zero.
    }

    void BackBuffer::DebugDrawVertical(int32 X, int32 top, int32 bottom, uint32 color) const
    {
        if (top > Height || bottom < 0)
        {
            return;
        }
        if (top <= 0)
        {
            top = 0;
        }

        if (bottom > Height)
        {
            bottom = Height;
        }

        if ((X >= 0) && (X < Width))
        {
            auto pixel = static_cast<uint8*>(Memory) + X * BytesPerPixel + top * Pitch;
            for (int Y = top; Y < bottom; ++Y)
            {
                *reinterpret_cast<uint32*>(pixel) = color;
                pixel += Pitch;
            }
        }
    }

} // namespace Windows