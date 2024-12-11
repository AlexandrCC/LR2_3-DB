#pragma once
#include <iostream>
#include "MODL.cpp"
#include "VIEW.cpp"
#include <locale>
#include <pqxx/pqxx> // Library for PostgreSQL operations
#include <string>
#include <cstdlib>
#include <ctime>

class Controller {
private:
    Model* model;
    View* view;
    bool isRunning;

    void addData() {
        int choice;
        view->showMessage("Choose table to add data:");
        view->showMessage("1 - Medicine, 2 - Spec, 3 - Category, 4 - Disease, 5 - Manufacturer");
        std::cin >> choice;

        if (choice == 1) {
            int medicine_id, manufacturer_id, specs_id, category_id, treats_id, contradicts_id;
            std::string medicine_name;

            view->showMessage("Enter Medicine ID:");
            std::cin >> medicine_id;
            view->showMessage("Enter Manufacturer ID:");
            std::cin >> manufacturer_id;
            view->showMessage("Enter Specs ID:");
            std::cin >> specs_id;
            view->showMessage("Enter Category ID:");
            std::cin >> category_id;
            view->showMessage("Enter Treats ID:");
            std::cin >> treats_id;
            view->showMessage("Enter Contradicts ID:");
            std::cin >> contradicts_id;
            view->showMessage("Enter Medicine Name:");
            std::cin >> medicine_name;

            model->createMedicine(medicine_id, manufacturer_id, specs_id, category_id, treats_id, contradicts_id, medicine_name);
        }
        else if (choice == 2) {
            int specs_id;
            std::string license_name, medicine_form;
            bool term_of_issue;

            view->showMessage("Enter Specs ID:");
            std::cin >> specs_id;
            view->showMessage("Enter License Name:");
            std::cin >> license_name;
            view->showMessage("Enter Term of Issue (1 for true, 0 for false):");
            std::cin >> term_of_issue;
            view->showMessage("Enter Medicine Form:");
            std::cin >> medicine_form;

            model->createSpec(specs_id, license_name, term_of_issue, medicine_form);
        }
        else if (choice == 3) {
            int category_id;
            std::string category_name;

            view->showMessage("Enter Category ID:");
            std::cin >> category_id;
            view->showMessage("Enter Category Name:");
            std::cin >> category_name;

            model->createCategory(category_id, category_name);
        }
        else if (choice == 4) {
            int disease_id;
            std::string disease_name;

            view->showMessage("Enter Disease ID:");
            std::cin >> disease_id;
            view->showMessage("Enter Disease Name:");
            std::cin >> disease_name;

            model->createDisease(disease_id, disease_name);
        }
        else if (choice == 5) {
            int manufacturer_id;
            std::string manufacturer_name, location;

            view->showMessage("Enter Manufacturer ID:");
            std::cin >> manufacturer_id;
            view->showMessage("Enter Manufacturer Name:");
            std::cin >> manufacturer_name;
            view->showMessage("Enter Location of Facilities:");
            std::cin >> location;

            model->createManufacturer(manufacturer_id, manufacturer_name, location);
        }
        else {
            view->showError("Invalid option. Please try again.");
        }

    }


