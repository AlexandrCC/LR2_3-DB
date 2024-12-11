#pragma once
#include <iostream>
#include <view.h>
#include <locale>
#include <pqxx/pqxx> // Library for PostgreSQL operations
#include <string>
#include <cstdlib>
#include <ctime>

class Model {
protected:
    pqxx::connection* conn;

    pqxx::connection* connectToDatabase() {
        try {
            // Establish connection to the database
            conn = new pqxx::connection("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            if (!conn->is_open()) {
                std::cerr << "Error: Unable to open database connection." << std::endl;
                delete conn;
                conn = nullptr;
            }
            return conn;
        }
        catch (const std::exception& e) {
            std::cerr << "Database connection error: " << e.what() << std::endl;
            return nullptr;
        }
    }


public:
    Model(pqxx::connection* connection) : conn(connection) {}

    ~Model() = default;
    int findNextFreeId(pqxx::work& txn, const std::string& table_name) {
        // Query to dynamically get the primary key column name for the specified table
        std::string pk_query = "SELECT column_name FROM information_schema.key_column_usage "
            "WHERE table_name = " + txn.quote(table_name) + " AND constraint_name = "
            "(SELECT constraint_name FROM information_schema.table_constraints "
            "WHERE table_name = " + txn.quote(table_name) + " AND constraint_type = 'PRIMARY KEY');";

        pqxx::result pk_result = txn.exec(pk_query);

        if (pk_result.empty()) {
            throw std::runtime_error("No primary key found for table: " + table_name);
        }

        std::string pk_column = pk_result[0][0].c_str();

        // Query to find the next available ID based on the primary key column
        std::string id_query = "SELECT COALESCE(MAX(" + pk_column + "), 0) + 1 FROM public.\"" + table_name + "\";";
        pqxx::result id_result = txn.exec(id_query);

        return id_result[0][0].as<int>();
    }


