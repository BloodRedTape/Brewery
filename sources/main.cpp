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

template<size_t SizeValue>
class InputBuffer{
private:
    char m_Data[SizeValue];
public:
    InputBuffer(){
        m_Data[0] = 0;
    }

    size_t Size()const{
        return SizeValue;
    }

    char *Data(){
        return m_Data;
    }
};

class QueryResult{
private:
    sqlite3_stmt *const m_Statement;

    bool m_Status;
public:
    QueryResult(sqlite3_stmt *statement):
        m_Statement(statement) 
    {
        Reset();
    }

    ~QueryResult(){
        sqlite3_finalize(m_Statement);
    }

    void Next(){
        m_Status = sqlite3_step(m_Statement) == SQLITE_ROW;
    }

    void Reset(){
        sqlite3_reset(m_Statement);
        Next();
    }

    operator bool()const{
        return m_Status;
    }

    int GetColumnInt(size_t index)const{
        return sqlite3_column_int(m_Statement, index);
    }

    const char *GetColumnString(size_t index)const{
        return (const char*)sqlite3_column_text(m_Statement, index);
    }

    float GetColumnFloat(size_t index)const{
        return sqlite3_column_double(m_Statement, index);
    }

    double GetColumnDouble(size_t index)const{
        return sqlite3_column_double(m_Statement, index);
    }
};

class Stmt{
private:
    static constexpr size_t s_BufferSize = 4096;
    char m_Buffer[s_BufferSize];
    int m_Written = 0;
public:
    template<typename ...Args>
    Stmt(const char *fmt, Args&&...args){
        const auto writer = [](char ch, void *user_data){
            Stmt *stmt = (Stmt*)user_data;
            stmt->m_Buffer[stmt->m_Written++] = ch;
        };

        WriterPrint(writer, &m_Buffer, fmt, Forward<Args>(args)...);
    }

    operator const char *()const{
        return m_Buffer;
    }
};

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

    bool Execute(const Stmt &stmt){
        char *message = nullptr;
        if(sqlite3_exec(m_Handle, stmt, nullptr, nullptr, &message) != SQLITE_OK){
            Println("SQLite3: %", message);
            sqlite3_free(message);
            return false;
        }

        return true;
    }
    
    QueryResult Query(const Stmt &stmt){
        sqlite3_stmt *sqlite_stmt = nullptr;
        sqlite3_prepare_v2(m_Handle, stmt, -1, &sqlite_stmt, nullptr);

        return {sqlite_stmt};
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

struct AddDrinkView{
    static constexpr size_t BufferSize = 1024;

    InputBuffer<BufferSize> DrinkName;
    float PricePerLiter;
    int AgeRestriction;

    bool Draw(){

        ImGui::InputText("Name", DrinkName.Data(), DrinkName.Size());
        ImGui::InputFloat("PricePerLiter", &PricePerLiter);
        ImGui::InputInt("AgeRestriction", &AgeRestriction);

        bool btn = ImGui::Button("Add");

        return btn;
    }
};

class DrinksListPanel{
private:
    Database &m_Database;
    int m_LastID = 0;
    AddDrinkView m_AddDrinkPanel;
public:
    DrinksListPanel(Database &db):
        m_Database(db) 
    {}

    void Draw(){
        ImGui::Begin("Drinks");

        if(m_AddDrinkPanel.Draw())
            InsertDrinkAndUpdateQuery();

        ImGui::Separator();

        if(ImGui::Button("Clear"))
            m_Database.Execute(Stmt("DELETE FROM Drinks"));
        
        ImGui::Separator();

        QueryResult query = m_Database.Query(Stmt("SELECT * FROM Drinks"));

        if(ImGui::BeginTable("Drinks", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)){
            for( ;query; query.Next()){
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", query.GetColumnString(1)); 
                ImGui::TableNextColumn();
                ImGui::Text("%f", query.GetColumnFloat(2)); 
                ImGui::TableNextColumn();
                ImGui::Text("%d", query.GetColumnInt(3)); 
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }
private:

    void InsertDrinkAndUpdateQuery(){
        m_Database.Execute(
            Stmt(
                "INSERT INTO Drinks(ID, Name, PricePerLiter, AgeRestriction) VALUES(%,'%',%,%)",
                ++m_LastID,
                m_AddDrinkPanel.DrinkName.Data(),
                m_AddDrinkPanel.PricePerLiter,
                m_AddDrinkPanel.AgeRestriction
            )
        );
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
    DrinksListPanel m_DrinksList{m_DB};
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
        m_DrinksList.Draw();
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