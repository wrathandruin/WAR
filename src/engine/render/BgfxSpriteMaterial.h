#pragma once

namespace war
{
    enum class BgfxSpriteMaterialId
    {
        Player,
        Crate,
        Terminal,
        Locker
    };

    struct BgfxUvRect
    {
        float u0 = 0.0f;
        float v0 = 0.0f;
        float u1 = 1.0f;
        float v1 = 1.0f;
    };

    class BgfxSpriteMaterials
    {
    public:
        [[nodiscard]] static BgfxUvRect uvFor(BgfxSpriteMaterialId material);
        [[nodiscard]] static const char* debugName(BgfxSpriteMaterialId material);
    };
}
