#include <imgui/backend.hpp>
#include <core/string.hpp>
#include <core/function.hpp>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <functional>
#include <map>
#include "helpers.cpp"
#include "mediators.cpp"
#include "imgui_internal.h"

static std::map<std::string, float> s_Available;

float& GetAvailableDrinks(const char* name) {
    if(!s_Available.count(name))
        s_Available[name] = rand() % 20;

    return s_Available[name];
}

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

namespace ImGui
{
    void InputDate(const char *label, int &current_day, int &current_month, int &current_year)
    {
        ImGui::PushID(ImGui::GetID(label));

        static constexpr int DaysInMonth[] = {
            31, 
            27,
            31,
            30,
            31,
            30,
            31,
            31,
            30,
            31,
            30,
            31
        };

        static constexpr const char *Months[] = {
            "Janyary", "February", "March", "April",   
            "May",    "June",  "July",  "August", 
            "September", "October", "November", "December"
        };

        if (current_year < 0) current_year = 0;
        if (current_month < 1) current_month = 1;
        if (current_month > 12) current_month = 12;
        if (current_month > 12) current_month = 12;
        if (current_day < 1) current_day = 1;
        if (current_day > DaysInMonth[current_month - 1]) current_day = DaysInMonth[current_month - 1];

        ImGui::Text(label);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        if (ImGui::DragInt("##Day", &current_day, 1.f, 1, DaysInMonth[current_month - 1]))
            current_day + 1;
        ImGui::SameLine();

        ImGui::SetNextItemWidth(140);
        if (ImGui::BeginCombo("##MonthCombo", Months[current_month - 1])) {
            for (int i = 0; i < lengthof(Months); i++)
                if (ImGui::Selectable(Months[i]))
                    current_month = i + 1;
            ImGui::EndCombo();
        }
        ImGui::SameLine();

        ImGui::SetNextItemWidth(140);
        ImGui::InputInt("##year", &current_year);
        ImGui::PopID();
    }

    void InputDate(const char* label, Date& date) {
        return InputDate(label, date.Day, date.Month, date.Year);
    }
}

class NewIngredientPopup{
    static constexpr size_t BufferSize = 1024;
private:
    IngredientsTableMediator m_IngredientsTable;
    SourcesTableMediator m_SourcesTable;
    InputBuffer<BufferSize> m_IngredientName;
    InputBuffer<BufferSize> m_UnitsName;

    int m_SourceID = 0;
    
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

class NewGobletPopup{
    static constexpr size_t BufferSize = 1024;
private:
    GobletsTableMediator m_GobletsTable;
    InputBuffer<BufferSize> m_GobletName;
    float m_Capacity = 0.f;

    const char *const m_Name = "New Goblet";
public:
    NewGobletPopup(Database &db):
            m_GobletsTable(db)
    {}

    void Open(){
        ImGui::OpenPopup(m_Name);
    }

    void Draw(){
        if(ImGui::BeginPopup(m_Name)) {
            ImGui::InputText("Name", m_GobletName.Data(), m_GobletName.Size());
            ImGui::InputFloat("Capacity", &m_Capacity);

            if (ImGui::Button("Add") && IsDataValid()) {
                m_GobletsTable.Add(
                        m_GobletName.Data(),
                        m_Capacity
                );

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }else{
            m_GobletName.Clear();
            m_Capacity = 0.f;
        }
    }
private:
    bool IsDataValid()const{
        return m_GobletName.Length() && m_Capacity > 0.f;
    }
};

class GobletsListPanel{
private:
    GobletsTableMediator m_GobletsTable;
    NewGobletPopup m_NewGobletPopup;
public:
    GobletsListPanel(Database &db):
            m_GobletsTable(db),
            m_NewGobletPopup(db)
    {}

