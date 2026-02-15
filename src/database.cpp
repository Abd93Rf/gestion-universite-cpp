#include "database.h"
#include <sstream>

Database::Database(const std::string& dbPath) : db(nullptr), dbPath(dbPath) {}

Database::~Database() { disconnect(); }

bool Database::connect() {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB ERROR] " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
        return false;
    }
    // Activer les clés étrangères
    execute("PRAGMA foreign_keys = ON;");
    std::cout << "[DB] Connecté à : " << dbPath << std::endl;
    initSchema();
    return true;
}

void Database::disconnect() {
    if (db) { sqlite3_close(db); db = nullptr; }
}

bool Database::isConnected() const { return db != nullptr; }

// Callback interne pour récupérer les résultats
static int queryCallback(void* data, int argc, char** argv, char** colNames) {
    auto* results = static_cast<ResultSet*>(data);
    Row row;
    for (int i = 0; i < argc; ++i)
        row[colNames[i]] = argv[i] ? argv[i] : "NULL";
    results->push_back(row);
    return 0;
}

ResultSet Database::query(const std::string& sql) {
    ResultSet results;
    char* errMsg = nullptr;
    sqlite3_exec(db, sql.c_str(), queryCallback, &results, &errMsg);
    if (errMsg) {
        std::cerr << "[DB ERROR] " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    return results;
}

bool Database::execute(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB ERROR] " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int Database::getLastInsertId() {
    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

// Échappe les apostrophes pour éviter les injections SQL
std::string Database::escape(const std::string& value) {
    std::string result;
    for (char c : value) {
        if (c == '\'') result += "''";
        else result += c;
    }
    return result;
}

void Database::initSchema() {
    // Création des tables
    execute(R"(
        CREATE TABLE IF NOT EXISTS users (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            username   TEXT NOT NULL UNIQUE,
            password   TEXT NOT NULL,
            role       TEXT NOT NULL CHECK(role IN ('admin','prof','student')),
            email      TEXT,
            created_at TEXT DEFAULT (datetime('now'))
        );
    )");

    execute(R"(
        CREATE TABLE IF NOT EXISTS students (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            name       TEXT NOT NULL,
            email      TEXT UNIQUE,
            birthdate  TEXT,
            created_at TEXT DEFAULT (datetime('now'))
        );
    )");

    execute(R"(
        CREATE TABLE IF NOT EXISTS courses (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT NOT NULL,
            description TEXT,
            credits     INTEGER DEFAULT 3
        );
    )");

    execute(R"(
        CREATE TABLE IF NOT EXISTS grades (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            student_id    INTEGER NOT NULL,
            course_id     INTEGER NOT NULL,
            grade         REAL    NOT NULL CHECK(grade >= 0 AND grade <= 20),
            date_recorded TEXT    DEFAULT (date('now')),
            FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,
            FOREIGN KEY (course_id)  REFERENCES courses(id)  ON DELETE CASCADE
        );
    )");

    // Insérer les données de test seulement si la table est vide
    auto rows = query("SELECT COUNT(*) AS nb FROM users;");
    if (!rows.empty() && rows[0]["nb"] == "0") {
        execute(R"(
            INSERT INTO users (username, password, role, email) VALUES
                ('admin',   'admin123',   'admin',   'admin@univ.fr'),
                ('dupont',  'prof456',    'prof',    'dupont@univ.fr'),
                ('alice',   'alice789',   'student', 'alice@etud.fr'),
                ('bob',     'bob101',     'student', 'bob@etud.fr'),
                ('charlie', 'charlie202', 'student', 'charlie@etud.fr');
        )");

        execute(R"(
            INSERT INTO students (name, email, birthdate) VALUES
                ('Alice Martin',  'alice@etud.fr',   '2002-03-15'),
                ('Bob Dupuis',    'bob@etud.fr',     '2001-07-22'),
                ('Charlie Leroy', 'charlie@etud.fr', '2003-01-10');
        )");

        execute(R"(
            INSERT INTO courses (name, description, credits) VALUES
                ('Algorithmique',    'Introduction aux algorithmes', 4),
                ('Bases de donnees', 'Conception et requetes SQL',   4),
                ('Programmation C++','POO, STL, templates',          5),
                ('Reseaux',          'Protocoles TCP/IP',            3),
                ('Mathematiques',    'Analyse et algebre lineaire',  3);
        )");

        execute(R"(
            INSERT INTO grades (student_id, course_id, grade) VALUES
                (1,1,15.5),(1,2,17.0),(1,3,14.5),(1,4,12.0),(1,5,16.0),
                (2,1,11.0),(2,2,13.5),(2,3,10.0),(2,4,14.0),
                (3,1,18.0),(3,3,19.5),(3,5,17.5);
        )");

        std::cout << "[DB] Donnees de test inserees.\n";
    }
}