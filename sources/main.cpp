#include <core/span.hpp>
#include <core/os/clock.hpp>
#include <core/os/window.hpp>
#include <graphics/api/swapchain.hpp>
#include <imgui/backend.hpp>

int StraitXMain(Span<const char *> args){
    Window window;
    window.Open(1280, 720, "const char *title");

    FramebufferChain chain(&window);
    ImGuiBackend back(chain.Pass());

    static const auto s_Handler = [&window, &chain, &back](const Event &e){
        if(e.Type == EventType::WindowClose)
            window.Close();
        if(e.Type == EventType::WindowResized)
            chain.Recreate();
        back.HandleEvent(e);
    };

    window.SetEventsHanlder([](const Event &e){
        s_Handler(e);
    });

    Semaphore begin, end;

    Clock cl;

    for(;;){
        float dt = cl.GetElapsedTime().AsSeconds();
        cl.Restart();
        window.DispatchEvents();
        if(!window.IsOpen())
            break;
        
        chain.AcquireNext(&begin);
        back.NewFrame(dt, Mouse::RelativePosition(window), window.Size());


        auto size = window.Size();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::SetNextWindowSize(ImVec2(size.x, size.y)); 
        ImGui::SetNextWindowPos({0, 0});
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoTitleBar;
        flags |= ImGuiWindowFlags_NoResize;
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        flags |= ImGuiWindowFlags_NoMove;

        ImGui::Begin("Brewery", nullptr, flags);
        {
            ImGui::DockSpace(ImGui::GetID("Dockspace"));
        }
        ImGui::End();

        ImGui::PopStyleVar(2);


        ImGui::ShowDemoWindow();

        back.RenderFrame(chain.CurrentFramebuffer(), &begin, &end);
        chain.PresentCurrent(&end);
    }

    return 0;
}