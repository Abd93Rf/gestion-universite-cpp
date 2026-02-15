#include <iostream>
#include <string>
#include <memory>

#include "database.h"
#include "user.h"
#include "admin.h"
#include "prof.h"
#include "student.h"
#include "filemanager.h"

void showBanner() {
    std::cout << "\n";
    std::cout << "==========================================\n";
    std::cout << "   SYSTEME DE GESTION ETUDIANT - C++     \n";
    std::cout << "   Connexion securisee par role           \n";
    std::cout << "==========================================\n\n";
}

std::unique_ptr<User> authenticate(Database& db) {
    std::string username, password;
    std::cout << "Login    : "; std::getline(std::cin, username);
    std::cout << "Password : "; std::getline(std::cin, password);

    std::string sql = "SELECT id, username, role FROM users WHERE username='"
        + db.escape(username) + "' AND password='" + db.escape(password) + "'";

    auto rows = db.query(sql);
    if (rows.empty()) {
        std::cout << "\n✗ Identifiants incorrects.\n";
        return nullptr;
    }

    int uid = std::stoi(rows[0]["id"]);
    std::string role = rows[0]["role"];
    std::cout << "\n✓ Connecte en tant que : " << username << " [" << role << "]\n";

    if (role == "admin")
        return std::make_unique<Admin>(uid, username, password, db);

    if (role == "prof")
        return std::make_unique<Prof>(uid, username, password, db);

    if (role == "student") {
        auto sRows = db.query(
            "SELECT id FROM students WHERE email=("
            "SELECT email FROM users WHERE id=" + std::to_string(uid) + ")");
        int studentId = sRows.empty() ? -1 : std::stoi(sRows[0]["id"]);
        return std::make_unique<Student>(uid, username, password, db, studentId);
    }

    return nullptr;
}

int main() {
    showBanner();

    // SQLite : juste un fichier dans le dossier du projet
    Database db("student_management.db");

    if (!db.connect()) {
        std::cerr << "Impossible d'ouvrir la base de donnees.\n";
        return 1;
    }

    FileManager fm(db);
    bool running = true;

    while (running) {
        std::cout << "\n[1] Se connecter   [0] Quitter\nChoix : ";
        int choice; std::cin >> choice; std::cin.ignore();

        if (choice == 0) {
            running = false;
            std::cout << "Au revoir !\n";
        } else if (choice == 1) {
            auto user = authenticate(db);
            if (user) user->showMenu();
        } else {
            std::cout << "Option invalide.\n";
        }
    }

    db.disconnect();
    return 0;
}