#ifndef USER_H
#define USER_H

#include <string>

enum class Role {
    ADMIN,
    PROF,
    STUDENT
};

class User {
protected:
    int id;
    std::string username;
    std::string password;
    Role role;

public:
    User(int id, const std::string& username, const std::string& password, Role role);
    virtual ~User() = default;

    int getId() const;
    std::string getUsername() const;
    Role getRole() const;
    std::string getRoleName() const;

    // Méthode polymorphique : chaque rôle affiche son propre menu
    virtual void showMenu() = 0;
};

#endif // USER_H