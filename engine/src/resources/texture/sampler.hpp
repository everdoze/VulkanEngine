#pragma once

#include "texture_types.hpp"

namespace Engine {

    struct SamplerCreateInfo {
        TextureFilterMode filter_minify;
        TextureFilterMode filter_magnify;

        TextureRepeat repeat_u;
        TextureRepeat repeat_v;
        TextureRepeat repeat_w;
    };

    class Sampler {
        public:
            Sampler(SamplerCreateInfo info) {
                filter_magnify = info.filter_magnify;
                filter_minify = info.filter_minify;

                repeat_u = info.repeat_u;
                repeat_v = info.repeat_v;
                repeat_w = info.repeat_w;
            };

            virtual ~Sampler() {};

            TextureFilterMode filter_minify;
            TextureFilterMode filter_magnify;

            TextureRepeat repeat_u;
            TextureRepeat repeat_v;
            TextureRepeat repeat_w;
    };

}
