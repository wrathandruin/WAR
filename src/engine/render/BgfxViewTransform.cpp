#include "engine/render/BgfxViewTransform.h"

#if defined(__has_include)
#  if __has_include(<bgfx/bgfx.h>) && __has_include(<bx/math.h>)
#    define WAR_HAS_BGFX 1
#  else
#    define WAR_HAS_BGFX 0
#  endif
#else
#  define WAR_HAS_BGFX 0
#endif

#if WAR_HAS_BGFX
#  include <bgfx/bgfx.h>
#  include <bx/math.h>
#endif

namespace war
{
    void BgfxViewTransform::buildMatrices(
        const Camera2D& camera,
        int viewWidth,
        int viewHeight,
        float outView[16],
        float outProj[16])
    {
#if WAR_HAS_BGFX
        bx::mtxIdentity(outView);

        const Vec2 topLeft = camera.screenToWorld(0.0f, 0.0f);
        const Vec2 bottomRight = camera.screenToWorld(
            static_cast<float>(viewWidth),
            static_cast<float>(viewHeight));

        const bgfx::Caps* caps = bgfx::getCaps();
        const bool homogeneousDepth = caps != nullptr ? caps->homogeneousDepth : true;

        bx::mtxOrtho(
            outProj,
            topLeft.x,
            bottomRight.x,
            bottomRight.y,
            topLeft.y,
            0.0f,
            100.0f,
            0.0f,
            homogeneousDepth);
#else
        (void)camera;
        (void)viewWidth;
        (void)viewHeight;
        (void)outView;
        (void)outProj;
#endif
    }
}
