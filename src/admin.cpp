#include "admin.h"
#include <iostream>
#include <iomanip>

Admin::Admin(int id, const std::string& username, const std::string& password, Database& db)
    : User(id, username, password, Role::ADMIN), db(db) {}

void Admin::showMenu() {
    int choice = 0;
    do {
        std::cout << "\n==============================\n";
        std::cout << "   MENU ADMINISTRATEUR\n";
        std::cout << "==============================\n";
        std::cout << "  [1] Gérer les étudiants\n";
        std::cout << "  [2] Gérer les cours\n";
        std::cout << "  [3] Gérer les notes\n";
        std::cout << "  [4] Gérer les utilisateurs\n";
        std::cout << "  [0] Déconnexion\n";
        std::cout << "------------------------------\n";
        std::cout << "Choix : ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1: {
                int sub = 0;
                std::cout << "\n-- Étudiants --\n";
                std::cout << "[1] Lister  [2] Ajouter  [3] Modifier  [4] Supprimer\nChoix : ";
                std::cin >> sub; std::cin.ignore();
                if (sub == 1) listStudents();
                else if (sub == 2) addStudent();
                else if (sub == 3) updateStudent();
                else if (sub == 4) deleteStudent();
                break;
            }
            case 2: {
                int sub = 0;
                std::cout << "\n-- Cours --\n";
                std::cout << "[1] Lister  [2] Ajouter  [3] Supprimer\nChoix : ";
                std::cin >> sub; std::cin.ignore();
                if (sub == 1) listCourses();
                else if (sub == 2) addCourse();
                else if (sub == 3) deleteCourse();
                break;
            }
            case 3: {
                int sub = 0;
                std::cout << "\n-- Notes --\n";
                std::cout << "[1] Lister  [2] Ajouter  [3] Modifier  [4] Supprimer\nChoix : ";
                std::cin >> sub; std::cin.ignore();
                if (sub == 1) listGrades();
                else if (sub == 2) addGrade();
                else if (sub == 3) updateGrade();
                else if (sub == 4) deleteGrade();
                break;
            }
            case 4: {
                int sub = 0;
                std::cout << "\n-- Utilisateurs --\n";
                std::cout << "[1] Lister  [2] Ajouter  [3] Supprimer\nChoix : ";
                std::cin >> sub; std::cin.ignore();
                if (sub == 1) listUsers();
                else if (sub == 2) addUser();
                else if (sub == 3) deleteUser();
                break;
            }
            case 0:
                std::cout << "Déconnexion...\n";
                break;
            default:
                std::cout << "Option invalide.\n";
        }
    } while (choice != 0);
}

// ─── ÉTUDIANTS ─────────────────────────────────────────────────────────────

void Admin::listStudents() {
    auto rows = db.query("SELECT s.id, s.name, s.email, s.birthdate "
                         "FROM students s ORDER BY s.name");
    if (rows.empty()) { std::cout << "Aucun étudiant trouvé.\n"; return; }

    std::cout << "\n" << std::left
              << std::setw(5)  << "ID"
              << std::setw(25) << "Nom"
              << std::setw(30) << "Email"
              << std::setw(15) << "Date de naissance" << "\n";
    std::cout << std::string(75, '-') << "\n";

    for (auto& row : rows) {
        std::cout << std::left
                  << std::setw(5)  << row["id"]
                  << std::setw(25) << row["name"]
                  << std::setw(30) << row["email"]
                  << std::setw(15) << row["birthdate"] << "\n";
    }
}

void Admin::addStudent() {
    std::string name, email, birthdate;
    std::cout << "Nom complet : "; std::getline(std::cin, name);
    std::cout << "Email       : "; std::getline(std::cin, email);
    std::cout << "Date de naissance (YYYY-MM-DD) : "; std::getline(std::cin, birthdate);

    std::string sql = "INSERT INTO students (name, email, birthdate) VALUES ('"
        + db.escape(name) + "', '"
        + db.escape(email) + "', '"
        + db.escape(birthdate) + "')";

    if (db.execute(sql))
        std::cout << "✓ Étudiant ajouté (ID=" << db.getLastInsertId() << ")\n";
    else
        std::cout << "✗ Erreur lors de l'ajout.\n";
}

void Admin::updateStudent() {
    listStudents();
    int id;
    std::cout << "ID étudiant à modifier : "; std::cin >> id; std::cin.ignore();

    std::string name, email;
    std::cout << "Nouveau nom  : "; std::getline(std::cin, name);
    std::cout << "Nouvel email : "; std::getline(std::cin, email);

    std::string sql = "UPDATE students SET name='" + db.escape(name)
        + "', email='" + db.escape(email)
        + "' WHERE id=" + std::to_string(id);

    if (db.execute(sql))
        std::cout << "✓ Étudiant mis à jour.\n";
    else
        std::cout << "✗ Erreur lors de la mise à jour.\n";
}

void Admin::deleteStudent() {
    listStudents();
    int id;
    std::cout << "ID étudiant à supprimer : "; std::cin >> id; std::cin.ignore();

    std::string sql = "DELETE FROM students WHERE id=" + std::to_string(id);
    if (db.execute(sql))
        std::cout << "✓ Étudiant supprimé.\n";
    else
        std::cout << "✗ Erreur lors de la suppression.\n";
}

// ─── COURS ─────────────────────────────────────────────────────────────────

