#ifndef STUDENT_H
#define STUDENT_H

#include "user.h"
#include "database.h"

class Student : public User {
private:
    Database& db;
    int studentId;  // ID dans la table students (≠ ID dans users)

public:
    Student(int userId, const std::string& username, const std::string& password,
            Database& db, int studentId);

    void showMenu() override;

    void viewMyInfo();    // Voir ses propres informations
    void viewMyGrades();  // Voir ses notes
    void viewMyAverage(); // Calculer sa moyenne générale
};

#endif // STUDENT_H
