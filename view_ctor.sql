

CREATE VIEW OrdersWithWaiterName AS
    SELECT OrdersLog.ID, CustomerShortName AS CustomerName, Tips, Waiters.ShortName AS WaiterName
    FROM OrdersLog JOIN Waiters on Waiters.ID = OrdersLog.WaiterID;