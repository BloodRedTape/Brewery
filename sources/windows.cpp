#include <imgui/backend.hpp>
#include <core/string.hpp>
#include <sstream>
#include <iomanip>
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

class NewWaiterPopup{
    static constexpr size_t BufferSize = 1024;
private:
    WaitersTableMediator m_WaitersTable;
    InputBuffer<BufferSize> m_WaiterName;
    float m_Salary = 0.f;
    int m_FullAge = 0;

    int m_LastID{(int)m_WaitersTable.Size()};

    const char *const m_Name = "New Waiter";
public:
    NewWaiterPopup(Database &db):
            m_WaitersTable(db)
    {}

    void Open(){
        ImGui::OpenPopup(m_Name);
    }

    void Draw(){
        if(ImGui::BeginPopup(m_Name)) {
            ImGui::InputText("Name", m_WaiterName.Data(), m_WaiterName.Size());
            ImGui::InputFloat("Salary", &m_Salary);
            ImGui::InputInt("FullAge", &m_FullAge);

            if (ImGui::Button("Add")) {
                m_WaitersTable.Add(
                        ++m_LastID,
                        m_WaiterName.Data(),
                        m_Salary,
                        m_FullAge
                );

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }else{
            m_WaiterName.Clear();
            m_Salary = 0.f;
            m_FullAge = 0;
        }
    }
};

class WaitersListPanel{
private:
    WaitersTableMediator m_WaitersTable;
    NewWaiterPopup m_NewWaiterPopup;
public:
    WaitersListPanel(Database &db):
            m_WaitersTable(db),
            m_NewWaiterPopup(db)
    {}

    void Draw(){

        ImGui::Begin("Waiters");

        if(ImGui::Button("Clear"))
            m_WaitersTable.Clear();

        ImGui::SameLine();

        if(ImGui::Button("New Waiter"))
            m_NewWaiterPopup.Open();
        m_NewWaiterPopup.Draw();

        ImGui::Separator();

        ImGui::BeginChild("##List");

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
        ImGui::EndChild();
        ImGui::End();
    }
};

class NewDrinkPopup{
    static constexpr size_t BufferSize = 1024;
private:
    DrinksTableMediator m_DrinksTable;
    InputBuffer<BufferSize> m_DrinkName;
    float m_PricePerLiter = 0.f;
    int m_AgeRestriction = 0;

    int m_LastID{(int)m_DrinksTable.Size()};

    const char *const m_Name = "New Drink";
public:
    NewDrinkPopup(Database &db):
            m_DrinksTable(db)
    {}

    void Open(){
        ImGui::OpenPopup(m_Name);
    }

