#include <iostream>
#include <string>
#include <optional>

using namespace Orm; // Or your chosen namespace as per TinyORM setup

class Category : public Model<Category> {
public:
    int Category_ID;
    std::string Category_Name;

    static constexpr auto tableName = "Categories";

    // Columns mapping (if TinyORM requires manual mapping)
    TINYORM_COLUMNS(Category_ID, Category_Name);

    // Relationships: 1 Category has many Medicines
    // The foreign key is on the Medicines table.
    auto medicines() {
        return hasMany<Medicine>("Category_id");
    }
};

class Disease : public Model<Disease> {
public:
    int Disease_id;
    std::string Disease_name;

    static constexpr auto tableName = "Diseases";
    TINYORM_COLUMNS(Disease_id, Disease_name);

    // Relationships:
    // Many-to-many to Medicine through Medicine_Treats_Diseases
    auto treatedByMedicines() {
        return belongsToMany<Medicine>("Medicine_Treats_Diseases", "Disease_id", "Medicine_id");
    }

    // Many-to-many to Medicine through Medicine_Contradicts_Diseases
    auto contradictedByMedicines() {
        return belongsToMany<Medicine>("Medicine_Contradicts_Diseases", "Disease_id", "Medicine_id");
    }
};

class Manufacturer : public Model<Manufacturer> {
public:
    int Manufacturer_id;
    std::string Manufacturer_name;
    std::string Location_of_facilities; // Remember to handle the column name with spaces appropriately

    static constexpr auto tableName = "Manufactures";
    TINYORM_COLUMNS(Manufacturer_id, Manufacturer_name, Location_of_facilities);

    // Relationships:
    // 1 Manufacturer has many Medicines
    auto medicines() {
        return hasMany<Medicine>("Manufacturer_id");
    }

    // Many-to-many with Medicine through man_med_tab
    auto medicinesThroughManMedTab() {
        return belongsToMany<Medicine>("man_med_tab", "Manufacturer_id", "Medicine_id");
    }
};

class Specs : public Model<Specs> {
public:
    int specs_id;
    std::string Lisence_name;
    bool Term_of_issue;
    std::string Medicine_form;

    static constexpr auto tableName = "Specs";
    TINYORM_COLUMNS(specs_id, Lisence_name, Term_of_issue, Medicine_form);

    // 1 Specs belongs to 1 Medicine (or 1:1)
    // If Specs_id is unique in Medicines, we can define:
    auto medicine() {
        return hasOne<Medicine>("Specs_id");
    }
};

class Medicine : public Model<Medicine> {
public:
    int Medicine_id;
    int Manufacturer_id;
    int Specs_id;
    int Category_id;
    int Treats_id;
    int Contradicts_id;
    std::string Medicine_name;

    static constexpr auto tableName = "Medicines";
    TINYORM_COLUMNS(Medicine_id, Manufacturer_id, Specs_id, Category_id, Treats_id, Contradicts_id, Medicine_name);

    // Relationships:
    auto manufacturer() {
        return belongsTo<Manufacturer>("Manufacturer_id");
    }

    auto specs() {
        return belongsTo<Specs>("Specs_id");
    }

    auto category() {
        return belongsTo<Category>("Category_id");
    }

    // Many-to-many relationships via pivot tables:
    auto treatsDiseases() {
        return belongsToMany<Disease>("Medicine_Treats_Diseases", "Medicine_id", "Disease_id");
    }

    auto contradictsDiseases() {
        return belongsToMany<Disease>("Medicine_Contradicts_Diseases", "Medicine_id", "Disease_id");
    }
};

// Pivot models if needed explicitly (TinyORM might not require explicit pivot models):
class MedicineTreatsDiseases : public Model<MedicineTreatsDiseases> {
public:
    int tab_id;
    int Medicine_id;
    int Disease_id;

    static constexpr auto tableName = "Medicine_Treats_Diseases";
    TINYORM_COLUMNS(tab_id, Medicine_id, Disease_id);
};

class MedicineContradictsDiseases : public Model<MedicineContradictsDiseases> {
public:
    int tab_id;
    int Medicine_id;
    int Disease_id;

    static constexpr auto tableName = "Medicine_Contradicts_Diseases";
    TINYORM_COLUMNS(tab_id, Medicine_id, Disease_id);
};

class ManMedTab : public Model<ManMedTab> {
public:
    int tab_id;
    int Medicine_id;
    int Manufacturer_id;

    static constexpr auto tableName = "man_med_tab";
    TINYORM_COLUMNS(tab_id, Medicine_id, Manufacturer_id);
};

class Model {
public:
    Model() {
        // Ensure that DB::connect(...) or equivalent TinyORM initialization
        // has been done at application start. Not shown here.
    }

