#include "olcPixelGameEngine.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_pge.h"
#include "imgui/imgui_impl_opengl2.h"

#include "procgen/Generator.h"

class Example : public olc::PixelGameEngine
{
    olc::imgui::PGE_ImGUI pge_imgui;
    int                   gameLayer;

    procgen::Generator gen;

    // Chunk visualization stuff
    const char* chunk_visualization_items[3]{"None", "Dots", "Lines"};
    int         chunk_visualization_current_item = 0;

    // Threshold
    bool  use_threshold = false;
    float threshold     = 0.5f;

public:
    Example() { sAppName = "World Generation"; }

public:
    bool OnUserCreate() override
    {
        //One time initialization of the Dear ImGui library
        ImGui::CreateContext();
        //Create an instance of the Dear ImGui PGE Integration
        pge_imgui = olc::imgui::PGE_ImGUI();

        //The vi2d for pixel size must match the values given in Construct()
        //Otherwise the mouse will not work correctly
        pge_imgui.ImGui_ImplPGE_Init(this);
        //Initialize the OpenGL2 rendering system
        ImGui_ImplOpenGL2_Init();
        //Create a new Layer which will be used for the game
        gameLayer = CreateLayer();
        //The layer is not enabled by default,  so we need to enable it
        EnableLayer(gameLayer, true);
        //Set a custom render function on layer 0.  Since DrawUI is a member of
        //our class, we need to use std::bind
        SetLayerCustomRenderFunction(0, std::bind(&Example::DrawUI, this));

        gen.unit_size = 4;

        auto [mid_x, mid_y] = gen.screen_to_world((ScreenWidth() - gen.unit_size) / 2,
                                                  (ScreenHeight() - gen.unit_size) / 2);

        gen.camera.x = mid_x;
        gen.camera.y = mid_y;

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {

        // Controls
        if (GetKey(olc::W).bHeld) gen.camera.y += 100.0f * fElapsedTime;
        if (GetKey(olc::A).bHeld) gen.camera.x += 100.0f * fElapsedTime;
        if (GetKey(olc::S).bHeld) gen.camera.y -= 100.0f * fElapsedTime;
        if (GetKey(olc::D).bHeld) gen.camera.x -= 100.0f * fElapsedTime;

        //Change the Draw Target to not be Layer 0
        SetDrawTarget((uint8_t)gameLayer);
        //Game Drawing code here

        auto [from_x, from_y] = gen.screen_to_world(0, 0);
        auto [to_x, to_y]     = gen.screen_to_world(ScreenWidth(), ScreenHeight());

#if 1
        auto chunk_mask = gen.chunk_size - 1;

        for (auto chunk_y = int(from_y) & ~chunk_mask; chunk_y < to_y; chunk_y += gen.chunk_size)
        {
            for (auto chunk_x = int(from_x) & ~chunk_mask; chunk_x < to_x;
                 chunk_x += gen.chunk_size)
            {
                auto vals = gen.generate_chunk(chunk_x, chunk_y);
                for (int sub_y = 0; sub_y < gen.chunk_size; sub_y++)
                {
                    for (int sub_x = 0; sub_x < gen.chunk_size; sub_x++)
                    {
                        auto [screen_x, screen_y]
                            = gen.world_to_screen(chunk_x + sub_x, chunk_y + sub_y);
                        auto value = vals[sub_y * gen.chunk_size + sub_x];

                        if (!use_threshold)
                        {
                            FillRect(screen_x,
                                     screen_y,
                                     gen.unit_size,
                                     gen.unit_size,
                                     olc::Pixel(value * 255.0f, value * 255.0f, value * 255.0f));
                        }
                        else
                        {
                            if (value < threshold)
                            {
                                FillRect(screen_x,
                                         screen_y,
                                         gen.unit_size,
                                         gen.unit_size,
                                         olc::BLACK);
                            }
                            else
                            {
                                FillRect(screen_x,
                                         screen_y,
                                         gen.unit_size,
                                         gen.unit_size,
                                         olc::WHITE);
                            }
                        }
                    }
                }
                auto [chunk_screen_x, chunk_screen_y] = gen.world_to_screen(chunk_x, chunk_y);
                if (chunk_visualization_current_item == 1)
                {
                    FillRect(chunk_screen_x,
                             chunk_screen_y,
                             gen.unit_size,
                             gen.unit_size,
                             olc::RED);
                }
                else if (chunk_visualization_current_item == 2)
                {
                    DrawLine(chunk_screen_x,
                             chunk_screen_y,
                             chunk_screen_x,
                             chunk_screen_y + gen.unit_size * gen.chunk_size,
                             olc::RED);
                    DrawLine(chunk_screen_x,
                             chunk_screen_y,
                             chunk_screen_x + gen.unit_size * gen.chunk_size,
                             chunk_screen_y,
                             olc::RED);
                }
            }
        }
#endif
#if 0
        for (int y = from_y - 1; y <= to_y; y++)
        {
            for (int x = from_x - 1; x <= to_x; x++)
            {
                auto [screen_x, screen_y] = gen.world_to_screen(x, y);
                auto value                = gen.perlin(x, y);

                if (!use_threshold)
                {
                    FillRect(screen_x,
                             screen_y,
                             gen.unit_size,
                             gen.unit_size,
                             olc::Pixel(value * 255.0f, value * 255.0f, value * 255.0f));
                }
                else
                {
                    if (value < threshold)
                    {
                        FillRect(screen_x, screen_y, gen.unit_size, gen.unit_size, olc::BLACK);
                    }
                    else
                    {
                        FillRect(screen_x, screen_y, gen.unit_size, gen.unit_size, olc::WHITE);
                    }
                }

                if (chunk_visualization_current_item == 1)
                {
                    if (x % gen.chunk_size == 0 && y % gen.chunk_size == 0)
                    {
                        FillRect(screen_x, screen_y, gen.unit_size, gen.unit_size, olc::RED);
                    }
                }
                else if (chunk_visualization_current_item == 2)
                {
                    if (x % gen.chunk_size == 0)
                    {
                        DrawLine(screen_x, screen_y, screen_x, screen_y + gen.unit_size, olc::RED);
                    }
                    if (y % gen.chunk_size == 0)
                    {
                        DrawLine(screen_x, screen_y, screen_x + gen.unit_size, screen_y, olc::RED);
                    }
                }
            }
        }
#endif

        return true;
    }

    void DrawUI()
    {
        static auto prev_chunk_size = gen.chunk_size;
        //These 3 lines are mandatory per-frame initialization
        ImGui_ImplOpenGL2_NewFrame();
        pge_imgui.ImGui_ImplPGE_NewFrame();
        ImGui::NewFrame();

        //Create and react to your UI here
        ImGui::Begin("Generation");

        ImGui::Combo("Chunk visualization",
                     &chunk_visualization_current_item,
                     chunk_visualization_items,
                     3);

        ImGui::Separator();

        ImGui::Checkbox("Use Hash", &gen.use_hash);
        ImGui::Checkbox("Use Perlin", &gen.use_perlin);

        ImGui::Separator();

        ImGui::InputInt("World Seed", &gen.world_seed);
        ImGui::InputInt("Chunk Size", &gen.chunk_size);
        if (gen.chunk_size < 1) gen.chunk_size = 1;
        if (gen.chunk_size < prev_chunk_size)
        {
            gen.chunk_size  = prev_chunk_size >> 1;
            prev_chunk_size = gen.chunk_size;
        }
        else if (gen.chunk_size > prev_chunk_size)
        {
            gen.chunk_size  = prev_chunk_size << 1;
            prev_chunk_size = gen.chunk_size;
        }
        ImGui::InputInt("Octaves", &gen.octaves);
        ImGui::InputInt("Unit Size", &gen.unit_size);
        if (gen.unit_size < 1) gen.unit_size = 1;
        ImGui::SliderFloat("Bias", &gen.bias, 0.1f, 7.0f);

        ImGui::Separator();

        ImGui::Checkbox("Use Threshold", &use_threshold);
        ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f);

        ImGui::Separator();

        ImGui::InputFloat("Camera X", &gen.camera.x);
        ImGui::InputFloat("Camera Y", &gen.camera.y);

        ImGui::End();

        //This finishes the Dear ImGui and renders it to the screen
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    }
};
int main()
{
    Example demo;
    if (demo.Construct(640, 370, 2, 2)) demo.Start();
    return 0;
}