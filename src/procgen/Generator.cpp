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
    // TODO: Chunks auf einmal generieren. So wiederholte berechnungen sparen.
    //       Das Array quasi "vorberechen" und cachen.
    //       Im Frontend dann chunk für chunk anstatt Reihe für Reihe rendern/laden.

    // TODO: evtl. intrinsics;

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

        float rand_top_left    = random(chunk_x, chunk_y, seed);
        float rand_top_right   = random(chunk_x + current_chunk_size, chunk_y, seed);
        float rand_bottom_left = random(chunk_x, chunk_y + current_chunk_size, seed);
        float rand_bottom_right
            = random(chunk_x + current_chunk_size, chunk_y + current_chunk_size, seed);

        noise += lerp(lerp(rand_top_left, rand_bottom_left, normal_tile_y),
                      lerp(rand_top_right, rand_bottom_right, normal_tile_y),
                      normal_tile_x)
                 * scale;

        scale_acc += scale;
        scale /= bias;
    }

    return noise / scale_acc;
}

void generate_chunk(const std::vector<float>& seed_values,
                    const int                 chunk_size,
                    int                       sub_chunk_size,
                    int                       x,
                    int                       y,
                    int                       octaves,
                    float                     scale,
                    float                     bias,
                    std::vector<float>&       result)
{
    // not a 'safe' linear interpolation like `std::lerp` but we don't need any special case checking
    static auto lerp  = [](float a, float b, float f) -> float { return a + f * (b - a); };
    static auto val2d = [](const std::vector<float>& vec, int x, int y, int size) -> float {
        return vec[y * size + x];
    };

    // break conditions
    // octaves run out
    if (octaves == 0) return;

    // we can do 1 faster than the rest, since no interpolation is necessary
    if (sub_chunk_size == 1)
    {
        result[y * chunk_size + x] += seed_values[y * chunk_size + x] * scale;
        return;
    }

    // Add current sub-chunk to result
    // 4 points of interpolation
    auto top_left     = val2d(seed_values, x, y, chunk_size + 1);
    auto top_right    = val2d(seed_values, x + sub_chunk_size, y, chunk_size + 1);
    auto bottom_right = val2d(seed_values, x + sub_chunk_size, y + sub_chunk_size, chunk_size + 1);
    auto bottom_left  = val2d(seed_values, x, y + sub_chunk_size, chunk_size + 1);

    for (int sub_y = 0; sub_y < sub_chunk_size; sub_y++)
    {
        float normal_y = float(sub_y) / float(sub_chunk_size);
        for (int sub_x = 0; sub_x < sub_chunk_size; sub_x++)
        {
            float normal_x = float(sub_x) / float(sub_chunk_size);

            result[(y + sub_y) * chunk_size + (x + sub_x)]
                += lerp(lerp(top_left, bottom_left, normal_y),
                        lerp(top_right, bottom_right, normal_y),
                        normal_x)
                   * scale;
        }
    }

    // call self for new 4 subchunks
    sub_chunk_size >>= 1;

    generate_chunk(seed_values,
                   chunk_size,
                   sub_chunk_size,
                   x,
                   y,
                   octaves - 1,
                   scale / bias,
                   bias,
                   result);
    generate_chunk(seed_values,
                   chunk_size,
                   sub_chunk_size,
                   x + sub_chunk_size,
                   y,
                   octaves - 1,
                   scale / bias,
                   bias,
                   result);
    generate_chunk(seed_values,
                   chunk_size,
                   sub_chunk_size,
                   x,
                   y + sub_chunk_size,
                   octaves - 1,
                   scale / bias,
                   bias,
                   result);
    generate_chunk(seed_values,
                   chunk_size,
                   sub_chunk_size,
                   x + sub_chunk_size,
                   y + sub_chunk_size,
                   octaves - 1,
                   scale / bias,
                   bias,
                   result);
}

std::vector<float> procgen::Generator::generate_chunk(int chunk_x, int chunk_y, int seed_offset)
{
    const auto seed = world_seed + seed_offset;

    std::vector<float> random_values(chunk_size * chunk_size + 2 * chunk_size + 1);

    for (int y = 0; y <= chunk_size; y++)
        for (int x = 0; x <= chunk_size; x++)
            random_values[y * (chunk_size + 1) + x] = random(x + chunk_x, y + chunk_y, seed);

    if (!use_perlin) return random_values;

    // initialization
    std::vector<float> result(chunk_size * chunk_size, 0.0f);

    float scale = 1.0f;

    ::generate_chunk(random_values, chunk_size, chunk_size, 0, 0, octaves, scale, bias, result);

    // scale_acc to normalize
    float scale_acc = 0.0f;
    for (int i = 0; i < octaves && chunk_size >> i > 0; i++)
    {
        scale_acc += scale;
        scale /= bias;
    }

    for (auto& v : result) v /= scale_acc;

    return result;
}
