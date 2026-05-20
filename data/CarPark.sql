PRAGMA foreign_keys = ON;

CREATE TABLE Users (
    id INTEGER PRIMARY KEY,
    login TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,
    role TEXT NOT NULL
);

CREATE TABLE Drivers (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    category TEXT,
    experience INTEGER,
    address TEXT,
    birth_year INTEGER,
    user_id INTEGER NOT NULL UNIQUE,
    FOREIGN KEY (user_id) REFERENCES Users(id)
);

CREATE TABLE Vehicles (
    id INTEGER PRIMARY KEY,
    number TEXT NOT NULL UNIQUE,
    brand TEXT,
    mileage INTEGER,
    capacity REAL NOT NULL CHECK (capacity > 0)
);

CREATE TABLE Orders (
    id INTEGER PRIMARY KEY,
    order_date DATE NOT NULL,
    driver_id INTEGER NOT NULL,
    vehicle_id INTEGER NOT NULL,
    distance REAL,
    cargo_weight REAL NOT NULL,
    cost REAL NOT NULL,
    FOREIGN KEY (driver_id) REFERENCES Drivers(id),
    FOREIGN KEY (vehicle_id) REFERENCES Vehicles(id)
);

CREATE TABLE Driver_earnings (
    id INTEGER PRIMARY KEY,
    driver_id INTEGER,
    period_start DATE,
    period_end DATE,
    total_amount REAL,
    FOREIGN KEY (driver_id) REFERENCES Drivers(id)
);