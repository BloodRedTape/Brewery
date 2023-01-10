#include "core/print.hpp"
#include <core/span.hpp>
#include <core/os/clock.hpp>
#include <core/os/window.hpp>
#include <core/os/sleep.hpp>
#include <core/os/directory.hpp>
#include <core/os/stacktrace.hpp>
#include <core/algorithm.hpp>
#include <graphics/api/swapchain.hpp>
#include <graphics/api/gpu.hpp>
#include "implot.h"

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

    AnalyticsWindow m_Analytics{m_DB};

    StoredProcedures m_Procedures{m_DB};

public:
    Application(){
        ImPlot::CreateContext();
        m_Window.SetEventsHandler({ this, &Application::OnEvent });

        m_Dockspace.Construct(m_Window.Size());
        
        auto &style = ImGui::GetStyle();
        style.WindowPadding = {17, 17};
        style.TabRounding = 8;
        style.ItemSpacing = {10, 10};
        style.WindowRounding = 12;
        style.FrameRounding = 6;
        style.PopupRounding = 12;
        style.ChildRounding = 12;
        style.GrabRounding = 8;

        auto def = ImColor(69, 69, 69, 255);
        auto hovered = ImColor(84, 84, 84, 255);
        auto active = ImColor(120, 120, 120, 255);
        style.Colors[ImGuiCol_FrameBg] = def;
        style.Colors[ImGuiCol_FrameBgHovered] = hovered;
        style.Colors[ImGuiCol_Tab] = def;
        style.Colors[ImGuiCol_TabHovered] = hovered;
        style.Colors[ImGuiCol_TabActive] = active;
        style.Colors[ImGuiCol_Button] = def;
        style.Colors[ImGuiCol_ButtonHovered] = hovered;
        style.Colors[ImGuiCol_TabActive] = active;
        style.Colors[ImGuiCol_CheckMark] = ImColor(210, 210, 210, 255);
        style.Colors[ImGuiCol_SliderGrab] = ImColor(117, 117, 117, 255);
        style.Colors[ImGuiCol_SliderGrabActive] = ImColor(128, 128, 128, 255);

        auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		
		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF(R"(Montserrat-Bold.ttf)", 18);
        m_Backend.RebuildFonts();
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
        m_Analytics.Draw();
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
#ifdef SX_DEBUG
    Directory::Change("../../../");
#endif
    Application app;
    app.Run();

    return 0;
}