#include "AutoparkService.h"
#include <cctype>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>

static void bindText(sqlite3_stmt* s, int i, const std::string& v){ sqlite3_bind_text(s,i,v.c_str(),-1,SQLITE_TRANSIENT); }
static void header(const std::string& title){ std::cout << "\n=== " << title << " ===\n"; }
static void requireDone(sqlite3* db, sqlite3_stmt* stmt) {
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string e = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(e);
    }
    sqlite3_finalize(stmt);
}

void AutoparkService::initialize() {
    db.exec(R"SQL(
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS Users (
    id INTEGER PRIMARY KEY,
    login TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,
    role TEXT NOT NULL CHECK(role IN ('admin','driver'))
);

CREATE TABLE IF NOT EXISTS Drivers (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    category TEXT,
    experience INTEGER,
    address TEXT,
    birth_year INTEGER,
    user_id INTEGER NOT NULL UNIQUE,
    FOREIGN KEY (user_id) REFERENCES Users(id)
);

CREATE TABLE IF NOT EXISTS Vehicles (
    id INTEGER PRIMARY KEY,
    number TEXT NOT NULL UNIQUE,
    brand TEXT,
    mileage INTEGER,
    capacity REAL NOT NULL CHECK (capacity > 0)
);

CREATE TABLE IF NOT EXISTS Orders (
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

CREATE TABLE IF NOT EXISTS Driver_earnings (
    id INTEGER PRIMARY KEY,
    driver_id INTEGER,
    period_start DATE,
    period_end DATE,
    total_amount REAL,
    FOREIGN KEY (driver_id) REFERENCES Drivers(id)
);

CREATE TRIGGER IF NOT EXISTS trg_orders_check_capacity
BEFORE INSERT ON Orders
FOR EACH ROW
BEGIN
    SELECT CASE
        WHEN NEW.cargo_weight > (SELECT capacity FROM Vehicles WHERE id = NEW.vehicle_id)
        THEN RAISE(ABORT, 'Cargo mass exceeds vehicle capacity')
    END;
END;
)SQL");
}

void AutoparkService::addCar(const std::string& number, const std::string& brand, int initialMileage, double capacityKg) {
    sqlite3_stmt* stmt=nullptr;
    sqlite3_prepare_v2(db.raw(), "INSERT INTO Vehicles(number,brand,mileage,capacity) VALUES(?,?,?,?)", -1, &stmt, nullptr);
    bindText(stmt,1,number); bindText(stmt,2,brand); sqlite3_bind_int(stmt,3,initialMileage); sqlite3_bind_double(stmt,4,capacityKg);
    requireDone(db.raw(), stmt);
}

void AutoparkService::updateCarMileage(const std::string& number, int mileage){
    sqlite3_stmt* stmt=nullptr;
    sqlite3_prepare_v2(db.raw(), "UPDATE Vehicles SET mileage=? WHERE number=?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt,1,mileage); bindText(stmt,2,number);
    requireDone(db.raw(), stmt);
}

void AutoparkService::deleteCar(const std::string& number){
    sqlite3_stmt* stmt=nullptr;
    sqlite3_prepare_v2(db.raw(), "DELETE FROM Vehicles WHERE number=?", -1, &stmt, nullptr);
    bindText(stmt,1,number);
    requireDone(db.raw(), stmt);
}

void AutoparkService::addDriver(int personnelNo, const std::string& surname, const std::string& category, int experience, const std::string& address, int birthYear){
    sqlite3_stmt* u=nullptr;
    std::string login = surname;
    for(char& ch: login) ch = static_cast<char>(tolower(ch));
    sqlite3_prepare_v2(db.raw(), "INSERT INTO Users(login,password,role) VALUES(?,'1234','driver')", -1, &u, nullptr);
    bindText(u,1,login);
    requireDone(db.raw(), u);
    int userId = static_cast<int>(sqlite3_last_insert_rowid(db.raw()));

    sqlite3_stmt* stmt=nullptr;
    sqlite3_prepare_v2(db.raw(), "INSERT INTO Drivers(id,name,category,experience,address,birth_year,user_id) VALUES(?,?,?,?,?,?,?)", -1,&stmt,nullptr);
    sqlite3_bind_int(stmt,1,personnelNo); bindText(stmt,2,surname); bindText(stmt,3,category); sqlite3_bind_int(stmt,4,experience); bindText(stmt,5,address); sqlite3_bind_int(stmt,6,birthYear); sqlite3_bind_int(stmt,7,userId);
    requireDone(db.raw(), stmt);
}

void AutoparkService::addOrder(const std::string& date, int driverId, int carId, int distanceKm, double cargoKg, double cost){
    sqlite3_stmt* stmt=nullptr;
    sqlite3_prepare_v2(db.raw(), "INSERT INTO Orders(order_date,driver_id,vehicle_id,distance,cargo_weight,cost) VALUES(?,?,?,?,?,?)", -1,&stmt,nullptr);
    bindText(stmt,1,date); sqlite3_bind_int(stmt,2,driverId); sqlite3_bind_int(stmt,3,carId); sqlite3_bind_int(stmt,4,distanceKm); sqlite3_bind_double(stmt,5,cargoKg); sqlite3_bind_double(stmt,6,cost);
    requireDone(db.raw(), stmt);
}

void AutoparkService::calculatePaymentsForPeriod(const std::string& from, const std::string& to){
    sqlite3_stmt* stmt=nullptr;
    const char* sql = R"SQL(
        INSERT INTO Driver_earnings(driver_id, period_start, period_end, total_amount)
        SELECT d.id, ?, ?, COALESCE(SUM(o.cost)*0.20,0)
        FROM Drivers d
        LEFT JOIN Orders o ON o.driver_id=d.id AND o.order_date BETWEEN ? AND ?
        GROUP BY d.id
    )SQL";
    sqlite3_prepare_v2(db.raw(), sql, -1, &stmt, nullptr);
    bindText(stmt,1,from); bindText(stmt,2,to); bindText(stmt,3,from); bindText(stmt,4,to);
    requireDone(db.raw(), stmt);
    std::cout << "Начисления сохранены в таблицу Driver_earnings.\n";
}

double AutoparkService::calculateDriverPayment(const std::string& from, const std::string& to, const std::string& surname){
    sqlite3_stmt* stmt=nullptr;
    const char* sql = "SELECT COALESCE(SUM(o.cost)*0.20,0) FROM Orders o JOIN Drivers d ON d.id=o.driver_id WHERE d.name=? AND o.order_date BETWEEN ? AND ?";
    sqlite3_prepare_v2(db.raw(), sql, -1, &stmt, nullptr);
    bindText(stmt,1,surname); bindText(stmt,2,from); bindText(stmt,3,to);
    double result = 0;
    if(sqlite3_step(stmt)==SQLITE_ROW) result = sqlite3_column_double(stmt,0);
    sqlite3_finalize(stmt);
    return result;
}

double AutoparkService::calculateDriverPaymentById(const std::string& from, const std::string& to, int driverId){
    sqlite3_stmt* stmt=nullptr;
    const char* sql = "SELECT COALESCE(SUM(cost)*0.20,0) FROM Orders WHERE driver_id=? AND order_date BETWEEN ? AND ?";
    sqlite3_prepare_v2(db.raw(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt,1,driverId); bindText(stmt,2,from); bindText(stmt,3,to);
    double result = 0;
    if(sqlite3_step(stmt)==SQLITE_ROW) result = sqlite3_column_double(stmt,0);
    sqlite3_finalize(stmt);
    return result;
}

void AutoparkService::printOrdersByDriver(const std::string& surname, const std::string& from, const std::string& to){
    header("Заказы водителя");
    for(auto&r: db.query("SELECT o.order_date,v.number,o.distance,o.cargo_weight,o.cost FROM Orders o JOIN Drivers d ON d.id=o.driver_id JOIN Vehicles v ON v.id=o.vehicle_id WHERE d.name='"+surname+"' AND o.order_date BETWEEN '"+from+"' AND '"+to+"' ORDER BY o.order_date"))
        std::cout<<r[0]<<" | машина "<<r[1]<<" | "<<r[2]<<" км | "<<r[3]<<" кг | "<<r[4]<<" руб.\n";
}

void AutoparkService::printOrdersByDriverId(int driverId, const std::string& from, const std::string& to){
    header("Мои заказы за период");
    sqlite3_stmt* stmt=nullptr;
    const char* sql = R"SQL(
        SELECT o.order_date, v.number, o.distance, o.cargo_weight, o.cost
        FROM Orders o
        JOIN Vehicles v ON v.id=o.vehicle_id
        WHERE o.driver_id=? AND o.order_date BETWEEN ? AND ?
        ORDER BY o.order_date
    )SQL";
    sqlite3_prepare_v2(db.raw(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt,1,driverId); bindText(stmt,2,from); bindText(stmt,3,to);
    bool found=false;
    while(sqlite3_step(stmt)==SQLITE_ROW){
        found=true;
        std::cout << reinterpret_cast<const char*>(sqlite3_column_text(stmt,0))
                  << " | машина " << reinterpret_cast<const char*>(sqlite3_column_text(stmt,1))
                  << " | " << sqlite3_column_double(stmt,2) << " км"
                  << " | " << sqlite3_column_double(stmt,3) << " кг"
                  << " | " << sqlite3_column_double(stmt,4) << " руб.\n";
    }
    if(!found) std::cout << "За указанный период заказов не найдено.\n";
    sqlite3_finalize(stmt);
}

void AutoparkService::printCarTotals(const std::string& carNumber){
    header("Итоги по машине");
    for(auto&r: db.query("SELECT v.number,v.brand,v.mileage+COALESCE(SUM(o.distance),0),COALESCE(SUM(o.cargo_weight),0) FROM Vehicles v LEFT JOIN Orders o ON o.vehicle_id=v.id WHERE v.number='"+carNumber+"' GROUP BY v.id"))
        std::cout<<"Машина: "<<r[0]<<" "<<r[1]<<", общий пробег: "<<r[2]<<", масса грузов: "<<r[3]<<" кг\n";
}

void AutoparkService::printCurrentDriverCarTotals(int driverId){
    header("Итоги по моей машине");
    sqlite3_stmt* stmt=nullptr;
    const char* sql = R"SQL(
        SELECT v.number, v.brand, v.mileage + COALESCE(SUM(o.distance),0) AS total_mileage,
               COALESCE(SUM(o.cargo_weight),0) AS total_cargo
        FROM Vehicles v
        JOIN Orders o ON o.vehicle_id = v.id
        WHERE o.driver_id = ?
        GROUP BY v.id, v.number, v.brand, v.mileage
        ORDER BY v.number
    )SQL";
    sqlite3_prepare_v2(db.raw(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt,1,driverId);
    bool found=false;
    while(sqlite3_step(stmt)==SQLITE_ROW){
        found=true;
        std::cout << "Машина: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt,0))
                  << " " << reinterpret_cast<const char*>(sqlite3_column_text(stmt,1))
                  << ", общий пробег: " << sqlite3_column_double(stmt,2)
                  << ", масса грузов: " << sqlite3_column_double(stmt,3) << " кг\n";
    }
    if(!found) std::cout << "Для текущего водителя машины/заказы не найдены.\n";
    sqlite3_finalize(stmt);
}

void AutoparkService::printDriverStats(){
    header("Статистика по водителям");
    for(auto&r: db.query("SELECT d.name,COUNT(o.id),COALESCE(SUM(o.cargo_weight),0),COALESCE(SUM(o.cost)*0.20,0) FROM Drivers d LEFT JOIN Orders o ON o.driver_id=d.id GROUP BY d.id ORDER BY d.name"))
        std::cout<<r[0]<<" | поездок: "<<r[1]<<" | груз: "<<r[2]<<" кг | заработано: "<<r[3]<<"\n";
}

void AutoparkService::printLeastTripsDriver(){
    header("Водитель с минимальным числом поездок");
    for(auto&r: db.query("SELECT d.id,d.name,d.category,d.experience,d.address,d.birth_year,COUNT(o.id),COALESCE(SUM(o.cost)*0.20,0) FROM Drivers d LEFT JOIN Orders o ON o.driver_id=d.id GROUP BY d.id ORDER BY COUNT(o.id) ASC LIMIT 1"))
        std::cout<<"№"<<r[0]<<" "<<r[1]<<", кат. "<<r[2]<<", стаж "<<r[3]<<", адрес "<<r[4]<<", год "<<r[5]<<", поездок "<<r[6]<<", денег "<<r[7]<<"\n";
}

void AutoparkService::printTopMileageCar(){
    header("Машина с наибольшим пробегом");
    for(auto&r: db.query("SELECT v.number,v.brand,v.mileage,v.capacity,v.mileage+COALESCE(SUM(o.distance),0) total FROM Vehicles v LEFT JOIN Orders o ON o.vehicle_id=v.id GROUP BY v.id ORDER BY total DESC LIMIT 1"))
        std::cout<<r[0]<<" "<<r[1]<<", пробег при покупке "<<r[2]<<", грузоподъемность "<<r[3]<<", общий пробег "<<r[4]<<"\n";
}

void AutoparkService::printCurrentDriverInfo(int driverId){
    header("Мои данные");
    for(auto&r: db.query("SELECT id,name,category,experience,address,birth_year FROM Drivers WHERE id="+std::to_string(driverId)))
        std::cout<<"№"<<r[0]<<" "<<r[1]<<", категория "<<r[2]<<", стаж "<<r[3]<<", адрес "<<r[4]<<", год рождения "<<r[5]<<"\n";
}
