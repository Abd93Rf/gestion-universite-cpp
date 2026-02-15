#include "Student.h"
#include <iostream>
#include <iomanip>
#include <numeric>

Student::Student(int userId, const std::string& username, const std::string& password,
                 Database& db, int studentId)
    : User(userId, username, password, Role::STUDENT), db(db), studentId(studentId) {}

void Student::showMenu() {
    int choice = 0;
    do {
        std::cout << "\n==============================\n";
        std::cout << "   MENU ÉTUDIANT\n";
        std::cout << "==============================\n";
        std::cout << "  [1] Mes informations\n";
        std::cout << "  [2] Mes notes\n";
        std::cout << "  [3] Ma moyenne générale\n";
        std::cout << "  [0] Déconnexion\n";
        std::cout << "------------------------------\n";
        std::cout << "Choix : ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1: viewMyInfo();    break;
            case 2: viewMyGrades();  break;
            case 3: viewMyAverage(); break;
            case 0: std::cout << "Déconnexion...\n"; break;
            default: std::cout << "Option invalide.\n";
        }
    } while (choice != 0);
}

void Student::viewMyInfo() {
    auto rows = db.query(
        "SELECT name, email, birthdate FROM students WHERE id=" + std::to_string(studentId));

    if (rows.empty()) {
        std::cout << "Informations introuvables.\n";
        return;
    }

    auto& row = rows[0];
    std::cout << "\n===== MES INFORMATIONS =====\n";
    std::cout << "  Nom       : " << row["name"]      << "\n";
    std::cout << "  Email     : " << row["email"]     << "\n";
    std::cout << "  Naissance : " << row["birthdate"] << "\n";
    std::cout << "  Login     : " << username          << "\n";
    std::cout << "============================\n";
}

void Student::viewMyGrades() {
    auto rows = db.query(
        "SELECT c.name AS course, g.grade, g.date_recorded "
        "FROM grades g "
        "JOIN courses c ON g.course_id = c.id "
        "WHERE g.student_id = " + std::to_string(studentId) +
        " ORDER BY c.name");

    if (rows.empty()) {
        std::cout << "Aucune note enregistrée.\n";
        return;
    }

    std::cout << "\n===== MES NOTES =====\n";
    std::cout << std::left
              << std::setw(30) << "Cours"
              << std::setw(8)  << "Note"
              << std::setw(12) << "Date" << "\n";
    std::cout << std::string(50, '-') << "\n";

    for (auto& row : rows) {
        std::cout << std::left
                  << std::setw(30) << row["course"]
                  << std::setw(8)  << row["grade"]
                  << std::setw(12) << row["date_recorded"] << "\n";
    }
}

void Student::viewMyAverage() {
    auto rows = db.query(
        "SELECT AVG(grade) AS avg FROM grades WHERE student_id=" + std::to_string(studentId));

    if (rows.empty() || rows[0]["avg"] == "NULL") {
        std::cout << "Aucune note pour calculer la moyenne.\n";
        return;
    }

    double avg = std::stod(rows[0]["avg"]);
    std::cout << "\nMoyenne générale : " << std::fixed << std::setprecision(2) << avg << " / 20\n";

    // Mention
    std::cout << "Mention : ";
    if (avg >= 16)       std::cout << "Très Bien\n";
    else if (avg >= 14)  std::cout << "Bien\n";
    else if (avg >= 12)  std::cout << "Assez Bien\n";
    else if (avg >= 10)  std::cout << "Passable\n";
    else                 std::cout << "Insuffisant\n";
}