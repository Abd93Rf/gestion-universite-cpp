#ifndef ADMIN_H
#define ADMIN_H

#include "User.h"
#include "Database.h"

class Admin : public User {
private:
    Database& db;

public:
    Admin(int id, const std::string& username, const std::string& password, Database& db);

    void showMenu() override;

    // Gestion des étudiants
    void listStudents();
    void addStudent();
    void updateStudent();
    void deleteStudent();

    // Gestion des cours
    void listCourses();
    void addCourse();
    void deleteCourse();

    // Gestion des notes
    void listGrades();
    void addGrade();
    void updateGrade();
    void deleteGrade();

    // Gestion des utilisateurs
    void listUsers();
    void addUser();
    void deleteUser();
};

#endif // ADMIN_H