#include "Application.h"
#include "imgui.h"

namespace MyApp
{
    void RenderUI()
    {
        static int counter = 0;
        //bool show_demo_window = true;

        //if (show_demo_window)
        //    ImGui::ShowDemoWindow(&show_demo_window);


        ImGui::Begin("Settings");


       
        if(ImGui::Button("Hello"))
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        static float value = 0.0f;
        ImGui::DragFloat("Value", &value);
        //ImGui::Checkbox("Demo Window", &show_demo_window);
        ImGui::End();
    }
}
