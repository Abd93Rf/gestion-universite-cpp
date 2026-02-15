#ifndef PROF_H
#define PROF_H

#include "User.h"
#include "Database.h"

class Prof : public User {
private:
    Database& db;

public:
    Prof(int id, const std::string& username, const std::string& password, Database& db);

    void showMenu() override;

    void listStudents();          // Consulter la liste des étudiants
    void listCourses();           // Voir les cours disponibles
    void listGrades();            // Voir toutes les notes
    void updateGrade();           // Modifier une note
    void addGrade();              // Ajouter une note
};

#endif // PROF_H