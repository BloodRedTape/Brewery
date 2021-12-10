CREATE TABLE Addresses(
    ID int PRIMARY KEY NOT NULL,
    City varchar(64),
    House varchar(2),
    PostalCode int
);

CREATE TABLE Sources(
    ID int PRIMARY KEY NOT NULL,
    Name varchar(64),
    AddressID REFERENCES Addresses(ID)
);

CREATE TABLE Ingredients(
    Name varchar(64) PRIMARY KEY NOT NULL,
    Units varchar(64) NOT NULL,
    SourceID REFERENCES Sources(ID)
);

CREATE TABLE IngredientStorages(
    Name varchar(256) PRIMARY KEY NOT NULL,
    IngredientName REFERENCES Ingredients(Name),
    UnitsCount float
);

CREATE TABLE IngredientPortions(
    ID int PRIMARY KEY,
    IngredientName REFERENCES Ingredients(Name),
    UnitsCount float
);

CREATE TABLE Drinks(
    ID int PRIMARY KEY NOT NULL,
    Name varchar(64),
    PricePerLiter float,
    AgeRestriction int
);

CREATE TABLE IngredientsDrinks(
    IngredientPortionID int REFERENCES IngredientPortions(ID),
    DrinkID REFERENCES Drinks(ID)
);

CREATE TABLE Waiters(
    ID int PRIMARY KEY NOT NULL,
    ShortName varchar(64),
    Salary float,
    FullAge int
);

CREATE TABLE Goblets(
    ID int PRIMARY KEY NOT NULL,
    Name varchar(64),
    Capacity float
);

CREATE TABLE DrinkOrders(
    ID int PRIMARY KEY NOT NULL,
    DrinkID REFERENCES Drinks(ID),
    GobletID REFERENCES Goblets(ID)
);

CREATE TABLE OrdersLog(
    ID int PRIMARY KEY NOT NULL,
    CustomerShortName varchar(64),
    Tips float,
    WaiterID REFERENCES Waiters(ID)
);

CREATE TABLE OrderDrinks(
    OrderID REFERENCES OrdersLog(ID),
    DrinkOrderID REFERENCES DrinkOrders(ID)
);