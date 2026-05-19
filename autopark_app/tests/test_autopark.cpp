#include "Auth.h"
#include "AutoparkService.h"
#include "Database.h"

#include <cmath>
#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

static int testsRun = 0;
static int testsFailed = 0;

#define ASSERT_TRUE(expr) do { \
    ++testsRun; \
    if (!(expr)) { \
        ++testsFailed; \
        std::cerr << "FAIL: " << __func__ << ": " << #expr \
                  << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
    } \
} while (0)

#define ASSERT_EQ(expected, actual) do { \
    ++testsRun; \
    auto e = (expected); \
    auto a = (actual); \
    if (!(e == a)) { \
        ++testsFailed; \
        std::cerr << "FAIL: " << __func__ << ": expected [" << e \
                  << "], got [" << a << "] at " << __FILE__ << ":" << __LINE__ << "\n"; \
    } \
} while (0)

#define ASSERT_NEAR(expected, actual, eps) do { \
    ++testsRun; \
    double e = (expected); \
    double a = (actual); \
    if (std::fabs(e - a) > (eps)) { \
        ++testsFailed; \
        std::cerr << "FAIL: " << __func__ << ": expected near [" << e \
                  << "], got [" << a << "] at " << __FILE__ << ":" << __LINE__ << "\n"; \
    } \
} while (0)

static std::string dbPath(const std::string& name) {
    return "build/" + name + ".sqlite";
}

static std::unique_ptr<Database> makeDb(const std::string& name) {
    std::string path = dbPath(name);
    std::remove(path.c_str());
    auto db = std::make_unique<Database>(path);
    AutoparkService service(*db);
    service.initialize();
    return db;
}

static void seedBase(AutoparkService& service) {
    service.addCar("A001AA", "GAZ", 10000, 5000.0);
    service.addCar("B002BB", "KAMAZ", 20000, 10000.0);
    service.addDriver(1, "Ivanov", "C", 8, "Moscow", 1985);
    service.addDriver(2, "Petrov", "CE", 12, "Tula", 1980);
    service.addOrder("2026-01-10", 1, 1, 120, 3000.0, 10000.0);
    service.addOrder("2026-01-12", 1, 2, 300, 8000.0, 20000.0);
    service.addOrder("2026-02-01", 2, 2, 150, 5000.0, 15000.0);
}

static void database_exec_and_query_work() {
    auto dbPtr = makeDb("database_exec_and_query_work");
    Database& db = *dbPtr;
    db.exec("INSERT INTO Users(login,password,role) VALUES('admin','admin123','admin')");
    auto rows = db.query("SELECT login, role FROM Users WHERE login='admin'");
    ASSERT_EQ(1U, rows.size());
    ASSERT_EQ(std::string("admin"), rows[0][0]);
    ASSERT_EQ(std::string("admin"), rows[0][1]);
}

static void database_foreign_keys_are_enabled() {
    auto dbPtr = makeDb("database_foreign_keys_are_enabled");
    Database& db = *dbPtr;
    bool thrown = false;
    try {
        db.exec("INSERT INTO Drivers(id,name,user_id) VALUES(77,'Bad',9999)");
    } catch (const std::exception&) {
        thrown = true;
    }
    ASSERT_TRUE(thrown);
}

static void database_reports_sql_errors() {
    auto dbPtr = makeDb("database_reports_sql_errors");
    Database& db = *dbPtr;
    bool thrown = false;
    try {
        db.query("SELECT * FROM NotExistingTable");
    } catch (const std::exception&) {
        thrown = true;
    }
    ASSERT_TRUE(thrown);
}

static void auth_accepts_valid_user() {
    auto dbPtr = makeDb("auth_accepts_valid_user");
    Database& db = *dbPtr;
    db.exec("INSERT INTO Users(login,password,role) VALUES('admin','admin123','admin')");
    auto user = Auth::login(db, "admin", "admin123");
    ASSERT_TRUE(user.has_value());
    ASSERT_EQ(std::string("admin"), user->login);
    ASSERT_EQ(std::string("admin"), user->role);
}

static void auth_rejects_wrong_password() {
    auto dbPtr = makeDb("auth_rejects_wrong_password");
    Database& db = *dbPtr;
    db.exec("INSERT INTO Users(login,password,role) VALUES('admin','admin123','admin')");
    auto user = Auth::login(db, "admin", "wrong");
    ASSERT_TRUE(!user.has_value());
}

