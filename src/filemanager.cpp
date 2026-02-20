#include "filemanager.h"
#include <fstream>
#include <sstream>
#include <iostream>

FileManager::FileManager(Database& db) : db(db) {}

// ─── Export public (routage selon le rôle) ─────────────────────────────────

void FileManager::exportData(User& user, int studentId) {
    std::string filename;
    std::cout << "Nom du fichier d'export (ex: export.txt) : ";
    std::cin.ignore();
    std::getline(std::cin, filename);

    switch (user.getRole()) {
        case Role::ADMIN:
            exportAll(filename);
            break;
        case Role::PROF:
            exportGradesOnly(filename);
            break;
        case Role::STUDENT:
            exportStudentInfo(filename, studentId);
            break;
    }
}

// ─── Import public (routage selon le rôle) ─────────────────────────────────

void FileManager::importData(User& user) {
    if (user.getRole() == Role::STUDENT) {
        std::cout << "Les étudiants ne peuvent pas importer de données.\n";
        return;
    }

    std::string filename;
    std::cout << "Nom du fichier à importer : ";
    std::cin.ignore();
    std::getline(std::cin, filename);

    switch (user.getRole()) {
        case Role::ADMIN:
            importAll(filename);
            break;
        case Role::PROF:
            importGradesOnly(filename);
            break;
        default:
            break;
    }
}

// ─── Export complet (Admin) ─────────────────────────────────────────────────

void FileManager::exportAll(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "✗ Impossible d'ouvrir le fichier : " << filename << "\n";
        return;
    }

    file << "=== EXPORT COMPLET - " << filename << " ===\n\n";

    // --- Étudiants ---
    file << "--- ETUDIANTS ---\n";
    file << "ID|Nom|Email|Date de naissance\n";
    auto students = db.query("SELECT id, name, email, birthdate FROM students");
    for (auto& row : students) {
        file << row["id"] << "|" << row["name"] << "|"
             << row["email"] << "|" << row["birthdate"] << "\n";
    }

    // --- Cours ---
    file << "\n--- COURS ---\n";
    file << "ID|Nom|Description|Credits\n";
    auto courses = db.query("SELECT id, name, description, credits FROM courses");
    for (auto& row : courses) {
        file << row["id"] << "|" << row["name"] << "|"
             << row["description"] << "|" << row["credits"] << "\n";
    }

    // --- Notes ---
    file << "\n--- NOTES ---\n";
    file << "ID|Etudiant|Cours|Note|Date\n";
    auto grades = db.query(
        "SELECT g.id, s.name AS student, c.name AS course, g.grade, g.date_recorded "
        "FROM grades g "
        "JOIN students s ON g.student_id = s.id "
        "JOIN courses  c ON g.course_id  = c.id");
    for (auto& row : grades) {
        file << row["id"] << "|" << row["student"] << "|"
             << row["course"] << "|" << row["grade"] << "|" << row["date_recorded"] << "\n";
    }

    file.close();
    std::cout << "✓ Export complet → " << filename << "\n";
}

// ─── Export notes seules (Prof) ────────────────────────────────────────────

void FileManager::exportGradesOnly(const std::string& filename, int studentId) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "✗ Impossible d'ouvrir le fichier.\n";
        return;
    }

    file << "--- NOTES ---\n";
    file << "ID|Etudiant|Cours|Note|Date\n";

    std::string sql =
        "SELECT g.id, s.name AS student, c.name AS course, g.grade, g.date_recorded "
        "FROM grades g "
        "JOIN students s ON g.student_id = s.id "
        "JOIN courses  c ON g.course_id  = c.id";

    if (studentId > 0)
        sql += " WHERE g.student_id = " + std::to_string(studentId);

    auto grades = db.query(sql);
    for (auto& row : grades) {
        file << row["id"] << "|" << row["student"] << "|"
             << row["course"] << "|" << row["grade"] << "|" << row["date_recorded"] << "\n";
    }

    file.close();
    std::cout << "✓ Export notes → " << filename << "\n";
}

