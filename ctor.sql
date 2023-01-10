CREATE TABLE Addresses(
    ID int PRIMARY KEY NOT NULL,
    City varchar(64),
    Street varchar(64),
    House varchar(2),
    PostalCode int
);

CREATE TABLE Sources(
    ID int PRIMARY KEY NOT NULL,
    Name varchar(64),
    AddressID REFERENCES Addresses(ID)
);

CREATE TABLE Ingredients(
    ID int PRIMARY KEY NOT NULL,
    Name varchar(64) NOT NULL,
    Units varchar(64) NOT NULL,
    PricePerUnit float NOT NULL,
    SourceID REFERENCES Sources(ID)
);

CREATE TABLE Drinks(
    ID int PRIMARY KEY NOT NULL,
    Name varchar(64),
    PricePerLiter float,
    AgeRestriction int
);

CREATE TABLE IngredientsDrinks(
    IngredientID REFERENCES Ingredients(ID),
    UnitsCount float,
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

CREATE TABLE OrdersLog(
    ID int PRIMARY KEY NOT NULL,
    CustomerShortName varchar(64),
    Tips float,
    WaiterID REFERENCES Waiters(ID),
    Checkout float,
    OrderDate date
);

CREATE TABLE DrinkOrders(
    OrderID REFERENCES OrdersLog(ID),
    DrinkID REFERENCES Drinks(ID),
    GobletID REFERENCES Goblets(ID)
);