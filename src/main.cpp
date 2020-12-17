#include "olcPixelGameEngine.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_pge.h"
#include "imgui/imgui_impl_opengl2.h"

class Example : public olc::PixelGameEngine
{
    olc::imgui::PGE_ImGUI pge_imgui;
    int                   gameLayer;

public:
    Example() { sAppName = "Test Application"; }

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
        return true;
    }
    bool OnUserUpdate(float fElapsedTime) override
    {
        //Change the Draw Target to not be Layer 0
        SetDrawTarget((uint8_t)gameLayer);
        //Game Drawing code here
        return true;
    }
    void DrawUI(void)
    {
        //These 3 lines are mandatory per-frame initialization
        ImGui_ImplOpenGL2_NewFrame();
        pge_imgui.ImGui_ImplPGE_NewFrame();
        ImGui::NewFrame();
        //Create and react to your UI here
        ImGui::ShowDemoWindow();
        //
        //This finishes the Dear ImGui and renders it to the screen
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    }
};
int main()
{
    Example demo;
    if (demo.Construct(640, 360, 2, 2)) demo.Start();
    return 0;
}