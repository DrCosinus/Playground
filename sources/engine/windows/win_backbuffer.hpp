#pragma once

#include <game.hpp>

#include <windows.h>

namespace Windows
{

    class BackBuffer : public PIBackBuffer
    {
    public:
        BackBuffer(int width, int height);

        void Resize(int width, int height);

        const PIBackBuffer& PrepareUpdate() const { return *this; }

        // operator const PIBackBuffer&() const { return pibb; }

        void DebugDrawVertical(int32 X, int32 top, int32 bottom, uint32 color) const;

        BITMAPINFO Info;
    };

} // namespace Windows