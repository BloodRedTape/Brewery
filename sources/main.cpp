#include "core/print.hpp"
#include <core/span.hpp>
#include <core/os/clock.hpp>
#include <core/os/window.hpp>
#include <core/os/sleep.hpp>
#include <core/algorithm.hpp>
#include <graphics/api/swapchain.hpp>
#include <imgui/backend.hpp>
#include <sqlite3.h>
#include <tuple>

class Database{
private:
    sqlite3 *m_Handle = nullptr;

    using CallbackType = Function<void(int, char**, char**)>;
public:

    Database(const char *filepath){
        sqlite3_open(filepath, &m_Handle);
    }

    ~Database(){
        sqlite3_close(m_Handle);
    }

    template<typename...ArgsType>
    bool Execute(const char *fmt, ArgsType&&...args){
        const size_t buffer_size = 4096;
        struct Buffer{
            size_t Written = 0;
            char Data[buffer_size];
        }buffer;

        buffer.Data[0] = 0;

        auto writer = [](char ch, void *user_data){
            Buffer *buffer = (Buffer*)user_data;
            buffer->Data[buffer->Written++] = ch;
        };

        WriterPrint(writer, &buffer, fmt, Forward<ArgsType>(args)...);

        char *message = nullptr;
        if(sqlite3_exec(m_Handle, buffer.Data, nullptr, nullptr,&message) != SQLITE_OK){
            Println("SQLite3: %", message);
            sqlite3_free(message);
            return false;
        }

        return true;
    }
};

struct AutoWindow: Window{
    AutoWindow(int width, int height, const char *title){
        Open(width, height, title);
    }

    ~AutoWindow(){
        if(IsOpen())
            Close();
    }
};

class Dockspace{
private:
    const Vector2s m_WindowSize;
public:
    Dockspace(Vector2s window_size):
        m_WindowSize(window_size)
    {}

    void Draw()const{
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::SetNextWindowSize(ImVec2(m_WindowSize.x, m_WindowSize.y)); 
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
};

class Application{
private:
    AutoWindow m_Window{1280, 720, "Brewery"};
    FramebufferChain m_Swapchain{&m_Window};
    ImGuiBackend m_Backend{m_Swapchain.Pass()};
    float FramerateLimit = 60.f;

    Semaphore m_Begin, m_End;

    Database m_DB{"brewery.sqlite"};

    RawVar<Dockspace> m_Dockspace;
public:
    Application(){
        Function<void(const Event&)> handler;
        handler.Bind<Application, &Application::OnEvent>(this);
        m_Window.SetEventsHanlder(handler);

        m_Dockspace.Construct(m_Window.Size());

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
        m_Dockspace->Draw();
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