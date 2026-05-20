#include "ConsoleUI.h"
#include <iostream>
#include <limits>

static std::string input(const std::string& prompt){ std::cout << prompt; std::string s; std::getline(std::cin, s); return s; }
static int inputInt(const std::string& prompt){ return std::stoi(input(prompt)); }
static double inputDouble(const std::string& prompt){ return std::stod(input(prompt)); }

void ConsoleUI::run(){
    service.initialize();
    std::cout << "Автопарк: консольная информационная система\n";
    while(true){
        auto login = input("Логин: ");
        auto pass = input("Пароль: ");
        auto user = Auth::login(db, login, pass);
        if(!user){ std::cout << "Неверный логин или пароль.\n"; continue; }
        if(user->role == "admin") adminMenu(*user); else driverMenu(*user);
        break;
    }
}

void ConsoleUI::adminMenu(const User&){
    while(true){
        std::cout << "\n1. Заказы водителя за период\n2. Итоги по машине\n3. Статистика водителей\n4. Водитель с минимумом поездок\n5. Машина с максимумом пробега\n6. Добавить машину\n7. Добавить водителя\n8. Добавить заказ\n9. Начислить всем водителям за период\n10. Начислить одному водителю\n0. Выход\n> ";
        std::string c; std::getline(std::cin,c);
        try{
            if(c=="0") return;
            if(c=="1") {
                std::string surname = input("Фамилия: ");
                std::string from = input("Дата с YYYY-MM-DD: ");
                std::string to = input("Дата по YYYY-MM-DD: ");
                service.printOrdersByDriver(surname, from, to);
            }
            else if(c=="2") service.printCarTotals(input("Номер машины: "));
            else if(c=="3") service.printDriverStats();
            else if(c=="4") service.printLeastTripsDriver();
            else if(c=="5") service.printTopMileageCar();
            else if(c=="6") service.addCar(input("Номер: "), input("Марка: "), inputInt("Пробег при покупке: "), inputDouble("Грузоподъемность кг: "));
            else if(c=="7") service.addDriver(inputInt("Табельный номер: "), input("Фамилия: "), input("Категория: "), inputInt("Стаж: "), input("Адрес: "), inputInt("Год рождения: "));
            else if(c=="8") service.addOrder(input("Дата: "), inputInt("ID водителя: "), inputInt("ID машины: "), inputInt("Километраж: "), inputDouble("Масса груза кг: "), inputDouble("Стоимость: "));
            else if(c=="9") service.calculatePaymentsForPeriod(input("Дата с: "), input("Дата по: "));
            else if(c=="10") {
                std::string surname = input("Фамилия: ");
                std::string from = input("Дата с: ");
                std::string to = input("Дата по: ");
                std::cout << "Начислено: " << service.calculateDriverPayment(from, to, surname) << "\n";
            }
        }catch(const std::exception& e){ std::cout << "Ошибка: " << e.what() << "\n"; }
    }
}

void ConsoleUI::driverMenu(const User& user){
    while(true){
        std::cout << "\n1. Мои данные\n2. Мои заказы за период\n3. Итоги по моим машинам\n4. Мои начисления за период\n0. Выход\n> ";
        std::string c; std::getline(std::cin,c);
        try{
            if(c=="0") return;
            if(c=="1") service.printCurrentDriverInfo(user.driver_id);
            else if(c=="2") {
                std::string from = input("Дата с: ");
                std::string to = input("Дата по: ");
                service.printOrdersByDriverId(user.driver_id, from, to);
            }
            else if(c=="3") service.printCurrentDriverCarTotals(user.driver_id);
            else if(c=="4") {
                std::string from = input("Дата с: ");
                std::string to = input("Дата по: ");
                std::cout << "Начислено: " << service.calculateDriverPaymentById(from, to, user.driver_id) << "\n";
            }
        }catch(const std::exception& e){ std::cout << "Ошибка: " << e.what() << "\n"; }
    }
}
