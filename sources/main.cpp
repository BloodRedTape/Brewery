#include <core/span.hpp>
#include <core/os/clock.hpp>
#include <core/os/window.hpp>
#include <core/os/sleep.hpp>
#include <core/algorithm.hpp>
#include <graphics/api/swapchain.hpp>
#include <imgui/backend.hpp>
#include <sqlite3.h>


struct AutoWindow: Window{
    AutoWindow(int width, int height, const char *title){
        Open(width, height, title);
    }

    ~AutoWindow(){
        if(IsOpen())
            Close();
    }
};

class Application{
private:
    AutoWindow m_Window{1280, 720, "Brewery"};
    FramebufferChain m_Swapchain{&m_Window};
    ImGuiBackend m_Backend{m_Swapchain.Pass()};
    float FramerateLimit = 60.f;

    Semaphore m_Begin, m_End;
public:
    Application(){
        Function<void(const Event&)> handler;
        handler.Bind<Application, &Application::OnEvent>(this);
        m_Window.SetEventsHanlder(handler);
    }

    void Run(){
        Clock cl;
        
        for(;;){
            float dt = cl.GetElapsedTime().AsSeconds();
            cl.Restart();

            Sleep(Seconds(Max(0.f, 2.f/FramerateLimit - dt)));

            m_Window.DispatchEvents();
            if(!m_Window.IsOpen())
                break;
        
            m_Swapchain.AcquireNext(&m_Begin);
            m_Backend.NewFrame(dt, Mouse::RelativePosition(m_Window), m_Window.Size());

            OnImGui();

            m_Backend.RenderFrame(m_Swapchain.CurrentFramebuffer(), &m_Begin, &m_End);
            m_Swapchain.PresentCurrent(&m_End);
        }
    }

    void OnImGui(){
        auto size = m_Window.Size();
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
    }

    void OnEvent(const Event &e){
        if(e.Type == EventType::WindowClose)
            m_Window.Close();
        if(e.Type == EventType::WindowResized)
            m_Swapchain.Recreate();

        m_Backend.HandleEvent(e);
    }
};

int StraitXMain(Span<const char *> args){
    Application app;

    app.Run();

    return 0;
}