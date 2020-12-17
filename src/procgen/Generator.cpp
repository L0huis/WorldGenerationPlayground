//
// Created by julianlohuis on 17/12/2020.
//

#include "Generator.h"

#include <limits>

std::pair<float, float> procgen::Generator::world_to_screen(float x, float y) const noexcept
{
    return {
        (x + camera.x) * float(unit_size),  // x
        (y + camera.y) * float(unit_size)   // y
    };
}

std::pair<float, float> procgen::Generator::screen_to_world(float x, float y) const noexcept
{
    return {
        x / float(unit_size) - camera.x,  // x
        y / float(unit_size) - camera.y   // y
    };
}

uint32_t procgen::Generator::hash(uint32_t high, uint32_t low) const noexcept
{
    if (!use_hash) return high * low;

    static uint64_t a = 0x65d200ce55b19ad8ULL;
    static uint64_t b = 0x4f2162926e40c299ULL;
    static uint64_t c = 0x162dd799029970f8ULL;

    auto result = uint32_t((a * low + b * high + c) >> 32);

    // return result * 2654435761;

    result -= (result << 6);
    result ^= (result >> 17);
    result -= (result << 9);
    result ^= (result << 4);
    result -= (result << 3);
    result ^= (result << 10);
    result ^= (result >> 15);
    return result;
}

float procgen::Generator::random(int x, int y, int seed) const noexcept
{
    // combine the 3 integers
    uint32_t result = hash(x ^ seed, y ^ seed);

    return float(result) / float(std::numeric_limits<uint32_t>::max());
}

float procgen::Generator::perlin(int x, int y, int seed_offset) const noexcept
{

    auto seed = world_seed + seed_offset;

    if (!use_perlin) return random(x, y, seed);

    static auto lerp = [](float a, float b, float f) -> float { return a + f * (b - a); };

    int32_t chunk_mask = (chunk_size << 1) - 1;

    float noise     = 0.0f;
    float scale_acc = 0.0f;
    float scale     = 1.0f;

    for (int i = 0; (chunk_mask & 1) && (i < octaves); i++)
    {
        chunk_mask              = (chunk_mask >> 1) | (chunk_mask & 0x80'00'00'00);
        auto current_chunk_size = chunk_mask + 1;

        int inner_tile_x = x & chunk_mask;
        int inner_tile_y = y & chunk_mask;
        int chunk_x      = x - inner_tile_x;
        int chunk_y      = y - inner_tile_y;

        float normal_tile_x = float(inner_tile_x) / float(current_chunk_size);
        float normal_tile_y = float(inner_tile_y) / float(current_chunk_size);

        float rand_top_left     = random(chunk_x, chunk_y, seed);
        float rand_top_right    = random(chunk_x + current_chunk_size, chunk_y, seed);
        float rand_bottom_left  = random(chunk_x, chunk_y + current_chunk_size, seed);
        float rand_bottom_right = random(chunk_x + current_chunk_size, chunk_y + current_chunk_size, seed);

        noise += lerp(lerp(rand_top_left, rand_bottom_left, normal_tile_y),
                      lerp(rand_top_right, rand_bottom_right, normal_tile_y),
                      normal_tile_x)
                 * scale;

        scale_acc += scale;
        scale /= bias;
    }

    return noise / scale_acc;
}