    void createMedicine(int medicine_id, int manufacturer_id, int specs_id, int category_id, int treats_id, int contradicts_id, const std::string& medicine_name) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);

            // Insert into Medicines table
            std::string query_medicine =
                "INSERT INTO public.\"Medicines\" (\"Medicine_id\", \"Manufacturer_id\", \"Specs_id\", \"Category_id\", \"Treats_id\", \"Contradicts_id\", \"Medicine_name\") VALUES (" +
                txn.quote(medicine_id) + ", " + txn.quote(manufacturer_id) + ", " + txn.quote(specs_id) + ", " + txn.quote(category_id) + ", " + txn.quote(treats_id) + ", " + txn.quote(contradicts_id) + ", " + txn.quote(medicine_name) + ");";
            txn.exec(query_medicine);

            // Insert into Medicine_Treats_Diseases
            int next_treats_id = findNextFreeId(txn, "Medicine_Treats_Diseases");
            std::string query_treats =
                "INSERT INTO public.\"Medicine_Treats_Diseases\" (tab_id, \"Medicine_id\", \"Disease_id\") VALUES (" +
                txn.quote(next_treats_id) + ", " + txn.quote(medicine_id) + ", " + txn.quote(treats_id) + ");";
            txn.exec(query_treats);

            // Insert into Medicine_Contradicts_Diseases
            int next_contradicts_id = findNextFreeId(txn, "Medicine_Contradicts_Diseases");
            std::string query_contradicts =
                "INSERT INTO public.\"Medicine_Contradicts_Diseases\" (tab_id, \"Medicine_id\", \"Disease_id\") VALUES (" +
                txn.quote(next_contradicts_id) + ", " + txn.quote(medicine_id) + ", " + txn.quote(contradicts_id) + ");";
            txn.exec(query_contradicts);

            // Insert into man_med_tab
            int next_man_med_id = findNextFreeId(txn, "man_med_tab");
            std::string query_man_med =
                "INSERT INTO public.man_med_tab (tab_id, \"Medicine_id\", \"Manufacturer_id\") VALUES (" +
                txn.quote(next_man_med_id) + ", " + txn.quote(medicine_id) + ", " + txn.quote(manufacturer_id) + ");";
            txn.exec(query_man_med);

            txn.commit();

            std::cout << "Medicine and related records created successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }


    void readMedicine(int medicine_id) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            if (!conn.is_open()) {
                std::cerr << "Error: Unable to connect to the database." << std::endl;
                return;
            }

            pqxx::work txn(conn);

            std::string query = "SELECT * FROM public.\"Medicines\" WHERE \"Medicine_id\" = " + txn.quote(medicine_id) + ";";
            std::cout << "Executing query: " << query << std::endl;

            pqxx::result result = txn.exec(query);

            if (result.empty()) {
                std::cout << "No medicine found with Medicine_id: " << medicine_id << std::endl;
                return;
            }

            // Access columns by index in the result set
            for (const auto& row : result) {
                std::cout << "Medicine ID: " << row[0].as<int>() << std::endl;        // "Medicine_id"
                std::cout << "Manufacturer ID: " << row[1].as<int>() << std::endl;     // "Manufacturer_id"
                std::cout << "Specs ID: " << row[2].as<int>() << std::endl;            // "Specs_id"
                std::cout << "Category ID: " << row[3].as<int>() << std::endl;         // "Category_id"
                std::cout << "Treats ID: " << row[4].as<int>() << std::endl;           // "Treats_id"
                std::cout << "Contradicts ID: " << row[5].as<int>() << std::endl;      // "Contradicts_id"
                std::cout << "Medicine Name: " << row[6].c_str() << std::endl;         // "Medicine_name"
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading medicine: " << e.what() << std::endl;
        }
    }


    void updateMedicine(int medicine_id, const std::string& column_name, const std::string& new_value) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);

            // Update the Medicines table
            std::string query = "UPDATE public.\"Medicines\" SET \"" + column_name + "\" = " + txn.quote(new_value) +
                " WHERE \"Medicine_id\" = " + txn.quote(medicine_id) + ";";
            txn.exec(query);

            // If the column is Treats_id, update Medicine_Treats_Diseases
            if (column_name == "Treats_id") {
                std::string query_update_treats =
                    "UPDATE public.\"Medicine_Treats_Diseases\" SET \"Disease_id\" = " + txn.quote(new_value) +
                    " WHERE \"Medicine_id\" = " + txn.quote(medicine_id) + ";";
                txn.exec(query_update_treats);
            }

            // If the column is Contradicts_id, update Medicine_Contradicts_Diseases
            if (column_name == "Contradicts_id") {
                std::string query_update_contradicts =
                    "UPDATE public.\"Medicine_Contradicts_Diseases\" SET \"Disease_id\" = " + txn.quote(new_value) +
                    " WHERE \"Medicine_id\" = " + txn.quote(medicine_id) + ";";
                txn.exec(query_update_contradicts);
            }

            // If the column is Manufacturer_id, update man_med_tab
            if (column_name == "Manufacturer_id") {
                std::string query_update_man_med =
                    "UPDATE public.man_med_tab SET \"Manufacturer_id\" = " + txn.quote(new_value) +
                    " WHERE \"Medicine_id\" = " + txn.quote(medicine_id) + ";";
                txn.exec(query_update_man_med);
            }

            txn.commit();

            std::cout << "Medicine and related records updated successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }



    void deleteMedicine(int medicine_id) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);

            // Delete from Medicine_Treats_Diseases
            std::string query_delete_treats = "DELETE FROM public.\"Medicine_Treats_Diseases\" WHERE \"Medicine_id\" = " + txn.quote(medicine_id) + ";";
            txn.exec(query_delete_treats);

            // Delete from Medicine_Contradicts_Diseases
            std::string query_delete_contradicts = "DELETE FROM public.\"Medicine_Contradicts_Diseases\" WHERE \"Medicine_id\" = " + txn.quote(medicine_id) + ";";
            txn.exec(query_delete_contradicts);

            // Delete from man_med_tab
            std::string query_delete_man_med = "DELETE FROM public.man_med_tab WHERE \"Medicine_id\" = " + txn.quote(medicine_id) + ";";
            txn.exec(query_delete_man_med);

            // Delete from Medicines table
            std::string query_delete_medicine = "DELETE FROM public.\"Medicines\" WHERE \"Medicine_id\" = " + txn.quote(medicine_id) + ";";
            txn.exec(query_delete_medicine);

            txn.commit();

            std::cout << "Medicine and related records deleted successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }


    //SPECS
    void createSpec(int specs_id, const std::string& license_name, bool term_of_issue, const std::string& medicine_form) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);

            std::string query = "INSERT INTO public.\"Specs\" (\"specs_id\", \"Lisence_name\", \"Term_of_issue\", \"Medicine_form\") VALUES (" +
                txn.quote(specs_id) + ", " + txn.quote(license_name) + ", " + txn.quote(term_of_issue) + ", " + txn.quote(medicine_form) + ");";

            txn.exec(query);
            txn.commit();

            std::cout << "Spec created successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void readSpec(int specs_id) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            if (!conn.is_open()) {
                std::cerr << "Error: Unable to connect to the database." << std::endl;
                return;
            }

            pqxx::work txn(conn);

            std::string query = "SELECT * FROM public.\"Specs\" WHERE \"specs_id\" = " + txn.quote(specs_id) + ";";
            std::cout << "Executing query: " << query << std::endl;

            pqxx::result result = txn.exec(query);

            if (result.empty()) {
                std::cout << "No spec found with Specs_id: " << specs_id << std::endl;
                return;
            }

            // Access columns by index if exact names cause issues
            for (const auto& row : result) {
                std::cout << "Specs ID: " << row[0].as<int>() << std::endl;
                std::cout << "License Name: " << row[1].c_str() << std::endl;
                std::cout << "Term of Issue: " << (row[2].as<bool>() ? "True" : "False") << std::endl;
                std::cout << "Medicine Form: " << row[3].c_str() << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading spec: " << e.what() << std::endl;
        }
    }

    void updateSpec(int specs_id, const std::string& column_name, const std::string& new_value) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);

            std::string query = "UPDATE public.\"Specs\" SET \"" + column_name + "\" = " + txn.quote(new_value) +
                " WHERE \"specs_id\" = " + txn.quote(specs_id) + ";";
            txn.exec(query);
            txn.commit();

            std::cout << "Spec updated successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }


    void createCategory(int category_id, const std::string& category_name) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);
            std::string query = "INSERT INTO public.\"Categories\" (\"Category_ID\", \"Category_Name\") VALUES (" +
                txn.quote(category_id) + ", " + txn.quote(category_name) + ");";
            txn.exec(query);
            txn.commit();
            std::cout << "Category created successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void readCategory(int category_id) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);
            std::string query = "SELECT * FROM public.\"Categories\" WHERE \"Category_ID\" = " + txn.quote(category_id) + ";";
            pqxx::result result = txn.exec(query);
            if (result.empty()) {
                std::cout << "No category found with Category_ID: " << category_id << std::endl;
                return;
            }
            for (const auto& row : result) {
                std::cout << "Category ID: " << row[0].as<int>() << std::endl;
                std::cout << "Category Name: " << row[1].c_str() << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading category: " << e.what() << std::endl;
        }
    }

    void deleteSpec(int specs_id) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);

            std::string query = "DELETE FROM public.\"Specs\" WHERE \"specs_id\" = " + txn.quote(specs_id) + ";";
            txn.exec(query);
            txn.commit();

            std::cout << "Spec deleted successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error deleting spec: " << e.what() << std::endl;
        }
    }


    void updateCategory(int category_id, const std::string& new_name) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);
            std::string query = "UPDATE public.\"Categories\" SET \"Category_Name\" = " + txn.quote(new_name) +
                " WHERE \"Category_ID\" = " + txn.quote(category_id) + ";";
            txn.exec(query);
            txn.commit();
            std::cout << "Category updated successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void deleteCategory(int category_id) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);
            std::string query = "DELETE FROM public.\"Categories\" WHERE \"Category_ID\" = " + txn.quote(category_id) + ";";
            txn.exec(query);
            txn.commit();
            std::cout << "Category deleted successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void createDisease(int disease_id, const std::string& disease_name) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);
            std::string query = "INSERT INTO public.\"Diseases\" (\"Disease_id\", \"Disease_name\") VALUES (" +
                txn.quote(disease_id) + ", " + txn.quote(disease_name) + ");";
            txn.exec(query);
            txn.commit();
            std::cout << "Disease created successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void readDisease(int disease_id) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);
            std::string query = "SELECT * FROM public.\"Diseases\" WHERE \"Disease_id\" = " + txn.quote(disease_id) + ";";
            pqxx::result result = txn.exec(query);
            if (result.empty()) {
                std::cout << "No disease found with Disease_id: " << disease_id << std::endl;
                return;
            }
            for (const auto& row : result) {
                std::cout << "Disease ID: " << row[0].as<int>() << std::endl;
                std::cout << "Disease Name: " << row[1].c_str() << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading disease: " << e.what() << std::endl;
        }
    }

    void updateDisease(int disease_id, const std::string& new_name) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);
            std::string query = "UPDATE public.\"Diseases\" SET \"Disease_name\" = " + txn.quote(new_name) +
                " WHERE \"Disease_id\" = " + txn.quote(disease_id) + ";";
            txn.exec(query);
            txn.commit();
            std::cout << "Disease updated successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void deleteDisease(int disease_id) {
        try {
            pqxx::connection conn("dbname=postgres user=postgres password=1 host=127.0.0.1 port=5432");
            pqxx::work txn(conn);
            std::string query = "DELETE FROM public.\"Diseases\" WHERE \"Disease_id\" = " + txn.quote(disease_id) + ";";
            txn.exec(query);
            txn.commit();
            std::cout << "Disease deleted successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void createManufacturer(int manufacturer_id, const std::string& manufacturer_name, const std::string& location) {
        if (!connectToDatabase()) return;
        try {
            pqxx::work txn(*conn);

            std::string query = "INSERT INTO public.\"Manufactures\" (\"Manufacturer_id\", \"Manufacturer_name\", \"Location of facilities\") "
                "VALUES (" + txn.quote(manufacturer_id) + ", " + txn.quote(manufacturer_name) + ", " + txn.quote(location) + ");";
            txn.exec(query);
            txn.commit();

            std::cout << "Manufacturer created successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error creating manufacturer: " << e.what() << std::endl;
        }
    }

    // Read a manufacturer along with its related medicines
    void readManufacturer(int manufacturer_id) {
        if (!connectToDatabase()) return;
        try {
            pqxx::work txn(*conn);

            // Corrected query to use the "Manufactures" table
            std::string query = "SELECT * FROM public.\"Manufactures\" WHERE \"Manufacturer_id\" = " + txn.quote(manufacturer_id) + ";";
            pqxx::result result = txn.exec(query);

            if (result.empty()) {
                std::cout << "No manufacturer found with Manufacturer_id: " << manufacturer_id << std::endl;
                return;
            }

            // Display manufacturer information
            for (const auto& row : result) {
                std::cout << "Manufacturer ID: " << row[0].as<int>() << "\n"
                    << "Manufacturer Name: " << row[1].c_str() << "\n"
                    << "Location of facilities: " << row[2].c_str() << "\n"
                    << "-----------------------------------" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading manufacturer: " << e.what() << std::endl;
        }
    }



    void updateManufacturer(int manufacturer_id, const std::string& column_name, const std::string& new_value) {
        if (!connectToDatabase()) return;
        try {
            pqxx::work txn(*conn);

            std::string query = "UPDATE public.\"Manufactures\" SET \"" + column_name + "\" = " + txn.quote(new_value) +
                " WHERE \"Manufacturer_id\" = " + txn.quote(manufacturer_id) + ";";
            txn.exec(query);
            txn.commit();

            std::cout << "Manufacturer updated successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error updating manufacturer: " << e.what() << std::endl;
        }
    }

    void deleteManufacturer(int manufacturer_id) {
        if (!connectToDatabase()) return;
        try {
            pqxx::work txn(*conn);

            // Delete related entries from the junction table first
            std::string delete_junction = "DELETE FROM public.man_med_tab WHERE \"Manufacturer_id\" = " + txn.quote(manufacturer_id) + ";";
            txn.exec(delete_junction);

            // Delete from Manufacturers table
            std::string delete_manufacturer = "DELETE FROM public.\"Manufactures\" WHERE \"Manufacturer_id\" = " + txn.quote(manufacturer_id) + ";";
            txn.exec(delete_manufacturer);

            txn.commit();
            std::cout << "Manufacturer and related records deleted successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error deleting manufacturer: " << e.what() << std::endl;
        }
    }


    void generateData(int count, int choice) {

        if (choice == 1) {
            try {
                pqxx::work txn(*conn);
                srand(static_cast<unsigned>(time(0)));  // Seed for random number generation

                for (int i = 0; i < count; ++i) {
                    // Generate the next available Medicine ID
                    int medicine_id = txn.exec("SELECT COALESCE(MAX(\"Medicine_id\"), 0) + 1 FROM public.\"Medicines\";")[0][0].as<int>();

                    // Generate unique Specs data and insert into the Specs table
                    int specs_id = txn.exec("SELECT COALESCE(MAX(\"specs_id\"), 0) + 1 FROM public.\"Specs\";")[0][0].as<int>();
                    std::string license_name = "License_" + std::to_string(rand() % 1000);
                    bool term_of_issue = rand() % 2 == 0;
                    std::string medicine_form = (rand() % 2 == 0) ? "Tablet" : "Capsule";

                    // Insert the new Specs entry
                    std::string insert_specs_query =
                        "INSERT INTO public.\"Specs\" (\"specs_id\", \"Lisence_name\", \"Term_of_issue\", \"Medicine_form\") VALUES (" +
                        txn.quote(specs_id) + ", " + txn.quote(license_name) + ", " + (term_of_issue ? "TRUE" : "FALSE") + ", " + txn.quote(medicine_form) + ");";
                    txn.exec(insert_specs_query);

                    // Randomly select existing Manufacturer, Category, and Disease IDs to use as foreign keys
                    int manufacturer_id = txn.exec("SELECT \"Manufacturer_id\" FROM public.\"Manufactures\" ORDER BY RANDOM() LIMIT 1;")[0][0].as<int>();
                    int category_id = txn.exec("SELECT \"Category_ID\" FROM public.\"Categories\" ORDER BY RANDOM() LIMIT 1;")[0][0].as<int>();
                    int treats_id = txn.exec("SELECT \"Disease_id\" FROM public.\"Diseases\" ORDER BY RANDOM() LIMIT 1;")[0][0].as<int>();
                    int contradicts_id = txn.exec("SELECT \"Disease_id\" FROM public.\"Diseases\" ORDER BY RANDOM() LIMIT 1;")[0][0].as<int>();

                    // Generate a random name for the medicine
                    std::string medicine_name = "Medicine_" + std::to_string(rand() % 1000);

                    // Insert into Medicines table
                    std::string insert_medicine_query =
                        "INSERT INTO public.\"Medicines\" (\"Medicine_id\", \"Manufacturer_id\", \"Specs_id\", \"Category_id\", \"Treats_id\", \"Contradicts_id\", \"Medicine_name\") VALUES (" +
                        txn.quote(medicine_id) + ", " + txn.quote(manufacturer_id) + ", " + txn.quote(specs_id) + ", " +
                        txn.quote(category_id) + ", " + txn.quote(treats_id) + ", " + txn.quote(contradicts_id) + ", " + txn.quote(medicine_name) + ");";
                    txn.exec(insert_medicine_query);

                    // Insert into Medicine_Contradicts_Diseases junction table
                    int contradicts_tab_id = txn.exec("SELECT COALESCE(MAX(tab_id), 0) + 1 FROM public.\"Medicine_Contradicts_Diseases\";")[0][0].as<int>();
                    std::string insert_contradicts_query =
                        "INSERT INTO public.\"Medicine_Contradicts_Diseases\" (tab_id, \"Medicine_id\", \"Disease_id\") VALUES (" +
                        txn.quote(contradicts_tab_id) + ", " + txn.quote(medicine_id) + ", " + txn.quote(contradicts_id) + ");";
                    txn.exec(insert_contradicts_query);

                    // Insert into Medicine_Treats_Diseases junction table
                    int treats_tab_id = txn.exec("SELECT COALESCE(MAX(tab_id), 0) + 1 FROM public.\"Medicine_Treats_Diseases\";")[0][0].as<int>();
                    std::string insert_treats_query =
                        "INSERT INTO public.\"Medicine_Treats_Diseases\" (tab_id, \"Medicine_id\", \"Disease_id\") VALUES (" +
                        txn.quote(treats_tab_id) + ", " + txn.quote(medicine_id) + ", " + txn.quote(treats_id) + ");";
                    txn.exec(insert_treats_query);

                    // Insert into man_med_tab junction table
                    int man_med_tab_id = txn.exec("SELECT COALESCE(MAX(tab_id), 0) + 1 FROM public.\"man_med_tab\";")[0][0].as<int>();
                    std::string insert_man_med_query =
                        "INSERT INTO public.\"man_med_tab\" (tab_id, \"Medicine_id\", \"Manufacturer_id\") VALUES (" +
                        txn.quote(man_med_tab_id) + ", " + txn.quote(medicine_id) + ", " + txn.quote(manufacturer_id) + ");";
                    txn.exec(insert_man_med_query);
                }

                // Commit the transaction after inserting all rows
                txn.commit();
                std::cout << "Random data generated successfully for the Medicines table and associated junction tables." << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error generating random data for Medicines: " << e.what() << std::endl;
            }
        }
        else if (choice == 2) {
            try {
                pqxx::work txn(*conn);
                srand(static_cast<unsigned>(time(0)));  // Seed for random number generation

                for (int i = 0; i < count; ++i) {
                    // Generate random values for each column
                    int specs_id = txn.exec("SELECT COALESCE(MAX(\"specs_id\"), 0) + 1 FROM public.\"Specs\";")[0][0].as<int>();  // Get the next available ID
                    std::string license_name = "License_" + std::to_string(rand() % 1000);  // Random license name
                    bool term_of_issue = rand() % 2 == 0;  // Random boolean
                    std::string medicine_form = (rand() % 2 == 0) ? "Tablet" : "Capsule";  // Random medicine form

                    // Construct the SQL insert query
                    std::string insert_query =
                        "INSERT INTO public.\"Specs\" (\"specs_id\", \"Lisence_name\", \"Term_of_issue\", \"Medicine_form\") VALUES (" +
                        txn.quote(specs_id) + ", " + txn.quote(license_name) + ", " + (term_of_issue ? "TRUE" : "FALSE") + ", " + txn.quote(medicine_form) + ");";

                    // Execute the insert query
                    txn.exec(insert_query);
                }

                // Commit the transaction after inserting all rows
                txn.commit();
                std::cout << "Random data generated successfully for the Specs table." << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error generating random data for Specs: " << e.what() << std::endl;
            }
        }
        else if (choice == 3) {
            try {
                pqxx::work txn(*conn);
                srand(static_cast<unsigned>(time(0)));  // Seed for random number generation

                for (int i = 0; i < count; ++i) {
                    // Generate random values for each column
                    int category_id = txn.exec("SELECT COALESCE(MAX(\"Category_ID\"), 0) + 1 FROM public.\"Categories\";")[0][0].as<int>();  // Get the next available ID
                    std::string category_name = "Category_" + std::to_string(rand() % 1000);  // Random category name

                    // Construct the SQL insert query
                    std::string insert_query =
                        "INSERT INTO public.\"Categories\" (\"Category_ID\", \"Category_Name\") VALUES (" +
                        txn.quote(category_id) + ", " + txn.quote(category_name) + ");";

                    // Execute the insert query
                    txn.exec(insert_query);
                }

                // Commit the transaction after inserting all rows
                txn.commit();
                std::cout << "Random data generated successfully for the Categories table." << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error generating random data for Categories: " << e.what() << std::endl;
            }
        }
        else if (choice == 4) {
            try {
                pqxx::work txn(*conn);
                srand(static_cast<unsigned>(time(0)));  // Seed for random number generation

                for (int i = 0; i < count; ++i) {
                    // Generate random values for each column
                    int disease_id = txn.exec("SELECT COALESCE(MAX(\"Disease_id\"), 0) + 1 FROM public.\"Diseases\";")[0][0].as<int>();  // Get the next available ID
                    std::string disease_name = "Disease_" + std::to_string(rand() % 1000);  // Random disease name

                    // Construct the SQL insert query
                    std::string insert_query =
                        "INSERT INTO public.\"Diseases\" (\"Disease_id\", \"Disease_name\") VALUES (" +
                        txn.quote(disease_id) + ", " + txn.quote(disease_name) + ");";

                    // Execute the insert query
                    txn.exec(insert_query);
                }

                // Commit the transaction after inserting all rows
                txn.commit();
                std::cout << "Random data generated successfully for the Diseases table." << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error generating random data for Diseases: " << e.what() << std::endl;
            }
        }
        else if (choice == 5) {
            try {
                pqxx::work txn(*conn);
                srand(static_cast<unsigned>(time(0)));  // Seed for random number generation

                for (int i = 0; i < count; ++i) {
                    // Generate random values for each column
                    int manufacturer_id = txn.exec("SELECT COALESCE(MAX(\"Manufacturer_id\"), 0) + 1 FROM public.\"Manufactures\";")[0][0].as<int>();  // Get the next available ID
                    std::string manufacturer_name = "Manufacturer_" + std::to_string(rand() % 1000);  // Random manufacturer name
                    std::string location = "Location_" + std::to_string(rand() % 100);  // Random location name

                    // Construct the SQL insert query
                    std::string insert_query =
                        "INSERT INTO public.\"Manufactures\" (\"Manufacturer_id\", \"Manufacturer_name\", \"Location of facilities\") VALUES (" +
                        txn.quote(manufacturer_id) + ", " + txn.quote(manufacturer_name) + ", " + txn.quote(location) + ");";

                    // Execute the insert query
                    txn.exec(insert_query);
                }

                // Commit the transaction after inserting all rows
                txn.commit();
                std::cout << "Random data generated successfully for the Manufacturers table." << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error generating random data for Manufacturers: " << e.what() << std::endl;
            }
        }
    }


    void outputAllTableNames() {
        try {
            pqxx::work txn(*conn);

            // Query to get all table names in the 'public' schema
            std::string query = "SELECT table_name FROM information_schema.tables WHERE table_schema = 'public';";
            pqxx::result result = txn.exec(query);

            std::cout << "Tables in the database:" << std::endl;
            for (const auto& row : result) {
                std::string table_name = row["table_name"].c_str();
                std::cout << "- " << table_name << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving table names: " << e.what() << std::endl;
        }
    }


    void outputTableColumns(const std::string& table_name) {
        try {
            pqxx::work txn(*conn);

            // Query to get all column names for the specified table in the 'public' schema
            std::string query = "SELECT column_name FROM information_schema.columns WHERE table_schema = 'public' AND table_name = " + txn.quote(table_name) + ";";
            pqxx::result result = txn.exec(query);

            // Check if the table has any columns
            if (result.empty()) {
                std::cout << "No columns found for table: " << table_name << " (or the table does not exist)." << std::endl;
                return;
            }

            std::cout << "Columns in table '" << table_name << "':" << std::endl;
            for (const auto& row : result) {
                std::string column_name = row["column_name"].c_str();
                std::cout << "- " << column_name << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving columns for table '" << table_name << "': " << e.what() << std::endl;
        }
    }


    void outputMostCommonManufacturer() {
        if (!connectToDatabase()) return;  // Ensure the database connection is established
        try {
            pqxx::work txn(*conn);

            // Query to find the most common Manufacturer_id in the man_med_tab table
            std::string query =
                "SELECT \"Manufacturer_id\", COUNT(*) AS frequency "
                "FROM public.\"man_med_tab\" "
                "GROUP BY \"Manufacturer_id\" "
                "ORDER BY frequency DESC "
                "LIMIT 1;";

            pqxx::result result = txn.exec(query);

            if (result.empty()) {
                std::cout << "No manufacturers found in the man_med_tab table." << std::endl;
                return;
            }

            // Extract the most common Manufacturer_id and its frequency
            // Extract the most common Manufacturer_id and its frequency
            int manufacturer_id = result[0][0].as<int>();  // Assuming Manufacturer_id is in the first column
            int frequency = result[0][1].as<int>();        // Assuming frequency is in the second column

            std::cout << "Most common manufacturer in the table:" << std::endl;
            std::cout << "- Manufacturer ID: " << manufacturer_id << std::endl;
            std::cout << "- Occurrences: " << frequency << std::endl;

            // Now, use the manufacturer_id to fetch detailed information
            std::string manufacturerQuery =
                "SELECT * FROM public.\"Manufactures\" WHERE \"Manufacturer_id\" = " + txn.quote(manufacturer_id) + ";";
            pqxx::result manufacturerResult = txn.exec(manufacturerQuery);

            if (manufacturerResult.empty()) {
                std::cout << "No details found for Manufacturer_id: " << manufacturer_id << std::endl;
                return;
            }


            // Display detailed manufacturer information
            const auto& row = manufacturerResult[0];
            std::cout << "- Manufacturer Name: " << row[1].c_str() << std::endl;
            std::cout << "- Location: " << row[2].c_str() << std::endl;


        }
        catch (const pqxx::sql_error& e) {
            std::cerr << "SQL error: " << e.what() << std::endl;
            std::cerr << "Query was: " << e.query() << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving most common manufacturer: " << e.what() << std::endl;
        }
    }


    void outputMostCommonTreat() {
        try {
            pqxx::work txn(*conn);

            // Query to find the most common Disease_id in the Medicine_Treats_Diseases table
            std::string query =
                "SELECT d.\"Disease_id\", dis.\"Disease_name\", COUNT(*) AS frequency "
                "FROM public.\"Medicine_Treats_Diseases\" d "
                "JOIN public.\"Diseases\" dis ON d.\"Disease_id\" = dis.\"Disease_id\" "
                "GROUP BY d.\"Disease_id\", dis.\"Disease_name\" "
                "ORDER BY frequency DESC "
                "LIMIT 1;";

            pqxx::result result = txn.exec(query);

            if (result.empty()) {
                std::cout << "No diseases found in the Medicine_Treats_Diseases table." << std::endl;
                return;
            }

            // Extract the most common Disease_id and its frequency
            int disease_id = result[0][0].as<int>();         // Disease_id
            std::string disease_name = result[0][1].c_str(); // Disease_name
            int frequency = result[0][2].as<int>();          // frequency

            // Output the most common disease
            std::cout << "Most common Treated disease:" << std::endl;
            std::cout << "- Disease ID: " << disease_id << std::endl;
            std::cout << "- Disease Name: " << disease_name << std::endl;
            std::cout << "- Occurrences: " << frequency << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving most common disease: " << e.what() << std::endl;
        }
    }


    void outputMostCommonContradicted() {
        try {
            pqxx::work txn(*conn);

            // Query to find the most common Disease_id in the Medicine_Contradicts_Diseases table
            std::string query =
                "SELECT c.\"Disease_id\", dis.\"Disease_name\", COUNT(*) AS frequency "
                "FROM public.\"Medicine_Contradicts_Diseases\" c "
                "JOIN public.\"Diseases\" dis ON c.\"Disease_id\" = dis.\"Disease_id\" "
                "GROUP BY c.\"Disease_id\", dis.\"Disease_name\" "
                "ORDER BY frequency DESC "
                "LIMIT 1;";

            pqxx::result result = txn.exec(query);

            if (result.empty()) {
                std::cout << "No diseases found in the Medicine_Contradicts_Diseases table." << std::endl;
                return;
            }

            // Extract the most common Disease_id and its frequency
            int disease_id = result[0][0].as<int>();         // Disease_id
            std::string disease_name = result[0][1].c_str(); // Disease_name
            int frequency = result[0][2].as<int>();          // frequency

            // Output the most common contradicted disease
            std::cout << "Most common contradicted disease:" << std::endl;
            std::cout << "- Disease ID: " << disease_id << std::endl;
            std::cout << "- Disease Name: " << disease_name << std::endl;
            std::cout << "- Occurrences: " << frequency << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving most common contradicted disease: " << e.what() << std::endl;
        }
    }

};