    void updateData() {
        int choice;
        view->showMessage("Choose table to update data:");
        view->showMessage("1 - Medicine, 2 - Spec, 3 - Category, 4 - Disease, 5 - Manufacturer");
        std::cin >> choice;

        if (choice == 1) {
            int medicine_id;
            std::string column_name, new_value;

            view->showMessage("Enter Medicine ID:");
            std::cin >> medicine_id;
            view->showMessage("Enter column name to update (e.g., Manufacturer_id, Specs_id, Category_id, Treats_id, Contradicts_id, Medicine_name):");
            std::cin >> column_name;
            view->showMessage("Enter new value:");
            std::cin >> new_value;

            model->updateMedicine(medicine_id, column_name, new_value);
        }
        else if (choice == 2) {
            int specs_id;
            std::string column_name, new_value;

            view->showMessage("Enter Specs ID:");
            std::cin >> specs_id;
            view->showMessage("Enter column name to update (e.g., License_name, Term_of_issue, Medicine_form):");
            std::cin >> column_name;
            view->showMessage("Enter new value:");
            std::cin >> new_value;

            model->updateSpec(specs_id, column_name, new_value);
        }
        else if (choice == 3) {
            int category_id;
            std::string new_value;

            view->showMessage("Enter Category ID:");
            std::cin >> category_id;
            view->showMessage("Enter new Category Name:");
            std::cin >> new_value;

            model->updateCategory(category_id, new_value);
        }
        else if (choice == 4) {
            int disease_id;
            std::string new_value;

            view->showMessage("Enter Disease ID:");
            std::cin >> disease_id;
            view->showMessage("Enter new Disease Name:");
            std::cin >> new_value;

            model->updateDisease(disease_id, new_value);
        }
        else if (choice == 5) {
            int manufacturer_id;
            std::string column_name, new_value;

            view->showMessage("Enter Manufacturer ID:");
            std::cin >> manufacturer_id;
            view->showMessage("Enter column name to update (e.g., Manufacturer_name, Location of facilities):");
            std::cin >> column_name;
            view->showMessage("Enter new value:");
            std::cin >> new_value;

            model->updateManufacturer(manufacturer_id, column_name, new_value);
        }
        else {
            view->showError("Invalid option. Please try again.");
        }
    }


    void deleteData() {
        int choice;
        view->showMessage("Choose table to delete data:");
        view->showMessage("1 - Medicine, 2 - Spec, 3 - Category, 4 - Disease, 5 - Manufacturer");
        std::cin >> choice;

        if (choice == 1) {
            int medicine_id;
            view->showMessage("Enter Medicine ID to delete:");
            std::cin >> medicine_id;
            model->deleteMedicine(medicine_id);
        }
        else if (choice == 2) {
            int specs_id;
            view->showMessage("Enter Specs ID to delete:");
            std::cin >> specs_id;
            model->deleteSpec(specs_id);
        }
        else if (choice == 3) {
            int category_id;
            view->showMessage("Enter Category ID to delete:");
            std::cin >> category_id;
            model->deleteCategory(category_id);
        }
        else if (choice == 4) {
            int disease_id;
            view->showMessage("Enter Disease ID to delete:");
            std::cin >> disease_id;
            model->deleteDisease(disease_id);
        }
        else if (choice == 5) {
            int manufacturer_id;
            view->showMessage("Enter Manufacturer ID to delete:");
            std::cin >> manufacturer_id;
            model->deleteManufacturer(manufacturer_id);
        }
        else {
            view->showError("Invalid option. Please try again.");
        }
    }





    void generateData() {

        view->showMessage("Choose table to generate data:");
        view->showMessage("1 - Medicine, 2 - Spec, 3 - Category, 4 - Disease, 5 - Manufacturer");
        int choice;

        std::cin >> choice;
        view->showMessage("Chooså number of writes:");
        int count;
        std::cin >> count;
        model->generateData(count, choice);
    }

    void outTableNames() {

        view->showMessage("Names of all tables:");
        model->outputAllTableNames();
    }

    void outTableColNames() {

        view->showMessage("Select table:");
        std::string table_name;
        std::cin >> table_name;
        model->outputTableColumns(table_name);
    }

    void findMostPopularMan() {

        view->showMessage("Most common manufacurer:");
        model->outputMostCommonManufacturer();
    }

    void findMostTreated() {


        model->outputMostCommonTreat();
    }

    void findMostContradicted()
    {

        model->outputMostCommonContradicted();
    }

public:
    Controller(Model* m, View* v) : model(m), view(v), isRunning(true) {};

    void run() {
        while (isRunning) {
            view->showMenu();
            int choice;
            std::cin >> choice;

            switch (choice) {
            case 1:
                addData();
                break;
            case 2:
                updateData();
                break;
            case 3:
                deleteData();
                break;
            case 4:
                generateData();
                break;
            case 5:
                isRunning = false;
                break;
            case 6:
                outTableNames();
                break;
            case 7:
                outTableColNames();
                break;
            case 8:
                findMostPopularMan();
                break;
            case 9:
                findMostContradicted();
                break;
            case 10:
                findMostTreated();
                break;
            default:
                view->showError("Invalid option. Please try again.");
                break;
            }
        }
    }


};