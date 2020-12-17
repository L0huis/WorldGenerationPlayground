//
// Created by julianlohuis on 17/12/2020.
//

#ifndef GENERATOR_H
#define GENERATOR_H

#include <utility>
#include <cstdint>

namespace procgen
{
    struct Generator
    {
        // expose settings to imgui
        bool use_hash   = true;
        bool use_perlin = true;

        // general settings
        int   world_seed = 1;
        int   octaves    = 5;
        int   chunk_size = 16;
        int   unit_size  = 1;
        float bias       = 2.0f;

        // camera settings
        struct
        {
            float x = 0;
            float y = 0;
        } camera;

        // conversion functions
        [[nodiscard]] std::pair<float, float> world_to_screen(float x, float y) const noexcept;
        [[nodiscard]] std::pair<float, float> screen_to_world(float x, float y) const noexcept;

        // "uniform" integer hashing function
        [[nodiscard]] uint32_t hash(uint32_t high, uint32_t low) const noexcept;
        [[nodiscard]] uint32_t interleave(uint32_t x, uint32_t y) const noexcept;

        // random functions
        [[nodiscard]] float random(int x, int y, int seed = 0) const noexcept;
        [[nodiscard]] float perlin(int x, int y, int seed_offset = 0) const noexcept;
    };

}  // namespace procgen

#endif  //GENERATOR_H