void Admin::listCourses() {
    auto rows = db.query("SELECT id, name, description, credits FROM courses ORDER BY name");
    if (rows.empty()) { std::cout << "Aucun cours trouvé.\n"; return; }

    std::cout << "\n" << std::left
              << std::setw(5)  << "ID"
              << std::setw(25) << "Cours"
              << std::setw(30) << "Description"
              << std::setw(10) << "Crédits" << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (auto& row : rows) {
        std::cout << std::left
                  << std::setw(5)  << row["id"]
                  << std::setw(25) << row["name"]
                  << std::setw(30) << row["description"]
                  << std::setw(10) << row["credits"] << "\n";
    }
}

void Admin::addCourse() {
    std::string name, desc, credits;
    std::cout << "Nom du cours  : "; std::getline(std::cin, name);
    std::cout << "Description   : "; std::getline(std::cin, desc);
    std::cout << "Crédits ECTS  : "; std::getline(std::cin, credits);

    std::string sql = "INSERT INTO courses (name, description, credits) VALUES ('"
        + db.escape(name) + "', '"
        + db.escape(desc) + "', "
        + db.escape(credits) + ")";

    if (db.execute(sql))
        std::cout << "✓ Cours ajouté (ID=" << db.getLastInsertId() << ")\n";
    else
        std::cout << "✗ Erreur lors de l'ajout.\n";
}

void Admin::deleteCourse() {
    listCourses();
    int id;
    std::cout << "ID cours à supprimer : "; std::cin >> id; std::cin.ignore();

    std::string sql = "DELETE FROM courses WHERE id=" + std::to_string(id);
    if (db.execute(sql))
        std::cout << "✓ Cours supprimé.\n";
    else
        std::cout << "✗ Erreur.\n";
}

// ─── NOTES ─────────────────────────────────────────────────────────────────

void Admin::listGrades() {
    auto rows = db.query(
        "SELECT g.id, s.name AS student, c.name AS course, g.grade, g.date_recorded "
        "FROM grades g "
        "JOIN students s ON g.student_id = s.id "
        "JOIN courses  c ON g.course_id  = c.id "
        "ORDER BY s.name, c.name");

    if (rows.empty()) { std::cout << "Aucune note trouvée.\n"; return; }

    std::cout << "\n" << std::left
              << std::setw(5)  << "ID"
              << std::setw(25) << "Étudiant"
              << std::setw(25) << "Cours"
              << std::setw(8)  << "Note"
              << std::setw(15) << "Date" << "\n";
    std::cout << std::string(78, '-') << "\n";

    for (auto& row : rows) {
        std::cout << std::left
                  << std::setw(5)  << row["id"]
                  << std::setw(25) << row["student"]
                  << std::setw(25) << row["course"]
                  << std::setw(8)  << row["grade"]
                  << std::setw(15) << row["date_recorded"] << "\n";
    }
}

void Admin::addGrade() {
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

void Admin::updateGrade() {
    listGrades();
    int id;
    std::cout << "ID note à modifier : "; std::cin >> id; std::cin.ignore();

    std::string grade;
    std::cout << "Nouvelle note : "; std::getline(std::cin, grade);

    std::string sql = "UPDATE grades SET grade=" + db.escape(grade)
        + " WHERE id=" + std::to_string(id);

    if (db.execute(sql))
        std::cout << "✓ Note mise à jour.\n";
    else
        std::cout << "✗ Erreur.\n";
}

void Admin::deleteGrade() {
    listGrades();
    int id;
    std::cout << "ID note à supprimer : "; std::cin >> id; std::cin.ignore();

    if (db.execute("DELETE FROM grades WHERE id=" + std::to_string(id)))
        std::cout << "✓ Note supprimée.\n";
    else
        std::cout << "✗ Erreur.\n";
}

// ─── UTILISATEURS ──────────────────────────────────────────────────────────

void Admin::listUsers() {
    auto rows = db.query("SELECT id, username, role FROM users ORDER BY role, username");
    if (rows.empty()) { std::cout << "Aucun utilisateur.\n"; return; }

    std::cout << "\n" << std::left
              << std::setw(5)  << "ID"
              << std::setw(20) << "Login"
              << std::setw(12) << "Rôle" << "\n";
    std::cout << std::string(37, '-') << "\n";

    for (auto& row : rows) {
        std::cout << std::left
                  << std::setw(5)  << row["id"]
                  << std::setw(20) << row["username"]
                  << std::setw(12) << row["role"] << "\n";
    }
}

void Admin::addUser() {
    std::string uname, pwd, role;
    std::cout << "Login      : "; std::getline(std::cin, uname);
    std::cout << "Mot de passe : "; std::getline(std::cin, pwd);
    std::cout << "Rôle (admin/prof/student) : "; std::getline(std::cin, role);

    std::string sql = "INSERT INTO users (username, password, role) VALUES ('"
        + db.escape(uname) + "', '"
        + db.escape(pwd) + "', '"
        + db.escape(role) + "')";

    if (db.execute(sql))
        std::cout << "✓ Utilisateur créé (ID=" << db.getLastInsertId() << ")\n";
    else
        std::cout << "✗ Erreur.\n";
}

void Admin::deleteUser() {
    listUsers();
    int id;
    std::cout << "ID utilisateur à supprimer : "; std::cin >> id; std::cin.ignore();

    if (db.execute("DELETE FROM users WHERE id=" + std::to_string(id)))
        std::cout << "✓ Utilisateur supprimé.\n";
    else
        std::cout << "✗ Erreur.\n";
}
