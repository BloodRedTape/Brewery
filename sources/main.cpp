#include "core/print.hpp"
#include <core/span.hpp>
#include <core/os/clock.hpp>
#include <core/os/window.hpp>
#include <core/os/sleep.hpp>
#include <core/os/stacktrace.hpp>
#include <core/algorithm.hpp>
#include <graphics/api/swapchain.hpp>

#include "windows.cpp"

class Application{
private:
    AutoWindow m_Window{1280, 720, "Brewery"};
    FramebufferChain m_Swapchain{&m_Window};
    ImGuiBackend m_Backend{m_Swapchain.Pass()};
    float FramerateLimit = 60.f;

    Semaphore m_Begin, m_End;
    DatabaseLogger m_Logger;
    Database m_DB{"brewery.sqlite", m_Logger};

    RawVar<Dockspace> m_Dockspace;

    ConsoleWindow m_ConsoleWindow{m_Logger, m_DB};
    DrinksListPanel m_DrinksList{m_DB};
    OrdersLogPanel m_OrdersLog{m_DB};
    WaitersListPanel m_WaitersList{m_DB};
    IngredientsListPanel m_IngredientsList{m_DB};

public:
    Application(){
        Function<void(const Event&)> handler;
        handler.Bind<Application, &Application::OnEvent>(this);
        m_Window.SetEventsHanlder(handler);

        m_Dockspace.Construct(m_Window.Size());

    }

    void Run(){
        Clock cl;

        Println("Stacktrace: %\n", Stacktrace());
        
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
        m_Dockspace->Draw();
        m_ConsoleWindow.Draw();
        m_DrinksList.Draw();
        m_OrdersLog.Draw();
        m_WaitersList.Draw();
        m_IngredientsList.Draw();
        //ImGui::ShowDemoWindow();
    }

    void OnEvent(const Event &e){
        if(e.Type == EventType::WindowClose)
            m_Window.Close();
        if(e.Type == EventType::WindowResized){
            m_Swapchain.Recreate();
            m_Dockspace.Destruct();
            m_Dockspace.Construct(Vector2s(e.WindowResized.x, e.WindowResized.y));
        }

        m_Backend.HandleEvent(e);
    }
};

int StraitXMain(Span<const char *> args){
    Application app;

    app.Run();

    return 0;
}