    //-------------------------
    // Medicine CRUD
    //-------------------------
    void createMedicine(int manufacturer_id, int specs_id, int category_id,
        int treats_id, int contradicts_id, const std::string& medicine_name)
    {
        Medicine medicine;
        medicine.Manufacturer_id = manufacturer_id;
        medicine.Specs_id = specs_id;
        medicine.Category_id = category_id;
        medicine.Treats_id = treats_id;
        medicine.Contradicts_id = contradicts_id;
        medicine.Medicine_name = medicine_name;
        medicine.save();
        std::cout << "Medicine created successfully." << std::endl;
    }

    void readMedicine(int medicine_id) {
        auto medicine = Medicine::find(medicine_id);
        if (!medicine) {
            std::cout << "No medicine found with ID: " << medicine_id << std::endl;
            return;
        }

        std::cout << "Medicine ID: " << medicine->Medicine_id << "\n"
            << "Manufacturer ID: " << medicine->Manufacturer_id << "\n"
            << "Specs ID: " << medicine->Specs_id << "\n"
            << "Category ID: " << medicine->Category_id << "\n"
            << "Treats ID: " << medicine->Treats_id << "\n"
            << "Contradicts ID: " << medicine->Contradicts_id << "\n"
            << "Medicine Name: " << medicine->Medicine_name << "\n";
    }

    void updateMedicine(int medicine_id, const std::string& column_name, const std::string& new_value) {
        auto medicine = Medicine::find(medicine_id);
        if (!medicine) {
            std::cerr << "No medicine found with ID: " << medicine_id << std::endl;
            return;
        }

        if (column_name == "Medicine_name") {
            medicine->Medicine_name = new_value;
        }
        else if (column_name == "Manufacturer_id") {
            medicine->Manufacturer_id = std::stoi(new_value);
        }
        else if (column_name == "Specs_id") {
            medicine->Specs_id = std::stoi(new_value);
        }
        else if (column_name == "Category_id") {
            medicine->Category_id = std::stoi(new_value);
        }
        else if (column_name == "Treats_id") {
            medicine->Treats_id = std::stoi(new_value);
        }
        else if (column_name == "Contradicts_id") {
            medicine->Contradicts_id = std::stoi(new_value);
        }
        else {
            std::cerr << "Unknown column: " << column_name << std::endl;
            return;
        }

        medicine->save();
        std::cout << "Medicine updated successfully." << std::endl;
    }

    void deleteMedicine(int medicine_id) {
        auto medicine = Medicine::find(medicine_id);
        if (!medicine) {
            std::cerr << "No medicine found with ID: " << medicine_id << std::endl;
            return;
        }

        medicine->remove();
        std::cout << "Medicine deleted successfully." << std::endl;
    }

    //-------------------------
    // Category CRUD
    //-------------------------
    void createCategory(int category_id, const std::string& category_name) {
        Category category;
        category.Category_ID = category_id;
        category.Category_Name = category_name;
        category.save();

        std::cout << "Category created successfully." << std::endl;
    }

    void readCategory(int category_id) {
        auto category = Category::find(category_id);
        if (!category) {
            std::cout << "No category found with ID: " << category_id << std::endl;
            return;
        }

        std::cout << "Category ID: " << category->Category_ID << "\n"
            << "Category Name: " << category->Category_Name << "\n";
    }

    void updateCategory(int category_id, const std::string& new_name) {
        auto category = Category::find(category_id);
        if (!category) {
            std::cerr << "No category found with ID: " << category_id << std::endl;
            return;
        }

        category->Category_Name = new_name;
        category->save();
        std::cout << "Category updated successfully." << std::endl;
    }

    void deleteCategory(int category_id) {
        auto category = Category::find(category_id);
        if (!category) {
            std::cerr << "No category found with ID: " << category_id << std::endl;
            return;
        }

        category->remove();
        std::cout << "Category deleted successfully." << std::endl;
    }

    //-------------------------
    // Disease CRUD
    //-------------------------
    void createDisease(int disease_id, const std::string& disease_name) {
        Disease disease;
        disease.Disease_id = disease_id;
        disease.Disease_name = disease_name;
        disease.save();

        std::cout << "Disease created successfully." << std::endl;
    }

    void readDisease(int disease_id) {
        auto disease = Disease::find(disease_id);
        if (!disease) {
            std::cout << "No disease found with ID: " << disease_id << std::endl;
            return;
        }

        std::cout << "Disease ID: " << disease->Disease_id << "\n"
            << "Disease Name: " << disease->Disease_name << "\n";
    }

