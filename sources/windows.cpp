#include <imgui/backend.hpp>
#include "helpers.cpp"
#include "mediators.cpp"

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

    int m_LastID{(int)m_WaitersTable.Size()};
public:
    NewWaiterWindow(Database &db):
            m_WaitersTable(db)
    {}

    void Draw(bool *is_open){
        if(!*is_open)return Clear();

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

    void Clear(){
        m_WaiterName.Clear();
        m_Salary = 0;
        m_FullAge = 0;
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
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Salary");
            ImGui::TableSetupColumn("Age");
            ImGui::TableHeadersRow();

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

    int m_LastID{(int)m_DrinksTable.Size()};
public:
    NewDrinkWindow(Database &db):
            m_DrinksTable(db)
    {}

    void Draw(bool *is_open){
        if(!*is_open)return Clear();

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

    void Clear(){
        m_AgeRestriction = 0;
        m_PricePerLiter = 0;
        m_DrinkName.Clear();
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
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Price");
            ImGui::TableSetupColumn("AgeRestr");
            ImGui::TableHeadersRow();
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
    WaitersTableMediator m_WaitersTable;

    InputBuffer<BufferSize> m_CustomerName;
    InputBuffer<BufferSize> m_WaiterName;
    float m_Tips = 0.f;

    List<DrinkOrder> m_Drinks;

    int m_CurrentDrinkID = -1;
    int m_CurrentGobletID = -1;

    int m_LastOrderLogID{(int)m_OrdersLogTable.Size()};
public:
    NewOrderWindow(Database &db):
            m_DrinksTable(db),
            m_GobletsTable(db),
            m_DrinksOrdersTable(db),
            m_OrdersLogTable(db),
            m_WaitersTable(db)
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

        if(ImGui::Button("Place Order") && IsDataValid() && m_WaitersTable.Exists(m_WaiterName.Data())){
            m_LastOrderLogID++;

            QueryResult waiter = m_WaitersTable.Query(m_WaiterName.Data());

            m_OrdersLogTable.Add(m_LastOrderLogID, m_CustomerName.Data(), m_Tips, waiter.GetColumnInt(0));

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
    WaitersTableMediator m_WaitersTable;
    bool m_IsNewOrderOpen = false;
    NewOrderWindow m_NewOrderView;
public:
    OrdersLogPanel(Database &db):
            m_DrinkOrders(db),
            m_OrdersLog(db),
            m_DrinksTable(db),
            m_GobletsTable(db),
            m_NewOrderView(db),
            m_WaitersTable(db)
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
            ImGui::Text("Waiter: %s", m_WaitersTable.Query(waiter_id).GetColumnString(1));
            ImGui::Text("Tips: %f", tips);

            if(ImGui::BeginTable("##Drinks_", 3, ImGuiTableFlags_RowBg)){

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