    void Draw(){

        ImGui::Begin("Goblets");

        if(ImGui::Button("Clear"))
            m_GobletsTable.Clear();

        ImGui::SameLine();

        if(ImGui::Button("New Goblet"))
            m_NewGobletPopup.Open();
        m_NewGobletPopup.Draw();

        ImGui::Separator();

        ImGui::BeginChild("##List");

        QueryResult query = m_GobletsTable.Query();

        if(ImGui::BeginTable("Goblets", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)){
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Capacity");
            ImGui::TableHeadersRow();

            for( ;query; query.Next()){
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", query.GetColumnString(1));
                ImGui::TableNextColumn();
                ImGui::Text("%.1f", query.GetColumnFloat(2));
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
                (void)m_AddressesTable.TryAdd(
                        m_City.Data(),
                        m_House.Data(),
                        m_PostalCode
                );

                auto address = m_AddressesTable.Query(m_City.Data(), m_House.Data(), m_PostalCode);

                m_SourcesTable.Add(m_SourceName.Data(), address.GetColumnInt(0));

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

enum class TransferStatus: int{
    Placed = 0,
    Accepted = 1,
    Brewing = 2,
    Transfering = 3,
    Done = 4,
    Max
};

static constexpr const char *TransferStatusNames[] = {
    "Placed",
    "Accepted",
    "Brewing",
    "Transfering",
    "Done",
    "INTERNAL_MAX"
};

using Item = std::pair<std::string, float>;

struct TransferEntry{
    int ID = 0;
    std::string Source;

    std::vector<Item> Items;

    TransferStatus Status = TransferStatus::Placed;
    
    std::function<void(const std::vector<Item> &items)> OnDone;

    void Draw(){
        ImGui::PushID(ID);
        ImGui::Text("Order ID: %d", ID);
        ImGui::Text("Status: %s", TransferStatusNames[(int)Status]);

        float color = float(Status) / float((int)TransferStatus::Max - 1);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(color, 0.5f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(color, 0.6f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(color, 0.7f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(color, 0.9f, 0.9f));
        ImGui::SliderInt("##Status", (int*)&Status, 0, (int)TransferStatus::Max - 1, "");
        ImGui::PopStyleColor(4);
        
        if (ImGui::TreeNode("Drinks"))
        {
            for (auto item : Items) 
                ImGui::BulletText("%s %.2f", item.first.c_str(), item.second);

            ImGui::TreePop();
        }

        if (Status != TransferStatus::Done) {
            if (ImGui::Button("Fake Done")) {
                if(OnDone) OnDone(Items);

                Status = TransferStatus::Done;
            }
        }

        ImGui::PopID();
    }
};

struct DrinksTransferProgressWindow {
    std::vector<TransferEntry> Entries;
    
    DrinksTransferProgressWindow() {
        Entries.push_back({
            0,
            "Granny",
            {
                {"Cider", 1.f}
            },
            TransferStatus::Accepted
        });

        Entries.push_back({
            1,
            "Granny",
            {
                {"Cider", 1.f}
            },
            TransferStatus::Brewing
        });


        Entries.push_back({
            2,
            "Someone",
            {
                {"Cider", 1.f}
            },
            TransferStatus::Done
        });
    }

    void Draw(){
        ImGui::Begin("Transfer Progress");
        
        for (auto& Entry : Entries) {
            Entry.Draw();
            ImGui::Separator();
        }
        ImGui::End();
    }
};

class NewDrinkOrderPopup{
    static constexpr size_t BufferSize = 1024;
private:
    DrinksTableMediator m_DrinksTable;

    const char *const m_Name = "New Drink Order";
    
    std::map<std::string, int> m_OrderCounts;
	DrinksTransferProgressWindow &m_Transfer;
    std::string m_Source;
public:
    NewDrinkOrderPopup(Database &db, DrinksTransferProgressWindow &transfer):
        m_DrinksTable(db),
        m_Transfer(transfer)
    {}

    void Open(const std::string &source){
        ImGui::OpenPopup(m_Name);
        m_Source = source;
    }

    void Draw(){
        if(ImGui::BeginPopup(m_Name)) {

            ImGui::Text("Order from: %s", m_Source.c_str());

            for (auto query = m_DrinksTable.Query(); query; query.Next()) {
                auto name = query.GetColumnString(1);
                int &count = m_OrderCounts[name];

                ImGui::PushID(query.GetColumnInt(0));
                ImGui::Text("%s", name);
                ImGui::SameLine();
                ImGui::Text("Available: %.2f, ", GetAvailableDrinks(name));
                ImGui::SameLine();
                ImGui::InputInt("Count", &count);

                ImGui::PopID();
            }

            if (ImGui::Button("Place Order")){
                std::vector<Item> items;

                for (auto order : m_OrderCounts) if(order.second > 0) items.push_back(order);

                m_Transfer.Entries.push_back({
                    (int)m_Transfer.Entries.size(),
                    m_Source,
                    items,
                    TransferStatus::Placed
                });

                m_Transfer.Entries.back().OnDone = [](const std::vector<Item> &items) {
                    for (auto item : items) {
                        GetAvailableDrinks(item.first.c_str()) += item.second;
                    }
                };
                ImGui::CloseCurrentPopup();
                m_OrderCounts = {};
            }

            if (ImGui::Button("Cancel")){
                ImGui::CloseCurrentPopup();
                m_OrderCounts = {};
            }

            ImGui::EndPopup();
        }else{
        }
    }
};


class SourcesListPanel{
private:
    SourcesTableMediator m_SourcesTable;
    AddressesTableMediator m_AddressesTable;
    NewSourcePopup m_NewSourcePopup;

    NewDrinkOrderPopup m_DrinkOrderPopup;
public:
    SourcesListPanel(Database &db, DrinksTransferProgressWindow &transfer):
            m_SourcesTable(db),
            m_AddressesTable(db),
            m_NewSourcePopup(db),
            m_DrinkOrderPopup(db, transfer)
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
            ImGui::TableSetupColumn("PostalCode");
            ImGui::TableHeadersRow();

            QueryResult query = m_SourcesTable.Query();
            for( ;query; query.Next()){
                ImGui::PushID(query.GetColumnInt(0));

                auto address = m_AddressesTable.Query(query.GetColumnInt(2));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", query.GetColumnString(1));
                ImGui::TableNextColumn();
                ImGui::Text("%s", address.GetColumnString(1));
                ImGui::TableNextColumn();
                ImGui::Text("%d", query.GetColumnInt(3));
                ImGui::TableNextColumn();
                if (ImGui::Button("Order")) {
                    m_DrinkOrderPopup.Open(query.GetColumnString(1));
                }
                m_DrinkOrderPopup.Draw();

                ImGui::PopID();
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


        if(ImGui::BeginTable("Drinks", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)){
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Price");
            ImGui::TableSetupColumn("AgeRestr");
            ImGui::TableSetupColumn("Available");

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
                ImGui::TableNextColumn();
                ImGui::Text("%.1f Liters", GetAvailableDrinks(query.GetColumnString(1)));

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
    float m_Tips = 0.f;

    List<DrinkOrder> m_Drinks;

    int m_CurrentDrinkID = -1;
    int m_CurrentGobletID = -1;
    int m_CurrentWaiterID = -1;

    int m_CurrentDay   = 0;
    int m_CurrentMonth = 0;
    int m_CurrentYear  = 2022;

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

    std::map<std::string, float> Usage() {
        std::map<std::string, float> usage;

        for(auto drink: m_Drinks){
            auto drink_query = m_DrinksTable.Query(drink.DrinkID);
            auto goblet_query = m_GobletsTable.Query(drink.GobletID);

            if (!drink_query || !goblet_query)continue;
            
            usage[drink_query.GetColumnString(1)] += goblet_query.GetColumnFloat(2);
        }

        return usage;
    }

    float AvailableLiters(const char* name) {
        if(!name)
            return 0;

        auto res = GetAvailableDrinks(name) - Usage()[name];

        if(res >= 0)
            return res;

        return 0;
    }

    bool IsAvailable(const char* name) {
        return AvailableLiters(name) > 0;
    }

    bool IsAvailableForGoblet(const char* name, float capacity) {
        return AvailableLiters(name) >= capacity;
    }

    void Draw(){

        if(ImGui::BeginPopup(m_Name)) {
            ImGui::InputText("Customer Codename", m_CustomerName.Data(), m_CustomerName.Size());
            if (ImGui::BeginCombo("##WaitersCombo",
                                  m_CurrentWaiterID != -1 ? m_WaitersTable.Query(m_CurrentWaiterID).GetColumnString(1) : "None")) {
                auto waiters_query= m_WaitersTable.Query();
                for (; waiters_query; waiters_query.Next()) {
                    int id = waiters_query.GetColumnInt(0);
                    const char *name = waiters_query.GetColumnString(1);

                    if (ImGui::Selectable(name))
                        m_CurrentWaiterID = id;
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();

            ImGui::Text("Waiter");

            ImGui::InputFloat("Tips", &m_Tips);

            auto selected_drink_query = m_DrinksTable.Query(m_CurrentDrinkID);

            ImGui::PushItemWidth(ImGui::GetWindowSize().x / 3);

            auto current_name = selected_drink_query.GetColumnString(1);

            if(!IsAvailable(current_name))
                m_CurrentDrinkID = -1;

            if (ImGui::BeginCombo("##DrinksCombo",
                                  IsAvailable(current_name) ? current_name : "None")) {
                auto drink_query = m_DrinksTable.Query();
                for (; drink_query; drink_query.Next()) {
                    int id = drink_query.GetColumnInt(0);
                    const char *name = drink_query.GetColumnString(1);

                    bool is_available = IsAvailable(name);

                    std::string selectable = name;
                    
                    if(!is_available)ImGui::PushDisabled();

                    if (ImGui::Selectable(selectable.c_str()))
                        m_CurrentDrinkID = id;

                    if(!is_available)ImGui::PopDisabled();
                }
                ImGui::EndCombo();
            }

          if(m_CurrentDrinkID != -1){
            ImGui::SameLine();
             
            float current_capacity = m_GobletsTable.Query(m_CurrentGobletID).GetColumnFloat(2);

            if(!IsAvailableForGoblet(current_name, current_capacity))
                m_CurrentGobletID = -1;

            if (ImGui::BeginCombo("##GobletsCombo", GetGobletName(m_GobletsTable.Query(m_CurrentGobletID)).c_str())) {
                auto goblet_query = m_GobletsTable.Query();
                for (; goblet_query; goblet_query.Next()) {
                    int id = goblet_query.GetColumnInt(0);
                    float capacity = goblet_query.GetColumnFloat(2);
                    std::string name = GetGobletName(goblet_query);

                    bool is_available = IsAvailableForGoblet(current_name, capacity);

                    if(!is_available)ImGui::PushDisabled();
                    if (ImGui::Selectable(name.c_str()))
                        m_CurrentGobletID = id;
                    if(!is_available)ImGui::PopDisabled();
                }
                ImGui::EndCombo();
            }
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

            float checkout = 0;

            for (DrinkOrder drink_order : m_Drinks) {
                QueryResult drink = m_DrinksTable.Query(drink_order.DrinkID);
                QueryResult goblet = m_GobletsTable.Query(drink_order.GobletID);
                float price_per_liter = drink.GetColumnFloat(2);
                float capacity = goblet.GetColumnFloat(2);
                checkout += price_per_liter * capacity;
            }

            ImGui::Text("Checkout: %.2f", checkout);
            
            ImGui::InputDate("Date", m_CurrentDay, m_CurrentMonth, m_CurrentYear);

            if (ImGui::Button("Place Order") && IsDataValid() && m_CurrentWaiterID != -1) {
                Date date{
                    m_CurrentDay,
                    m_CurrentMonth,
                    m_CurrentYear
                };
                int id = m_OrdersLogTable.Add(m_CustomerName.Data(), m_Tips, m_CurrentWaiterID, checkout, date);

                for (auto drink: m_Drinks) {
                    m_DrinksOrdersTable.Add(id, drink.DrinkID, drink.GobletID);
                }

                for (auto usage : Usage()) {
                    GetAvailableDrinks(usage.first.c_str()) -= usage.second;
                }

                ImGui::CloseCurrentPopup();
            }


            ImGui::SameLine();

            if(ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }else{
            m_CustomerName.Clear();

            m_Drinks.Clear();
        }
    }
private:
    bool IsDataValid(){
        return m_Drinks.Size() && m_CustomerName.Length();
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
                    ImGui::Text("%.1f", goblet.GetColumnFloat(2));
                }

                ImGui::EndTable();
            }
            ImGui::Text("Checkout: %.2f", orders.GetColumnFloat(4));
            ImGui::Text("Date: %s", orders.GetColumnString(5));

            ImGui::Separator();
        }

        ImGui::EndChild();

        ImGui::End();


    }
};

class AnalyticsWindow{
private:
    Database &m_DB;
    DrinksTableMediator m_DrinksTable{m_DB};
    WaitersTableMediator m_WaitersTable{m_DB};
    OrdersLogTableMediator m_OrdersLogTable{m_DB};
    DrinkOrdersTableMediator m_DrinkOrdersTable{m_DB};

    Date m_WaitersBegin{1, 1, 2020};
    Date m_WaitersEnd{1, 1, 2024};

    Date m_DrinkMarketBegin{1, 1, 2020};
    Date m_DrinkMarketEnd{1, 1, 2024};

private:
    static double DateToDouble(std::string date) {
        int day;
        int month;
        int year;
        sscanf(date.data(), "%d-%d-%d", &year, &month, &day);

        return year * 10000 + month * 100 + day * 1;
    }

    static String DoubleToDate(double date) {
        int day = (int)date % 100;
        int month = ((int)date / 100) % 100;
        int year = (int)date / 10000;
        return StringPrint("%-%-%", year, month, day);
    }
   
public:
    AnalyticsWindow(Database &db): 
        m_DB(db) 
    {}

    void Draw() {
        ImGui::Begin("Stats");
        
        
        std::unordered_map<std::string, float> checkouts;

        for (auto query = m_OrdersLogTable.Query(); query; query.Next()) {
            float checkout = query.GetColumnFloat(4);
            std::string date = query.GetColumnString(5) ? query.GetColumnString(5) : "Null";
            
            checkouts[date] += checkout;
        }

        const auto win_size = ImGui::GetContentRegionAvail();
        const int plot_count = 3;
        const auto plot_size = ImVec2{win_size.x * 0.6f, win_size.y * 0.6f};

        ImGui::InputDate("Waiters Start", m_WaitersBegin);

        ImGui::InputDate("Waiters End", m_WaitersEnd);
        
        if (ImPlot::BeginPlot("Waiters stats", plot_size)) {
            List<String> names;
            List<float> values;
            List<const char *> names_ptr;

            std::map<int, int> orders_by_waiter;

            for (auto query = m_OrdersLogTable.Query(m_WaitersBegin, m_WaitersEnd); query; query.Next()) {
                int waiter_id = query.GetColumnInt(3);

                orders_by_waiter[waiter_id] += 1;
            }

            for (auto [waiter_id, orders] : orders_by_waiter) {
                auto query = m_WaitersTable.Query(waiter_id);
                auto waiter_name = query.GetColumnString(1);
                names.Add(waiter_name);
                names_ptr.Add(names.Last().Data());
                values.Add(orders);
            }

            float sum = 0;
            
            for (auto value : values) 
                sum += value; 

            for (float &value : values) value = (value / sum) * 100;


            ImPlot::PlotPieChart(names_ptr.Data(), values.Data(), values.Size(),
                                 0, 0, 1, "%.1f%%");

            ImPlot::EndPlot();
        }

        ImGui::InputDate("Market Share Start", m_DrinkMarketBegin);

        ImGui::InputDate("Market Share End", m_DrinkMarketEnd);

        if (ImPlot::BeginPlot("Drinks market share", plot_size)) {
             
            List<String> names;
            List<float> values;
            List<const char *> names_ptr;

            std::map<int, int> drink_market_share;

            for (auto query = m_OrdersLogTable.Query(m_DrinkMarketBegin, m_DrinkMarketEnd); query; query.Next()) {
                int order_id = query.GetColumnInt(0);

                for (auto order_drink = m_DrinkOrdersTable.Query(order_id); order_drink; order_drink.Next()) {
                    int drink_id = order_drink.GetColumnInt(1);

                    drink_market_share[drink_id] += 1;
                }
            }

            for (auto [drink_id, bought] : drink_market_share) {
                auto query = m_DrinksTable.Query(drink_id);
                auto drink_name = query.GetColumnString(1);
                names.Add(drink_name);
                names_ptr.Add(names.Last().Data());
                values.Add(bought);
            }

            float sum = 0;
            
            for (auto value : values) 
                sum += value; 

            for (float &value : values) value = (value / sum) * 100;


            ImPlot::PlotPieChart(names_ptr.Data(), values.Data(), values.Size(),
                                 0, 0, 1, "%.1f%%");

            ImPlot::EndPlot();
        }        

        ImGui::End();
    }
};


class ConsoleWindow: public SimpleInterpreter{
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
    {

        Register("clear", {this, &ConsoleWindow::OnClear});
    }

    void Draw(){

        const auto &lines = m_Logger.Lines();
        ImGui::Begin("Console");

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("##Text", ImVec2(0, -footer_height_to_reserve));

        for(const auto &line: lines)
            ImGui::TextWrapped("%s", line.Data());

        ImGui::EndChild();

        ImGui::Separator();

        ImGui::PushItemWidth(ImGui::GetWindowWidth());

        if(ImGui::InputText("##Input", m_CurrentLine.Data(), m_CurrentLine.Size(), ImGuiInputTextFlags_EnterReturnsTrue)){
            m_Logger.Log("[User]: %", m_CurrentLine.Data());

            if(!TryInterpret(m_CurrentLine.Data())) {
                auto query = m_Database.Query(m_CurrentLine.Data());
                if(query)
                    m_Logger.Log(query);
            }

            m_CurrentLine.Clear();


            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
        }
        ImGui::PopItemWidth();

        ImGui::End();
    }

    void OnClear(const char *){
        m_Logger.Clear();
    }
};