// ─── Export infos étudiant (Student) ───────────────────────────────────────

void FileManager::exportStudentInfo(const std::string& filename, int studentId) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "✗ Impossible d'ouvrir le fichier.\n";
        return;
    }

    // Infos personnelles
    auto info = db.query(
        "SELECT name, email, birthdate FROM students WHERE id=" + std::to_string(studentId));

    if (!info.empty()) {
        file << "=== MES INFORMATIONS ===\n";
        file << "Nom       : " << info[0]["name"]      << "\n";
        file << "Email     : " << info[0]["email"]     << "\n";
        file << "Naissance : " << info[0]["birthdate"] << "\n\n";
    }

    // Notes
    file << "=== MES NOTES ===\n";
    file << "Cours|Note|Date\n";
    auto grades = db.query(
        "SELECT c.name AS course, g.grade, g.date_recorded "
        "FROM grades g "
        "JOIN courses c ON g.course_id = c.id "
        "WHERE g.student_id = " + std::to_string(studentId));

    for (auto& row : grades) {
        file << row["course"] << "|" << row["grade"] << "|" << row["date_recorded"] << "\n";
    }

    file.close();
    std::cout << "✓ Export mes données → " << filename << "\n";
}

// ─── Import complet (Admin) ─────────────────────────────────────────────────

void FileManager::importAll(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "✗ Fichier introuvable : " << filename << "\n";
        return;
    }

    std::string section;
    std::string line;
    int imported = 0;

    while (std::getline(file, line)) {
        // Ignorer lignes vides et commentaires
        if (line.empty() || line[0] == '=' || line[0] == 'I') continue;

        if (line == "--- ETUDIANTS ---") { section = "students"; continue; }
        if (line == "--- COURS ---")     { section = "courses";  continue; }
        if (line == "--- NOTES ---")     { section = "grades";   continue; }

        // Parser les champs pipe-séparés
        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, '|')) fields.push_back(field);

        if (section == "students" && fields.size() == 4) {
            std::string sql = "INSERT IGNORE INTO students (id, name, email, birthdate) VALUES ("
                + db.escape(fields[0]) + ", '"
                + db.escape(fields[1]) + "', '"
                + db.escape(fields[2]) + "', '"
                + db.escape(fields[3]) + "')";
            if (db.execute(sql)) ++imported;
        }
        else if (section == "courses" && fields.size() == 4) {
            std::string sql = "INSERT IGNORE INTO courses (id, name, description, credits) VALUES ("
                + db.escape(fields[0]) + ", '"
                + db.escape(fields[1]) + "', '"
                + db.escape(fields[2]) + "', "
                + db.escape(fields[3]) + ")";
            if (db.execute(sql)) ++imported;
        }
    }

    file.close();
    std::cout << "✓ Import terminé — " << imported << " ligne(s) importée(s).\n";
}

// ─── Import notes seules (Prof) ────────────────────────────────────────────

void FileManager::importGradesOnly(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "✗ Fichier introuvable.\n";
        return;
    }

    // Format attendu : student_id|course_id|grade
    std::string line;
    int imported = 0;
    bool inGrades = false;

    while (std::getline(file, line)) {
        if (line == "--- NOTES ---") { inGrades = true; continue; }
        if (!inGrades || line.empty() || line[0] == 'I') continue;

        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, '|')) fields.push_back(field);

        // Format : id|student|course|grade|date  → on skippe l'en-tête
        if (fields.size() >= 4 && fields[0] != "ID") {
            // Utilisation directe des IDs si on a un import structuré
            // Format simplifié pour le prof : student_id|course_id|note
            std::string sql = "INSERT INTO grades (student_id, course_id, grade) VALUES ("
                + db.escape(fields[0]) + ", "
                + db.escape(fields[1]) + ", "
                + db.escape(fields[2]) + ")";
            if (db.execute(sql)) ++imported;
        }
    }

    file.close();
    std::cout << "✓ Import notes — " << imported << " ligne(s) importée(s).\n";
}
