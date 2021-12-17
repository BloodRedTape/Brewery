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

    size_t Size(){
        return m_Database.Size("Goblets");
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
        m_Database.Execute(Stmt("DELETE FROM Waiters"));
    }
};

class IngredientsTableMediator{
private:
    Database &m_Database;
public:
    IngredientsTableMediator(Database &db):
            m_Database(db)
    {}

    QueryResult Query(){
        return m_Database.Query(Stmt("SELECT * FROM Ingredients"));
    }

    void Add(int id, const char *name, const char *units, int source_id){
        m_Database.Execute(
                Stmt(
                    "INSERT INTO Ingredients(ID, Name, Units, SourceID) VALUES(%, '%', '%', %)",
                    id,
                    name,
                    units,
                    source_id
                )
        );
    }

    QueryResult Query(int id){
        return m_Database.Query(Stmt("SELECT * FROM Ingredients WHERE ID = %", id));
    }

    void Clear(){
        m_Database.Execute(Stmt("DELETE FROM Ingredients"));
    }

    size_t Size(){
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