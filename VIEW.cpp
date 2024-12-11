#include <iostream>
#include <locale>
#include <pqxx/pqxx> // Library for PostgreSQL operations
#include <string>
#include <cstdlib>
#include <ctime>
#pragma once
class View {
public:
    View() {}
    ~View() = default;
    void showMessage(const std::string& message) {
        std::cout << message << std::endl;
    }

    void showError(const std::string& errorMessage) {
        std::cerr << "Error: " << errorMessage << std::endl;
    }

    void showMenu()
    {
        std::cerr << "\nChoose opitions:\n " << std::endl;
        std::cerr << "1 Add to tables " << std::endl;
        std::cerr << "2 Update tables " << std::endl;
        std::cerr << "3 Delete from tables " << std::endl;
        std::cerr << "4 Generate Data " << std::endl;
        std::cerr << "5 Exit program " << std::endl;
        std::cerr << "6 Out All tables names " << std::endl;
        std::cerr << "7 Out table col names " << std::endl;
        std::cerr << "8 Out most common man " << std::endl;
        std::cerr << "9 Out most Treated desiease " << std::endl;
        std::cerr << "10 Out most Contradicted desease desiease " << std::endl;
    }

    void askTableName()
    {
        std::cerr << "\nEnter valid table name: " << std::endl;
    }

    void askColName(const std::string& message)
    {
        std::cerr << "\nEnter valid coulumn name in table " << message << ":" << std::endl;
    }

};