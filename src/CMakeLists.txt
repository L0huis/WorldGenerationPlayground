
add_library(procgen procgen/Generator.cpp procgen/Generator.h)

add_executable(main main.cpp olcPixelGameEngine.cpp olcPixelGameEngine.h imgui/imgui_impl_pge.h imgui/imgui_impl_pge.cpp imgui/imconfig.h imgui/imgui.cpp imgui/imgui.h imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_impl_opengl2.cpp imgui/imgui_impl_opengl2.h imgui/imgui_internal.h imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imstb_rectpack.h imgui/imstb_textedit.h imgui/imstb_truetype.h procgen/Generator.cpp procgen/Generator.h)

target_link_libraries(main
    PRIVATE
    project_options
    project_warnings
    procgen)