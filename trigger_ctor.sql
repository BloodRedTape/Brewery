

CREATE TRIGGER ValidateWaiterAge
    BEFORE INSERT ON Waiters
BEGIN
    SELECT
    CASE
    WHEN NEW.FullAge < 18 THEN
        RAISE (ABORT,'This waiter can not be added because of an age restriction')
    END;
END;