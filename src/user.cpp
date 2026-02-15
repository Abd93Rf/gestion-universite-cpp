#include "User.h"

User::User(int id, const std::string& username, const std::string& password, Role role)
    : id(id), username(username), password(password), role(role) {}

int User::getId() const { return id; }
std::string User::getUsername() const { return username; }
Role User::getRole() const { return role; }

std::string User::getRoleName() const {
    switch (role) {
        case Role::ADMIN:   return "Admin";
        case Role::PROF:    return "Professeur";
        case Role::STUDENT: return "Étudiant";
        default:            return "Inconnu";
    }
}