static void auth_returns_driver_id_for_driver() {
    auto dbPtr = makeDb("auth_returns_driver_id_for_driver");
    Database& db = *dbPtr;
    AutoparkService service(db);
    service.addDriver(15, "Sidorov", "C", 4, "Kazan", 1993);
    auto user = Auth::login(db, "sidorov", "1234");
    ASSERT_TRUE(user.has_value());
    ASSERT_EQ(std::string("driver"), user->role);
    ASSERT_EQ(15, user->driver_id);
}

static void service_adds_cars_drivers_and_orders() {
    auto dbPtr = makeDb("service_adds_cars_drivers_and_orders");
    Database& db = *dbPtr;
    AutoparkService service(db);
    seedBase(service);
    ASSERT_EQ(2U, db.query("SELECT id FROM Vehicles").size());
    ASSERT_EQ(2U, db.query("SELECT id FROM Drivers").size());
    ASSERT_EQ(3U, db.query("SELECT id FROM Orders").size());
}

static void service_calculates_driver_payment_by_surname() {
    auto dbPtr = makeDb("service_calculates_driver_payment_by_surname");
    Database& db = *dbPtr;
    AutoparkService service(db);
    seedBase(service);
    ASSERT_NEAR(6000.0, service.calculateDriverPayment("2026-01-01", "2026-01-31", "Ivanov"), 0.001);
}

static void service_calculates_driver_payment_by_id() {
    auto dbPtr = makeDb("service_calculates_driver_payment_by_id");
    Database& db = *dbPtr;
    AutoparkService service(db);
    seedBase(service);
    ASSERT_NEAR(3000.0, service.calculateDriverPaymentById("2026-02-01", "2026-02-28", 2), 0.001);
}

static void service_saves_payments_for_period() {
    auto dbPtr = makeDb("service_saves_payments_for_period");
    Database& db = *dbPtr;
    AutoparkService service(db);
    seedBase(service);
    service.calculatePaymentsForPeriod("2026-01-01", "2026-01-31");
    auto rows = db.query("SELECT COUNT(*), SUM(total_amount) FROM Driver_earnings");
    ASSERT_EQ(std::string("2"), rows[0][0]);
    ASSERT_EQ(std::string("6000.0"), rows[0][1]);
}

static void service_rejects_overloaded_order() {
    auto dbPtr = makeDb("service_rejects_overloaded_order");
    Database& db = *dbPtr;
    AutoparkService service(db);
    service.addCar("A001AA", "GAZ", 10000, 1000.0);
    service.addDriver(1, "Ivanov", "C", 8, "Moscow", 1985);
    bool thrown = false;
    try {
        service.addOrder("2026-01-10", 1, 1, 100, 1500.0, 9000.0);
    } catch (const std::exception&) {
        thrown = true;
    }
    ASSERT_TRUE(thrown);
    ASSERT_EQ(0U, db.query("SELECT id FROM Orders").size());
}

static void service_updates_and_deletes_car() {
    auto dbPtr = makeDb("service_updates_and_deletes_car");
    Database& db = *dbPtr;
    AutoparkService service(db);
    service.addCar("A001AA", "GAZ", 10000, 5000.0);
    service.updateCarMileage("A001AA", 12345);
    auto rows = db.query("SELECT mileage FROM Vehicles WHERE number='A001AA'");
    ASSERT_EQ(std::string("12345"), rows[0][0]);
    service.deleteCar("A001AA");
    ASSERT_EQ(0U, db.query("SELECT id FROM Vehicles WHERE number='A001AA'").size());
}

int main() {
    database_exec_and_query_work();
    database_foreign_keys_are_enabled();
    database_reports_sql_errors();

    auth_accepts_valid_user();
    auth_rejects_wrong_password();
    auth_returns_driver_id_for_driver();

    service_adds_cars_drivers_and_orders();
    service_calculates_driver_payment_by_surname();
    service_calculates_driver_payment_by_id();
    service_saves_payments_for_period();
    service_rejects_overloaded_order();
    service_updates_and_deletes_car();

    std::cout << "Tests run: " << testsRun << ", failed: " << testsFailed << "\n";
    return testsFailed == 0 ? 0 : 1;
}
