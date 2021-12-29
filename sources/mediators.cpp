#include "database.cpp"

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

    size_t Size(){
        return m_Database.Size("Drinks");
    }
};

class GobletsTableMediator{
private:
    Database &m_Database;
    int m_LastID{(int)m_Database.Size("Goblets")};
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
        m_LastID = 0;
        m_Database.Execute(Stmt("DELETE FROM Goblets"));
    }

    void Add(const char *name, float capacity){
        m_Database.Execute(
                Stmt(
                        "INSERT INTO Goblets(ID, Name, Capacity) VALUES(%, '%', %)",
                        ++m_LastID,
                        name,
                        capacity
                )
        );
    }

    size_t Size(){
        return m_Database.Size("Goblets");
    }
};

class OrdersLogTableMediator{
private:
    Database &m_Database;
    int m_LastID{(int)m_Database.Size("OrdersLog")};
public:
    OrdersLogTableMediator(Database &db):
            m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM OrdersLog"));
    }

    void Clear(){
        m_LastID = 0;
        m_Database.Execute(Stmt("DELETE FROM OrdersLog"));
    }

    int Add(const char *customer_name, float tips, int waiter_id){
        ++m_LastID;
        m_Database.Execute(
                Stmt(
                        "INSERT INTO OrdersLog(ID, CustomerShortName, Tips, WaiterID) VALUES(%,'%',%,%)",
                        m_LastID,
                        customer_name,
                        tips,
                        waiter_id
                )
        );
        return m_LastID;
    }

    size_t Size(){
        return m_Database.Size("OrdersLog");
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

class WaitersTableMediator{
private:
    Database &m_Database;
    int m_LastID{(int)m_Database.Size("Waiters")};
public:
    WaitersTableMediator(Database &db):
            m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM Waiters"));
    }

    int Add(const char *name, float salary, int age){
        ++m_LastID;
        m_Database.Execute(
                Stmt(
                        "INSERT INTO Waiters(ID, ShortName, Salary, FullAge) VALUES(%, '%', %, %)",
                        m_LastID,
                        name,
                        salary,
                        age
                )
        );
        return m_LastID;
    }

    QueryResult Query(const char *name){
        return m_Database.Query(Stmt("SELECT * FROM Waiters WHERE ShortName = '%'", name));
    }

    QueryResult Query(int id){
        return m_Database.Query(Stmt("SELECT * FROM Waiters WHERE ID = %", id));
    }

    bool Exists(const char *name){
        return Query(name);
    }

    size_t Size(){
        return m_Database.Size("Waiters");
    }

    void Clear(){
        m_LastID = 0;
        m_Database.Execute(Stmt("DELETE FROM Waiters"));
    }
};

class IngredientsTableMediator{
private:
    Database &m_Database;
    int m_LastID{(int)m_Database.Size("Ingredients")};
public:
    IngredientsTableMediator(Database &db):
            m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM Ingredients"));
    }

    int Add(const char *name, const char *units, int source_id){
        ++m_LastID;
        m_Database.Execute(
                Stmt(
                    "INSERT INTO Ingredients(ID, Name, Units, SourceID) VALUES(%, '%', '%', %)",
                    m_LastID,
                    name,
                    units,
                    source_id
                )
        );
        return m_LastID;
    }

    QueryResult Query(int id){
        return m_Database.Query(Stmt("SELECT * FROM Ingredients WHERE ID = %", id));
    }

    void Clear(){
        m_Database.Execute(Stmt("DELETE FROM Ingredients"));
    }

    size_t Size(){
        m_LastID = 0;
        return m_Database.Size("Ingredients");
    }
};



