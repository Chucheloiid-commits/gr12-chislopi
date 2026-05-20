#pragma once
#include "Database.h"
#include <string>

class AutoparkService {
    Database& db;
public:
    explicit AutoparkService(Database& database) : db(database) {}
    void initialize();
    void addCar(const std::string& number, const std::string& brand, int initialMileage, double capacityKg);
    void updateCarMileage(const std::string& number, int mileage);
    void deleteCar(const std::string& number);
    void addDriver(int personnelNo, const std::string& surname, const std::string& category, int experience, const std::string& address, int birthYear);
    void addOrder(const std::string& date, int driverId, int carId, int distanceKm, double cargoKg, double cost);
    void calculatePaymentsForPeriod(const std::string& from, const std::string& to);
    double calculateDriverPayment(const std::string& from, const std::string& to, const std::string& surname);
    double calculateDriverPaymentById(const std::string& from, const std::string& to, int driverId);
    void printOrdersByDriver(const std::string& surname, const std::string& from, const std::string& to);
    void printOrdersByDriverId(int driverId, const std::string& from, const std::string& to);
    void printCarTotals(const std::string& carNumber);
    void printCurrentDriverCarTotals(int driverId);
    void printDriverStats();
    void printLeastTripsDriver();
    void printTopMileageCar();
    void printCurrentDriverInfo(int driverId);
};
