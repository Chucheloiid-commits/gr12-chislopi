#include "Database.h"
#include "ConsoleUI.h"
#include <iostream>

int main(){
    try{
        Database db("data/carpark.db");
        ConsoleUI ui(db);
        ui.run();
    }catch(const std::exception& e){
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
