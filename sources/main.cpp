#include "core/print.hpp"
#include <core/span.hpp>
#include <core/os/clock.hpp>
#include <core/os/window.hpp>
#include <core/os/sleep.hpp>
#include <core/os/stacktrace.hpp>
#include <core/algorithm.hpp>
#include <graphics/api/swapchain.hpp>
#include <graphics/api/gpu.hpp>

#include "windows.cpp"

class Application{
private:
    Window m_Window{1280, 720, "Brewery"};
    FramebufferChain m_Swapchain{&m_Window};
    ImGuiBackend m_Backend{m_Swapchain.Pass()};
    UniquePtr<CommandPool> m_CmdPool{
        CommandPool::Create()
    };
    UniquePtr<CommandBuffer, CommandBufferDeleter> m_CmdBuffer{
        m_CmdPool->Alloc(), m_CmdPool.Get()
    };
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
    SourcesListPanel m_SourcesList{m_DB};
    GobletsListPanel m_GobletsList{m_DB};

    StoredProcedures m_Procedures{m_DB};

public:
    Application(){
        m_Window.SetEventsHandler({ this, &Application::OnEvent });

        m_Dockspace.Construct(m_Window.Size());
    }

    void Run(){
        Clock cl;
        Fence fence;
        fence.Signal();
        for(;;){
            float dt = cl.GetElapsedTime().AsSeconds();
            cl.Restart();

            Sleep(Seconds(Max(0.f, 2.f/FramerateLimit - dt)));

            m_Window.DispatchEvents();
            if(!m_Window.IsOpen())
                break;
        
            m_Backend.NewFrame(dt, Mouse::RelativePosition(m_Window), m_Window.Size());
            OnImGui();

            m_Swapchain.AcquireNext(&m_Begin);
            {
                fence.WaitAndReset();
                m_CmdBuffer->Begin();
                m_Backend.CmdRenderFrame(m_CmdBuffer.Get(), m_Swapchain.CurrentFramebuffer());
                m_CmdBuffer->End();
                GPU::Execute(m_CmdBuffer.Get(), m_Begin, m_End, fence);
            }
            m_Swapchain.PresentCurrent(&m_End);
        }
        GPU::WaitIdle();
    }

    void OnImGui(){
        m_Dockspace->Draw();
        m_ConsoleWindow.Draw();
        m_DrinksList.Draw();
        m_OrdersLog.Draw();
        m_WaitersList.Draw();
        m_IngredientsList.Draw();
        m_SourcesList.Draw();
        m_GobletsList.Draw();
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

int main(){
    Application app;
    app.Run();

    return 0;
}