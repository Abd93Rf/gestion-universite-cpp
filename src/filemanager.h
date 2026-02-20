#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "user.h"
#include "database.h"
#include <string>

class FileManager {
private:
    Database& db;

    // Helpers internes
    void exportAll(const std::string& filename);
    void exportGradesOnly(const std::string& filename, int studentId = -1);
    void exportStudentInfo(const std::string& filename, int studentId);

    void importAll(const std::string& filename);
    void importGradesOnly(const std::string& filename);

public:
    explicit FileManager(Database& db);

    // Export selon le rôle
    void exportData(User& user, int studentId = -1);

    // Import selon le rôle
    void importData(User& user);
};

#endif // FILEMANAGER_H