    void updateDisease(int disease_id, const std::string& new_name) {
        auto disease = Disease::find(disease_id);
        if (!disease) {
            std::cerr << "No disease found with ID: " << disease_id << std::endl;
            return;
        }

        disease->Disease_name = new_name;
        disease->save();
        std::cout << "Disease updated successfully." << std::endl;
    }

    void deleteDisease(int disease_id) {
        auto disease = Disease::find(disease_id);
        if (!disease) {
            std::cerr << "No disease found with ID: " << disease_id << std::endl;
            return;
        }

        disease->remove();
        std::cout << "Disease deleted successfully." << std::endl;
    }

    //-------------------------
    // Manufacturer CRUD
    //-------------------------
    void createManufacturer(int manufacturer_id, const std::string& manufacturer_name, const std::string& location) {
        Manufacturer manufacturer;
        manufacturer.Manufacturer_id = manufacturer_id;
        manufacturer.Manufacturer_name = manufacturer_name;
        manufacturer.Location_of_facilities = location;
        manufacturer.save();

        std::cout << "Manufacturer created successfully." << std::endl;
    }

    void readManufacturer(int manufacturer_id) {
        auto manufacturer = Manufacturer::find(manufacturer_id);
        if (!manufacturer) {
            std::cout << "No manufacturer found with ID: " << manufacturer_id << std::endl;
            return;
        }

        std::cout << "Manufacturer ID: " << manufacturer->Manufacturer_id << "\n"
            << "Manufacturer Name: " << manufacturer->Manufacturer_name << "\n"
            << "Location of Facilities: " << manufacturer->Location_of_facilities << "\n";
    }

    void updateManufacturer(int manufacturer_id, const std::string& column_name, const std::string& new_value) {
        auto manufacturer = Manufacturer::find(manufacturer_id);
        if (!manufacturer) {
            std::cerr << "No manufacturer found with ID: " << manufacturer_id << std::endl;
            return;
        }

        if (column_name == "Manufacturer_name") {
            manufacturer->Manufacturer_name = new_value;
        }
        else if (column_name == "Location_of_facilities") {
            manufacturer->Location_of_facilities = new_value;
        }
        else {
            std::cerr << "Unknown column: " << column_name << std::endl;
            return;
        }

        manufacturer->save();
        std::cout << "Manufacturer updated successfully." << std::endl;
    }

    void deleteManufacturer(int manufacturer_id) {
        auto manufacturer = Manufacturer::find(manufacturer_id);
        if (!manufacturer) {
            std::cerr << "No manufacturer found with ID: " << manufacturer_id << std::endl;
            return;
        }

        manufacturer->remove();
        std::cout << "Manufacturer deleted successfully." << std::endl;
    }

    //-------------------------
    // Specs CRUD
    //-------------------------
    void createSpec(int specs_id, const std::string& license_name, bool term_of_issue, const std::string& medicine_form) {
        Specs specs;
        specs.specs_id = specs_id;
        specs.Lisence_name = license_name;
        specs.Term_of_issue = term_of_issue;
        specs.Medicine_form = medicine_form;
        specs.save();

        std::cout << "Spec created successfully." << std::endl;
    }

    void readSpec(int specs_id) {
        auto specs = Specs::find(specs_id);
        if (!specs) {
            std::cout << "No specs found with ID: " << specs_id << std::endl;
            return;
        }

        std::cout << "Specs ID: " << specs->specs_id << "\n"
            << "License Name: " << specs->Lisence_name << "\n"
            << "Term of Issue: " << (specs->Term_of_issue ? "True" : "False") << "\n"
            << "Medicine Form: " << specs->Medicine_form << "\n";
    }

    void updateSpec(int specs_id, const std::string& column_name, const std::string& new_value) {
        auto specs = Specs::find(specs_id);
        if (!specs) {
            std::cerr << "No specs found with ID: " << specs_id << std::endl;
            return;
        }

        if (column_name == "Lisence_name") {
            specs->Lisence_name = new_value;
        }
        else if (column_name == "Term_of_issue") {
            // Convert string to bool if necessary (e.g. "true"/"false")
            bool val = (new_value == "true" || new_value == "1");
            specs->Term_of_issue = val;
        }
        else if (column_name == "Medicine_form") {
            specs->Medicine_form = new_value;
        }
        else {
            std::cerr << "Unknown column: " << column_name << std::endl;
            return;
        }

        specs->save();
        std::cout << "Specs updated successfully." << std::endl;
    }

    void deleteSpec(int specs_id) {
        auto specs = Specs::find(specs_id);
        if (!specs) {
            std::cerr << "No specs found with ID: " << specs_id << std::endl;
            return;
        }

        specs->remove();
        std::cout << "Spec deleted successfully." << std::endl;
    }
};