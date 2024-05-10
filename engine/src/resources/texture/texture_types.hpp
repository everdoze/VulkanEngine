#pragma once

namespace Engine {

    enum class TextureUse {
        UNKNOWN = 0x00,
        MAP_DIFFUSE = 0x01,
        MAP_SPECULAR = 0x02,
        MAP_NORMAL = 0x04
    };

    enum class TextureFilterMode {
        NEAREST = 0x0,
        LINEAR = 0x1
    };

    enum class TextureRepeat {
        REPEAT = 0x1,
        MIRRORED_REPEAT = 0x2,
        CLAMP_TO_EDGE = 0x3,
        CLAMP_TO_BORDER = 0x4
    };

};