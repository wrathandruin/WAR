#pragma once

#include "engine/render/Camera2D.h"

namespace war
{
    class BgfxViewTransform
    {
    public:
        static void buildMatrices(
            const Camera2D& camera,
            int viewWidth,
            int viewHeight,
            float outView[16],
            float outProj[16]);
    };
}
