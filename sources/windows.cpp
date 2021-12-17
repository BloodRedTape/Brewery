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

class NewIngredientPopup{
    static constexpr size_t BufferSize = 1024;
private:
    IngredientsTableMediator m_IngredientsTable;
    SourcesTableMediator m_SourcesTable;
    InputBuffer<BufferSize> m_IngredientName;
    InputBuffer<BufferSize> m_UnitsName;

    int m_SourceID = 0;

    int m_LastID{(int)m_IngredientsTable.Size()};

    const char *const m_Name = "New Ingredient";
public:
    NewIngredientPopup(Database &db):
            m_IngredientsTable(db),
            m_SourcesTable(db)
    {}

    void Open(){
        ImGui::OpenPopup(m_Name);
    }

    void Draw(){

        if(ImGui::BeginPopup(m_Name)) {
            ImGui::InputText("Name", m_IngredientName.Data(), m_IngredientName.Size());
            ImGui::InputText("Units", m_UnitsName.Data(), m_UnitsName.Size());

            auto current_source = m_SourcesTable.Query(m_SourceID);

            if (ImGui::BeginCombo("##IngredientsCombo",
                                  current_source ? current_source.GetColumnString(1) : "None")) {
                auto sources_query = m_SourcesTable.Query();
                for (; sources_query; sources_query.Next()) {
                    int id = sources_query.GetColumnInt(0);
                    const char *name = sources_query.GetColumnString(1);

                    if (ImGui::Selectable(name))
                        m_SourceID = id;
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Add") && IsDataValid()) {
                m_IngredientsTable.Add(
                        ++m_LastID,
                        m_IngredientName.Data(),
                        m_UnitsName.Data(),
                        m_SourceID
                );
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if(ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }else{
            m_IngredientName.Clear();
            m_UnitsName.Clear();
            m_SourceID = 0;
        }
    }

private:
    bool IsDataValid()const{
        return m_SourceID && m_IngredientName.Length() && m_UnitsName.Length();
    }
};

class IngredientsListPanel{
private:
    IngredientsTableMediator m_IngredientsTable;
    SourcesTableMediator m_SourcesTable;
    NewIngredientPopup m_NewIngredientPopup;
public:
    IngredientsListPanel(Database &db):
            m_NewIngredientPopup(db),
            m_IngredientsTable(db),
            m_SourcesTable(db)
    {}

    void Draw(){

        ImGui::Begin("Ingredients");

        if(ImGui::Button("Clear"))
            m_IngredientsTable.Clear();

        ImGui::SameLine();

        if(ImGui::Button("New Ingredient"))
            m_NewIngredientPopup.Open();
        m_NewIngredientPopup.Draw();

        ImGui::Separator();

        ImGui::BeginChild("##List");

        QueryResult query = m_IngredientsTable.Query();

        if(ImGui::BeginTable("Ingredients", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)){
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Units");
            ImGui::TableSetupColumn("Source");
            ImGui::TableHeadersRow();
            for( ;query; query.Next()){
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", query.GetColumnString(1));
                ImGui::TableNextColumn();
                ImGui::Text("%s", query.GetColumnString(2));
                ImGui::TableNextColumn();
                auto source = m_SourcesTable.Query(query.GetColumnInt(3));
                ImGui::Text("%s", source.GetColumnString(1));

            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
        ImGui::End();

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

class NewSourcePopup{
    static constexpr size_t BufferSize = 1024;
private:
    SourcesTableMediator m_SourcesTable;
    AddressesTableMediator m_AddressesTable;
    InputBuffer<BufferSize> m_SourceName;
    InputBuffer<BufferSize> m_City;
    InputBuffer<BufferSize> m_House;
    int m_PostalCode = 0;

    int m_LastSourceID{(int)m_SourcesTable.Size()};
    int m_LastAddressID{(int)m_AddressesTable.Size()};

    const char *const m_Name = "New Source";

public:
    NewSourcePopup(Database &db):
            m_SourcesTable(db),
            m_AddressesTable(db)
    {}

    void Open(){
        ImGui::OpenPopup(m_Name);
    }

    void Draw(){
        if(ImGui::BeginPopup(m_Name)) {
            ImGui::InputText("Source Name", m_SourceName.Data(), m_SourceName.Size());
            ImGui::InputText("City", m_City.Data(), m_City.Size());
            ImGui::InputText("House", m_House.Data(), m_House.Size());
            ImGui::InputInt("PostalCode", &m_PostalCode);

            if (ImGui::Button("Add") && IsDataValid()) {
                m_AddressesTable.TryAdd(
                        ++m_LastAddressID,
                        m_City.Data(),
                        m_House.Data(),
                        m_PostalCode
                );

                auto address = m_AddressesTable.Query(m_City.Data(), m_House.Data(), m_PostalCode);

                m_SourcesTable.Add(++m_LastSourceID, m_SourceName.Data(), address.GetColumnInt(0));

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }else{
            m_House.Clear();
            m_City.Clear();
            m_SourceName.Clear();
            m_PostalCode = 0;
        }
    }
private:
    bool IsDataValid()const{
        return m_SourceName.Length() && m_House.Length() && m_PostalCode && m_City.Length();
    }
};

class SourcesListPanel{
private:
    SourcesTableMediator m_SourcesTable;
    AddressesTableMediator m_AddressesTable;
    NewSourcePopup m_NewSourcePopup;
public:
    SourcesListPanel(Database &db):
            m_SourcesTable(db),
            m_AddressesTable(db),
            m_NewSourcePopup(db)
    {}

    void Draw(){

        ImGui::Begin("Sources");

        if(ImGui::Button("Clear"))
            m_SourcesTable.Clear();

        ImGui::SameLine();

        if(ImGui::Button("New Source"))
            m_NewSourcePopup.Open();
        m_NewSourcePopup.Draw();

        ImGui::Separator();

        ImGui::BeginChild("##List");


        if(ImGui::BeginTable("Sources", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)){
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("City");
            ImGui::TableSetupColumn("House");
            ImGui::TableSetupColumn("PostalCode");
            ImGui::TableHeadersRow();

            QueryResult query = m_SourcesTable.Query();
            for( ;query; query.Next()){

                auto address = m_AddressesTable.Query(query.GetColumnInt(2));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", query.GetColumnString(1));
                ImGui::TableNextColumn();
                ImGui::Text("%s", address.GetColumnString(1));
                ImGui::TableNextColumn();
                ImGui::Text("%s", query.GetColumnString(2));
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

    struct IngredientAddInfo{
        int ID = 0;
        float Amount = 0.f;
    };
private:
    DrinksTableMediator m_DrinksTable;
    IngredientsTableMediator m_IngredientsTable;
    IngredientsDrinksTableMediator m_IngredientsDrinksTable;
    InputBuffer<BufferSize> m_DrinkName;
    float m_PricePerLiter = 0.f;
    int m_AgeRestriction = 0;

    int m_LastID{(int)m_DrinksTable.Size()};

    IngredientAddInfo m_CurrentIngredient;

    List<IngredientAddInfo> m_Ingredients;

    const char *const m_Name = "New Drink";
public:
    NewDrinkPopup(Database &db):
            m_DrinksTable(db),
            m_IngredientsTable(db),
            m_IngredientsDrinksTable(db)
    {}

    void Open(){
        ImGui::OpenPopup(m_Name);
    }

    void Draw(){

        if(ImGui::BeginPopup(m_Name)) {
            ImGui::InputText("Name", m_DrinkName.Data(), m_DrinkName.Size());
            ImGui::InputFloat("PricePerLiter", &m_PricePerLiter);
            ImGui::InputInt("AgeRestriction", &m_AgeRestriction);

            auto current_ingredient = m_IngredientsTable.Query(m_CurrentIngredient.ID);

            if (ImGui::BeginCombo("##IngredientsCombo",
                                  current_ingredient ? current_ingredient.GetColumnString(1) : "None")) {
                auto ingredients_query = m_IngredientsTable.Query();
                for (; ingredients_query; ingredients_query.Next()) {
                    int id = ingredients_query.GetColumnInt(0);
                    const char *name = ingredients_query.GetColumnString(1);

                    if (ImGui::Selectable(name))
                        m_CurrentIngredient.ID = id;
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();

            ImGui::InputFloat("##Amount", &m_CurrentIngredient.Amount);

            ImGui::SameLine();

            if(ImGui::Button("Append") && m_CurrentIngredient.ID && m_CurrentIngredient.Amount > 0.f){
                m_Ingredients.Add(m_CurrentIngredient);
                m_CurrentIngredient = {};
            }

            if(ImGui::BeginTable("##Ingredients", 2)){
                for(auto info: m_Ingredients){
                    auto ingredient = m_IngredientsTable.Query(info.ID);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", ingredient.GetColumnString(1));
                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f", info.Amount);
                }

                ImGui::EndTable();
            }

            if (ImGui::Button("Add")
            && m_Ingredients.Size()) {
                m_DrinksTable.Add(
                        ++m_LastID,
                        m_DrinkName.Data(),
                        m_PricePerLiter,
                        m_AgeRestriction
                );
                for(auto info: m_Ingredients)
                    m_IngredientsDrinksTable.Add(info.ID, info.Amount, m_LastID);
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::SameLine();
            
            if(ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }else{
            m_Ingredients.Clear();
            m_CurrentIngredient = {};
            m_DrinkName.Clear();
            m_PricePerLiter = 0.f;
            m_AgeRestriction = 0;
        }
    }
};

class DrinksListPanel{
private:
    DrinksTableMediator m_DrinksTable;
    IngredientsTableMediator m_IngredientsTable;
    IngredientsDrinksTableMediator m_IngredientsDrinksTable;
    NewDrinkPopup m_NewDrinkPopup;
public:
    DrinksListPanel(Database &db):
        m_DrinksTable(db),
        m_NewDrinkPopup(db),
        m_IngredientsTable(db),
        m_IngredientsDrinksTable(db)
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

                if(ImGui::IsItemHovered()) {
                    int drink_id = query.GetColumnInt(0);

                    //Yes, it is fucking horrible, i know
                    std::stringstream tooltip;

                    QueryResult ingredients_of_drink = m_IngredientsDrinksTable.Query(drink_id);


                    for(; ingredients_of_drink; ingredients_of_drink.Next()){
                        int ingredient_id = ingredients_of_drink.GetColumnInt(0);
                        float units = ingredients_of_drink.GetColumnFloat(1);

                        QueryResult ingredient = m_IngredientsTable.Query(ingredient_id);

                        tooltip << ingredient.GetColumnString(1) << ' ' << units << ' ' << ingredient.GetColumnString(2) << '\n';
                    }

                    ImGui::SetTooltip(tooltip.str().c_str());
                }

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
                && ImGui::BeginTable("Drinks", 3)) {
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
    List<std::string> m_History;
    size_t m_HistoryIndex = 0;
public:
    ConsoleWindow(DatabaseLogger &logger, Database &db):
            m_Logger(logger),
            m_Database(db),
            m_History{""}
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