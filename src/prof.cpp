#include "prof.h"
#include <iostream>
#include <iomanip>

Prof::Prof(int id, const std::string& username, const std::string& password, Database& db)
    : User(id, username, password, Role::PROF), db(db) {}

void Prof::showMenu() {
    int choice = 0;
    do {
        std::cout << "\n==============================\n";
        std::cout << "   MENU PROFESSEUR\n";
        std::cout << "==============================\n";
        std::cout << "  [1] Voir les étudiants\n";
        std::cout << "  [2] Voir les cours\n";
        std::cout << "  [3] Voir les notes\n";
        std::cout << "  [4] Ajouter une note\n";
        std::cout << "  [5] Modifier une note\n";
        std::cout << "  [0] Déconnexion\n";
        std::cout << "------------------------------\n";
        std::cout << "Choix : ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1: listStudents(); break;
            case 2: listCourses();  break;
            case 3: listGrades();   break;
            case 4: addGrade();     break;
            case 5: updateGrade();  break;
            case 0: std::cout << "Déconnexion...\n"; break;
            default: std::cout << "Option invalide.\n";
        }
    } while (choice != 0);
}

void Prof::listStudents() {
    auto rows = db.query("SELECT id, name, email FROM students ORDER BY name");
    if (rows.empty()) { std::cout << "Aucun étudiant.\n"; return; }

    std::cout << "\n" << std::left
              << std::setw(5)  << "ID"
              << std::setw(25) << "Nom"
              << std::setw(30) << "Email" << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (auto& row : rows) {
        std::cout << std::left
                  << std::setw(5)  << row["id"]
                  << std::setw(25) << row["name"]
                  << std::setw(30) << row["email"] << "\n";
    }
}

void Prof::listCourses() {
    auto rows = db.query("SELECT id, name, credits FROM courses ORDER BY name");
    if (rows.empty()) { std::cout << "Aucun cours.\n"; return; }

    std::cout << "\n" << std::left
              << std::setw(5)  << "ID"
              << std::setw(25) << "Cours"
              << std::setw(10) << "Crédits" << "\n";
    std::cout << std::string(40, '-') << "\n";

    for (auto& row : rows) {
        std::cout << std::left
                  << std::setw(5)  << row["id"]
                  << std::setw(25) << row["name"]
                  << std::setw(10) << row["credits"] << "\n";
    }
}

void Prof::listGrades() {
    auto rows = db.query(
        "SELECT g.id, s.name AS student, c.name AS course, g.grade, g.date_recorded "
        "FROM grades g "
        "JOIN students s ON g.student_id = s.id "
        "JOIN courses  c ON g.course_id  = c.id "
        "ORDER BY s.name, c.name");

    if (rows.empty()) { std::cout << "Aucune note.\n"; return; }

    std::cout << "\n" << std::left
              << std::setw(5)  << "ID"
              << std::setw(25) << "Étudiant"
              << std::setw(25) << "Cours"
              << std::setw(8)  << "Note"
              << std::setw(12) << "Date" << "\n";
    std::cout << std::string(75, '-') << "\n";

    for (auto& row : rows) {
        std::cout << std::left
                  << std::setw(5)  << row["id"]
                  << std::setw(25) << row["student"]
                  << std::setw(25) << row["course"]
                  << std::setw(8)  << row["grade"]
                  << std::setw(12) << row["date_recorded"] << "\n";
    }
}

void Prof::addGrade() {
    listStudents();
    int sId;
    std::cout << "ID étudiant : "; std::cin >> sId; std::cin.ignore();

    listCourses();
    int cId;
    std::cout << "ID cours    : "; std::cin >> cId; std::cin.ignore();

    std::string grade;
    std::cout << "Note (0-20) : "; std::getline(std::cin, grade);

    std::string sql = "INSERT INTO grades (student_id, course_id, grade) VALUES ("
        + std::to_string(sId) + ", "
        + std::to_string(cId) + ", "
        + db.escape(grade) + ")";

    if (db.execute(sql))
        std::cout << "✓ Note ajoutée.\n";
    else
        std::cout << "✗ Erreur lors de l'ajout.\n";
}

void Prof::updateGrade() {
    listGrades();
    int id;
    std::cout << "ID de la note à modifier : "; std::cin >> id; std::cin.ignore();

    std::string grade;
    std::cout << "Nouvelle note : "; std::getline(std::cin, grade);

    std::string sql = "UPDATE grades SET grade=" + db.escape(grade)
        + " WHERE id=" + std::to_string(id);

    if (db.execute(sql))
        std::cout << "✓ Note mise à jour.\n";
    else
        std::cout << "✗ Erreur lors de la mise à jour.\n";
}