    void Draw(){

        if(ImGui::BeginPopup(m_Name)) {
            ImGui::InputText("Name", m_DrinkName.Data(), m_DrinkName.Size());
            ImGui::InputFloat("PricePerLiter", &m_PricePerLiter);
            ImGui::InputInt("AgeRestriction", &m_AgeRestriction);

            if (ImGui::Button("Add")) {
                m_DrinksTable.Add(
                        ++m_LastID,
                        m_DrinkName.Data(),
                        m_PricePerLiter,
                        m_AgeRestriction
                );
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::SameLine();
            
            if(ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }else{
            m_DrinkName.Clear();
            m_PricePerLiter = 0.f;
            m_AgeRestriction = 0;
        }
    }
};

class DrinksListPanel{
private:
    DrinksTableMediator m_DrinksTable;
    NewDrinkPopup m_NewDrinkPopup;
public:
    DrinksListPanel(Database &db):
            m_DrinksTable(db),
            m_NewDrinkPopup(db)
    {}

    void Draw(){

        ImGui::Begin("Drinks");

        if(ImGui::Button("Clear"))
            m_DrinksTable.Clear();

        ImGui::SameLine();

        if(ImGui::Button("New Drink"))
            m_NewDrinkPopup.Open();
        m_NewDrinkPopup.Draw();

        ImGui::Separator();

        ImGui::BeginChild("##List");

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

                if(ImGui::IsItemHovered())
                    ImGui::SetTooltip(query.GetColumnString(1));

                ImGui::TableNextColumn();
                ImGui::Text("%f", query.GetColumnFloat(2));
                ImGui::TableNextColumn();
                ImGui::Text("%d", query.GetColumnInt(3));

            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
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

class NewOrderPopup{
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

    const char *const m_Name = "New Order";
public:
    NewOrderPopup(Database &db):
            m_DrinksTable(db),
            m_GobletsTable(db),
            m_DrinksOrdersTable(db),
            m_OrdersLogTable(db),
            m_WaitersTable(db)
    {}

    void Open(){
        ImGui::OpenPopup(m_Name);
    }

    void Draw(){

        if(ImGui::BeginPopup(m_Name)) {
            ImGui::InputText("CustomerName", m_CustomerName.Data(), m_CustomerName.Size());
            ImGui::InputText("WaiterName", m_WaiterName.Data(), m_WaiterName.Size());
            ImGui::InputFloat("Tips", &m_Tips);

            auto selected_drink_query = m_DrinksTable.Query(m_CurrentDrinkID);

            ImGui::PushItemWidth(ImGui::GetWindowSize().x / 3);

            if (ImGui::BeginCombo("##DrinksCombo",
                                  selected_drink_query ? selected_drink_query.GetColumnString(1) : "None")) {
                auto drink_query = m_DrinksTable.Query();
                for (; drink_query; drink_query.Next()) {
                    int id = drink_query.GetColumnInt(0);
                    const char *name = drink_query.GetColumnString(1);

                    if (ImGui::Selectable(name))
                        m_CurrentDrinkID = id;
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();

            if (ImGui::BeginCombo("##GobletsCombo", GetGobletName(m_GobletsTable.Query(m_CurrentGobletID)).c_str())) {
                auto goblet_query = m_GobletsTable.Query();
                for (; goblet_query; goblet_query.Next()) {
                    int id = goblet_query.GetColumnInt(0);
                    std::string name = GetGobletName(goblet_query);

                    if (ImGui::Selectable(name.c_str()))
                        m_CurrentGobletID = id;
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();

            if (ImGui::Button("Add"))
                if (m_CurrentGobletID != -1 && m_CurrentDrinkID != -1)
                    m_Drinks.Add({m_CurrentDrinkID, m_CurrentGobletID});

            ImGui::PopItemWidth();

            if (m_Drinks.Size()
                && ImGui::BeginTable("Drinks", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                for (const auto &drink: m_Drinks) {
                    auto drink_query = m_DrinksTable.Query(drink.DrinkID);
                    auto goblet_query = m_GobletsTable.Query(drink.GobletID);

                    if (!drink_query || !goblet_query)continue;

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

            if (ImGui::Button("Place Order") && IsDataValid() && m_WaitersTable.Exists(m_WaiterName.Data())) {
                m_LastOrderLogID++;

                QueryResult waiter = m_WaitersTable.Query(m_WaiterName.Data());

                m_OrdersLogTable.Add(m_LastOrderLogID, m_CustomerName.Data(), m_Tips, waiter.GetColumnInt(0));

                for (auto drink: m_Drinks) {
                    m_DrinksOrdersTable.Add(m_LastOrderLogID, drink.DrinkID, drink.GobletID);
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if(ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }else{
            m_CustomerName.Clear();
            m_WaiterName.Clear();
            m_Drinks.Clear();
        }
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
    NewOrderPopup m_NewOrderPopup;
public:
    OrdersLogPanel(Database &db):
            m_DrinkOrders(db),
            m_OrdersLog(db),
            m_DrinksTable(db),
            m_GobletsTable(db),
            m_NewOrderPopup(db),
            m_WaitersTable(db)
    {}

    void Draw(){

        ImGui::Begin("Orders Log");

        if(ImGui::Button("Clear")){
            m_DrinkOrders.Clear();
            m_OrdersLog.Clear();
        }

        ImGui::SameLine();

        if(ImGui::Button("New Order"))
            m_NewOrderPopup.Open();
        m_NewOrderPopup.Draw();

        ImGui::Separator();

        ImGui::BeginChild("##List");

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

        ImGui::EndChild();

        ImGui::End();


    }
};


class ConsoleWindow{
private:
    DatabaseLogger &m_Logger;
    InputBuffer<1024> m_CurrentLine;
    Database &m_Database;
public:
    ConsoleWindow(DatabaseLogger &logger, Database &db):
            m_Logger(logger),
            m_Database(db)
    {}

    void Draw(){

        const auto &lines = m_Logger.Lines();
        ImGui::Begin("Console");

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("##Text", ImVec2(0, -footer_height_to_reserve));

        for(const auto &line: lines)
            ImGui::TextWrapped("%s", line.c_str());

        ImGui::EndChild();

        ImGui::Separator();

        ImGui::PushItemWidth(ImGui::GetWindowWidth());

        if(ImGui::InputText("##Input", m_CurrentLine.Data(), m_CurrentLine.Size(), ImGuiInputTextFlags_EnterReturnsTrue)){
            m_Logger.Log("[User]: %", m_CurrentLine.Data());

            m_Database.Execute(m_CurrentLine.Data(), [](void *usr, int count, char **data, char **name)->int{
                ConsoleWindow *self = (ConsoleWindow*)usr;

                std::stringstream string;
                for(int i = 0; ; i++) {
                    string << data[i];

                    if(i == count - 1)break;

                    string << std::setw(20);
                }

                self->m_Logger.Log(string.str().c_str());

                return 0;
            }, this);

            m_CurrentLine.Clear();


            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
        }
        ImGui::PopItemWidth();

        ImGui::End();
    }
};