#include <iostream>
#include <locale>
#include <pqxx/pqxx> // Library for PostgreSQL operations
#include <string>
#include <cstdlib>
#include <ctime>
#include "MODL.cpp"
#include "CONTRL.cpp"











int main() {
    std::locale::global(std::locale("en_US.UTF-8"));
    std::wcout.imbue(std::locale()); // For wide characters, if needed

    pqxx::connection* conn = nullptr;

    try {
        // Attempt to establish a connection to the database
        conn = new pqxx::connection("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
        if (!conn->is_open()) {
            std::cerr << "Error: Unable to open database connection." << std::endl;
            delete conn;
            return 1; // Exit the program with an error code
        }
        std::cerr << "Connection to database retrived" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Database connection error: " << e.what() << std::endl;
        return 1; // Exit the program with an error code
    }


    // Instantiate the Model class with the database connection
    Model model(conn);

    View view;
    view.showMenu();
    view.askColName("testtable");

    // Create a new medicine
    model.createMedicine(1, 122, 201, 301, 401, 501, "Aspirin");
    model.createMedicine(2, 122, 202, 301, 401, 501, "Bebra");

    // Read a medicine
    model.readMedicine(2);

    // Update the medicine name
    model.updateMedicine(1, "Medicine_name", "New Medicine Name");

    // Update the Manufacturer_id for the medicine
    model.updateMedicine(1, "Manufacturer_id", "111");

    // Update the Category_id column for the same Medicine_id
    model.updateMedicine(1, "Category_id", "2");

    // Create a spec
    model.createSpec(1, "General License", true, "Tablet");

    // Read a spec
    model.readSpec(1);

    // Create a category
    model.createCategory(1, "Analgesic");

    // Read a category
    model.readCategory(1);

    // Update the category name
    model.updateCategory(1, "Anti-inflammatory");

    // Create a disease
    model.createDisease(1, "Flu");

    // Read a disease
    model.readDisease(1);

    // Update the disease name
    model.updateDisease(1, "Common Cold");

    model.updateDisease(1, "unCommon Cold");

    model.deleteDisease(401);\

    model.readManufacturer(111);
    model.readSpec(2);
    Controller con(&model, &view);
    con.run();

    // Clean up the database connection
    delete conn;

    return 0;
}
