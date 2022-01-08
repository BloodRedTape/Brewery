
CREATE VIEW OrdersWithNameAndCount AS
    SELECT OrdersLog.ID, CustomerShortName AS CustomerName, Tips, Waiters.ShortName AS WaiterName,
    (SELECT count(*) FROM DrinkOrders WHERE OrderID == OrdersLog.ID) AS DrinksCount
    FROM OrdersLog JOIN Waiters on Waiters.ID = OrdersLog.WaiterID;

CREATE VIEW DrinksWithIngredientsCount AS
    SELECT *, (SELECT count(*) FROM IngredientsDrinks WHERE DrinkID == ID) AS IngredientsCount
    FROM Drinks;