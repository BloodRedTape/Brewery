#include "core/print.hpp"
#include <core/span.hpp>
#include <core/os/clock.hpp>
#include <core/os/window.hpp>
#include <core/os/sleep.hpp>
#include <core/os/stacktrace.hpp>
#include <core/algorithm.hpp>
#include <graphics/api/swapchain.hpp>
#include <imgui/backend.hpp>
#include <sqlite3.h>
#include <tuple>
#include <core/list.hpp>
#include <string>

template<size_t SizeValue>
class InputBuffer{
private:
    char m_Data[SizeValue];
public:
    InputBuffer(){
        Clear();
    }

    size_t Size()const{
        return SizeValue;
    }

    size_t Length()const{
        return strlen(m_Data);
    }

    char *Data(){
        return m_Data;
    }

    void Clear(){
        m_Data[0] = 0;
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

class DatabaseLogger{
private:
    List<std::string> m_Lines;
public:

    template<typename ...ArgsType>
    void Log(const char *fmt, ArgsType&&...args){
        std::string buffer;

        const auto writer = [](char ch, void *data){
            std::string &buffer = *(std::string*)data;
            buffer.push_back(ch);
        };

        WriterPrint(writer, &buffer, fmt, Forward<ArgsType>(args)...);

        m_Lines.Add(Move(buffer));
    }

    const List<std::string> &Lines()const{
        return m_Lines;
    }
};

class Database{
private:
    sqlite3 *m_Handle = nullptr;
    DatabaseLogger &m_Logger;

    using CallbackType = Function<void(int, char**, char**)>;
public:

    Database(const char *filepath, DatabaseLogger &logger):
        m_Logger(logger)
    {
        sqlite3_open(filepath, &m_Handle);
    }

    ~Database(){
        sqlite3_close(m_Handle);
    }

    bool Execute(const Stmt &stmt){
        char *message = nullptr;
        if(sqlite3_exec(m_Handle, stmt, nullptr, nullptr, &message) != SQLITE_OK){
            m_Logger.Log("[SQLite]: %", message);
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

class DrinksTableMediator{
private:
    Database &m_Database;
public:
    DrinksTableMediator(Database &db):
        m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM Drinks"));
    }

    QueryResult Query(int id){
        return m_Database.Query(Stmt("SELECT * FROM Drinks WHERE ID = %", id));
    }

    QueryResult Query(const char *name){
        return m_Database.Query(Stmt("SELECT * FROM Drinks WHERE Name = %", name));
    }

    void Clear(){
        m_Database.Execute(Stmt("DELETE FROM Drinks"));
    }

    void Add(int id, const char *name, float price_per_liter, int age_restriction){
        m_Database.Execute(
            Stmt(
                "INSERT INTO Drinks(ID, Name, PricePerLiter, AgeRestriction) VALUES(%,'%',%,%)",
                id,
                name,
                price_per_liter,
                age_restriction 
            )
        );
    }
};

class OrdersLogTableMediator{
private:
    Database &m_Database;
public:
    OrdersLogTableMediator(Database &db):
        m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM OrdersLog"));
    }

    void Clear(){
        m_Database.Execute(Stmt("DELETE FROM OrdersLog"));
    }

    void Add(int id, const char *customer_name, float tips, int waiter_id){
        m_Database.Execute(
            Stmt(
                "INSERT INTO OrdersLog(ID, CustomerShortName, Tips, WaiterID) VALUES(%,'%',%,%)",
                id,
                customer_name,
                tips,
                waiter_id 
            )
        );
    }
};

class DrinkOrdersTableMediator{
private:
    Database &m_Database;
public:
    DrinkOrdersTableMediator(Database &db):
        m_Database(db)
    {}

    QueryResult Query(int order_id){
        return m_Database.Query(Stmt("SELECT * FROM DrinkOrders WHERE OrderID = %", order_id));
    }

    void Clear(){
        m_Database.Execute(Stmt("DELETE FROM DrinkOrders"));
    }

    void Add(int order_id, int drink_id, int goblet_id){
        m_Database.Execute(
            Stmt(
                "INSERT INTO DrinkOrders(OrderID, DrinkID, GobletID) VALUES(%, %, %)",
                order_id,
                drink_id,
                goblet_id 
            )
        );
    }
};

class GobletsTableMediator{
private:
    Database &m_Database;
public:
    GobletsTableMediator(Database &db):
        m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM Goblets"));
    }

    QueryResult Query(int id){
        return m_Database.Query(Stmt("SELECT * FROM Goblets WHERE ID = %", id));
    }

    QueryResult Query(const char *name){
        return m_Database.Query(Stmt("SELECT * FROM Goblets WHERE Name = %", name));
    }

    void Clear(){
        m_Database.Execute(Stmt("DELETE FROM Goblets"));
    }
};

class WaitersTableMediator{
private:
    Database &m_Database;
public:
    WaitersTableMediator(Database &db):
            m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM Waiters"));
    }

    void Add(int id, const char *name, float salary, int age){
        m_Database.Execute(
                Stmt(
                        "INSERT INTO Waiters(ID, ShortName, Salary, FullAge) VALUES(%, '%', %, %)",
                        id,
                        name,
                        salary,
                        age
                )
        );
    }

    void Clear(){
        m_Database.Execute(Stmt("DELETE FROM Waiters"));
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

class NewWaiterWindow{
    static constexpr size_t BufferSize = 1024;
private:
    WaitersTableMediator m_WaitersTable;
    InputBuffer<BufferSize> m_WaiterName;
    float m_Salary = 0.f;
    int m_FullAge = 0;

    int m_LastID = 0;
public:
    NewWaiterWindow(Database &db):
        m_WaitersTable(db)
    {}

    void Draw(bool *is_open){
        if(!*is_open)return;

        ImGui::Begin("New Waiter", is_open);
        ImGui::InputText("Name", m_WaiterName.Data(), m_WaiterName.Size());
        ImGui::InputFloat("Salary", &m_Salary);
        ImGui::InputInt("FullAge", &m_FullAge);

        if(ImGui::Button("Add")){
            m_WaitersTable.Add(
                    ++m_LastID,
                    m_WaiterName.Data(),
                    m_Salary,
                    m_FullAge
            );

            *is_open = false;
        }

        ImGui::End();
    }
};

class WaitersListPanel{
private:
    WaitersTableMediator m_WaitersTable;
    NewWaiterWindow m_NewWaiterWindow;
    bool m_IsNewWaiterOpen = false;
public:
    WaitersListPanel(Database &db):
            m_WaitersTable(db),
            m_NewWaiterWindow(db)
    {}

    void Draw(){
        m_NewWaiterWindow.Draw(&m_IsNewWaiterOpen);

        ImGui::Begin("Waiters");

        if(ImGui::Button("Clear"))
            m_WaitersTable.Clear();



        if(!m_IsNewWaiterOpen && (ImGui::SameLine(), ImGui::Button("New Waiter")))
            m_IsNewWaiterOpen = true;

        ImGui::Separator();

        QueryResult query = m_WaitersTable.Query();

        if(ImGui::BeginTable("Waiters", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)){
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
};

class NewDrinkWindow{
    static constexpr size_t BufferSize = 1024;
private:
    DrinksTableMediator m_DrinksTable;
    InputBuffer<BufferSize> m_DrinkName;
    float m_PricePerLiter = 0.f;
    int m_AgeRestriction = 0;

    int m_LastID = 0;
public:
    NewDrinkWindow(Database &db):
        m_DrinksTable(db)
    {}

    void Draw(bool *is_open){
        if(!*is_open)return;

        ImGui::Begin("New Drink", is_open);
        ImGui::InputText("Name", m_DrinkName.Data(), m_DrinkName.Size());
        ImGui::InputFloat("PricePerLiter", &m_PricePerLiter);
        ImGui::InputInt("AgeRestriction", &m_AgeRestriction);

        if(ImGui::Button("Add")){
            m_DrinksTable.Add(
                ++m_LastID,
                m_DrinkName.Data(),
                m_PricePerLiter,
                m_AgeRestriction
            );

            *is_open = false;
        }

        ImGui::End();
    }
};

class DrinksListPanel{
private:
    DrinksTableMediator m_DrinksTable;
    NewDrinkWindow m_NewDrinkWindow;
    bool m_IsNewDrinkOpen = false;
public:
    DrinksListPanel(Database &db):
        m_DrinksTable(db),
        m_NewDrinkWindow(db)
    {}

    void Draw(){
        m_NewDrinkWindow.Draw(&m_IsNewDrinkOpen);

        ImGui::Begin("Drinks");

        if(ImGui::Button("Clear"))
            m_DrinksTable.Clear();



        if(!m_IsNewDrinkOpen && (ImGui::SameLine(), ImGui::Button("New Drink")))
            m_IsNewDrinkOpen = true;

        ImGui::Separator();

        QueryResult query = m_DrinksTable.Query();

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
};

struct DrinkOrder{
    int DrinkID;
    int GobletID;
};

std::string GetGobletName(const QueryResult &query){
    if(!query)return "None";

    return query.GetColumnString(1) + std::string(" ") + std::to_string(query.GetColumnFloat(2)) + "l";
}

class NewOrderWindow{
    static constexpr size_t BufferSize = 1024;
private:
    DrinksTableMediator m_DrinksTable;
    GobletsTableMediator m_GobletsTable;
    DrinkOrdersTableMediator m_DrinksOrdersTable;
    OrdersLogTableMediator m_OrdersLogTable;

    InputBuffer<BufferSize> m_CustomerName;
    InputBuffer<BufferSize> m_WaiterName;
    float m_Tips = 0.f;

    List<DrinkOrder> m_Drinks;

    int m_CurrentDrinkID = -1;
    int m_CurrentGobletID = -1;

    int m_LastDrinksOrderID = 0;
    int m_LastOrderLogID = 0;
public:
    NewOrderWindow(Database &db):
        m_DrinksTable(db),
        m_GobletsTable(db),
        m_DrinksOrdersTable(db),
        m_OrdersLogTable(db)
    {}

    void Draw(bool *is_open){
        if(!*is_open)return;

        ImGui::Begin("New Order Window", is_open);
        ImGui::InputText("CustomerName", m_CustomerName.Data(), m_CustomerName.Size());
        ImGui::InputText("WaiterName", m_WaiterName.Data(), m_WaiterName.Size());
        ImGui::InputFloat("Tips", &m_Tips);

        auto selected_drink_query = m_DrinksTable.Query(m_CurrentDrinkID);

        ImGui::PushItemWidth(ImGui::GetWindowSize().x / 3);

        if(ImGui::BeginCombo("##DrinksCombo", selected_drink_query ? selected_drink_query.GetColumnString(1) : "None")){
            auto drink_query = m_DrinksTable.Query();
            for(;drink_query; drink_query.Next()){
                int id = drink_query.GetColumnInt(0);
                const char *name = drink_query.GetColumnString(1);

                if(ImGui::Selectable(name))
                    m_CurrentDrinkID = id;
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        
        if(ImGui::BeginCombo("##GobletsCombo", GetGobletName(m_GobletsTable.Query(m_CurrentGobletID)).c_str())){
            auto goblet_query = m_GobletsTable.Query();
            for(;goblet_query; goblet_query.Next()){
                int id = goblet_query.GetColumnInt(0);
                std::string name = GetGobletName(goblet_query);

                if(ImGui::Selectable(name.c_str()))
                    m_CurrentGobletID = id;
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();

        if(ImGui::Button("Add"))
            if(m_CurrentGobletID != -1 && m_CurrentDrinkID != -1)
                m_Drinks.Add({m_CurrentDrinkID, m_CurrentGobletID});

        ImGui::PopItemWidth();

        if(m_Drinks.Size()
        && ImGui::BeginTable("Drinks", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)){
            for(const auto &drink: m_Drinks){
                auto drink_query = m_DrinksTable.Query(drink.DrinkID);
                auto goblet_query = m_GobletsTable.Query(drink.GobletID);

                if(!drink_query || !goblet_query)continue;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", drink_query.GetColumnString(1));
                ImGui::TableNextColumn();
                ImGui::Text("%s", goblet_query.GetColumnString(1));
                ImGui::TableNextColumn();
                ImGui::Text("%.1f", goblet_query.GetColumnFloat(2));
            }
            ImGui::EndTable();
        }

        if(ImGui::Button("Place Order") && IsDataValid()){
            m_LastOrderLogID++;

            m_OrdersLogTable.Add(m_LastOrderLogID, m_CustomerName.Data(), m_Tips, 1);
            
            for(auto drink: m_Drinks){
                m_DrinksOrdersTable.Add(m_LastOrderLogID, drink.DrinkID, drink.GobletID);
            }

            *is_open = false;

            m_CustomerName.Clear();
            m_WaiterName.Clear();
            m_Drinks.Clear();
        }
        ImGui::End();
    }
private:
    bool IsDataValid(){
        return m_Drinks.Size() && m_CustomerName.Length() && m_WaiterName.Length();
    }
};

class OrdersLogPanel{
private:
    DrinkOrdersTableMediator m_DrinkOrders;
    OrdersLogTableMediator m_OrdersLog;
    DrinksTableMediator m_DrinksTable;
    GobletsTableMediator m_GobletsTable;
    bool m_IsNewOrderOpen = false;
    NewOrderWindow m_NewOrderView;
public:
    OrdersLogPanel(Database &db):
        m_DrinkOrders(db),
        m_OrdersLog(db),
        m_DrinksTable(db),
        m_GobletsTable(db),
        m_NewOrderView(db)
    {}

    void Draw(){
        m_NewOrderView.Draw(&m_IsNewOrderOpen);

        ImGui::Begin("Orders Log");

        if(ImGui::Button("Clear")){
            m_DrinkOrders.Clear();
            m_OrdersLog.Clear();
        }

        

        if(!m_IsNewOrderOpen && (ImGui::SameLine(), ImGui::Button("New Order")))
            m_IsNewOrderOpen = true;

        ImGui::Separator();

        QueryResult orders = m_OrdersLog.Query();

        for( ;orders; orders.Next()){
            int order_id = orders.GetColumnInt(0);
            const char *customer_name = orders.GetColumnString(1);
            float tips = orders.GetColumnFloat(2);
            int waiter_id = orders.GetColumnInt(3);
            ImGui::Text("CustomerName: %s", customer_name);
            ImGui::Text("Waiter...");
            ImGui::Text("Tips: %f", tips);

            if(ImGui::BeginTable("##Drinks_", 3)){
                
                QueryResult order_drinks = m_DrinkOrders.Query(order_id);

                for(; order_drinks; order_drinks.Next()){
                    QueryResult drink = m_DrinksTable.Query(order_drinks.GetColumnInt(1));
                    QueryResult goblet = m_GobletsTable.Query(order_drinks.GetColumnInt(2));

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", drink.GetColumnString(1));
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", goblet.GetColumnString(1));
                    ImGui::TableNextColumn();
                    ImGui::Text("%f", goblet.GetColumnFloat(2));
                }

                ImGui::EndTable();
            }

            ImGui::Separator();
        }

        ImGui::End();
        

    }
};


class LogWindow{
private:
    DatabaseLogger &m_Logger;
public:
    LogWindow(DatabaseLogger &logger):
        m_Logger(logger)
    {}

    void Draw(){

        const auto &lines = m_Logger.Lines();
        ImGui::Begin("Log");

        for(const auto &line: lines)
            ImGui::Text("%s", line.c_str());

        ImGui::End();
    }
};

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

    LogWindow m_LogWindow{m_Logger};
    DrinksListPanel m_DrinksList{m_DB};
    OrdersLogPanel m_OrdersLog{m_DB};
    WaitersListPanel m_WaitersList{m_DB};

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
        m_LogWindow.Draw();
        m_DrinksList.Draw();
        m_OrdersLog.Draw();
        m_WaitersList.Draw();
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