class IngredientsDrinksTableMediator{
private:
    Database &m_Database;
public:
    IngredientsDrinksTableMediator(Database &db):
            m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM IngredientsDrinks"));
    }

    void Add(int ingredient_id, float units, int drink_id){
        m_Database.Execute(
                Stmt(
                        "INSERT INTO IngredientsDrinks(IngredientID, UnitsCount, DrinkID) VALUES(%, %, %)",
                        ingredient_id,
                        units,
                        drink_id
                )
        );
    }

    QueryResult Query(int drink_id){
        return m_Database.Query(Stmt("SELECT * FROM IngredientsDrinks WHERE DrinkID = %", drink_id));
    }

    void Clear(){
        m_Database.Execute(Stmt("DELETE FROM IngredientsDrinks"));
    }
};

class SourcesTableMediator{
private:
    Database &m_Database;
    int m_LastID{(int)m_Database.Size("Sources")};
public:
    SourcesTableMediator(Database &db):
            m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM Sources"));
    }

    int Add(const char *name, int address_id){
        ++m_LastID;
        m_Database.Execute(
                Stmt(
                        "INSERT INTO Sources(ID, Name, AddressID) VALUES(%, '%', %)",
                        m_LastID,
                        name,
                        address_id
                )
        );
        return m_LastID;
    }

    QueryResult Query(int id){
        return m_Database.Query(Stmt("SELECT * FROM Sources WHERE ID = %", id));
    }

    void Clear(){
        m_LastID = 0;
        m_Database.Execute(Stmt("DELETE FROM Sources"));
    }

    size_t Size()const{
        return m_Database.Size("Sources");
    }
};

class AddressesTableMediator{
private:
    Database &m_Database;
    int m_LastID{(int)m_Database.Size("Addresses")};
public:
    AddressesTableMediator(Database &db):
            m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM Addresses"));
    }

    int TryAdd(const char *city, const char *house, int postal_code){
        if(Query(city, house, postal_code))return 0;

        ++m_LastID;
        m_Database.Execute(
                Stmt(
                        "INSERT INTO Addresses(ID, City, House, PostalCode) VALUES(%, '%', '%', %)",
                        m_LastID,
                        city,
                        house,
                        postal_code
                )
        );

        return m_LastID;
    }

    QueryResult Query(const char *city, const char *house, int postal_code){
        return m_Database.Query(Stmt("SELECT * FROM Addresses WHERE City = '%' AND House = '%' AND PostalCode = %", city, house, postal_code));
    }

    QueryResult Query(int id){
        return m_Database.Query(Stmt("SELECT * FROM Addresses WHERE ID = %", id));
    }


    void Clear(){
        m_LastID = 0;
        m_Database.Execute(Stmt("DELETE FROM Addresses"));
    }

    size_t Size()const{
        return m_Database.Size("Addresses");
    }
};

class StoredProcedures {
    Database &m_Database;
public:
    StoredProcedures(Database &db) :
            m_Database(db)
    {}

    QueryResult GetAllSourcesWithCity(const char *city){
        return m_Database.Query(Stmt("SELECT * FROM Sources WHERE AddressID IN (SELECT ID FROM Addresses WHERE City = '%')", city));
    }

    QueryResult GetExpensiveWaiters(float salary_limit){
        return m_Database.Query(Stmt("SELECT * FROM Waiters WHERE Salary > %", salary_limit));
    }

    QueryResult GetDrinksWith(const char *ingredient_name){
        return m_Database.Query(Stmt("SELECT * FROM Drinks WHERE Drinks.ID IN"
                                     "(SELECT DrinkID FROM IngredientsDrinks WHERE IngredientID IN"
                                     "(SELECT ID FROM Ingredients WHERE Name = '%'))", ingredient_name));
    }

    QueryResult GetIngredientsWithPriceLessThan(float price){
        return m_Database.Query(Stmt("SELECT * FROM Ingredients WHERE PricePerUnit < %", price));
    }

    QueryResult GetGobletWithCapacityMoreThan(float capacity){
        return m_Database.Query(Stmt("SELECT * FROM Goblets WHERE Capacity > %", capacity));
    }
};