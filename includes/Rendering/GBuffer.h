#pragma once
#include "FrameBuffer.h"



class GBuffer
{
public:
    FrameBuffer frameBufferHolder;

    unsigned gDepth{};
    unsigned gColorSpec{};
    unsigned gNormal{};
    unsigned gMaterial{};

    unsigned stencilBuffer{};

    void setup(int width, int height);
    void update(int width, int height);
};

