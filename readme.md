# Système de Gestion des Données Étudiantes

Projet C++ orienté objet — TP Noté C++ 2

---

## Structure du projet

```
student-management/
├── src/
│   ├── main.cpp             ← Point d'entrée, authentification, boucle principale
│   ├── user.h / .cpp        ← Classe de base abstraite
│   ├── admin.h / .cpp       ← Hérite de User — accès complet
│   ├── prof.h / .cpp        ← Hérite de User — accès limité
│   ├── student.h / .cpp     ← Hérite de User — lecture seule
│   ├── database.h / .cpp    ← Gestion connexion SQLite
│   ├── filemanager.h / .cpp ← Export / Import selon le rôle
│   ├── sqlite3.h            ← Header SQLite (amalgamation)
│   └── sqlite3.c            ← Source SQLite (amalgamation)
└── README.md
```

---

## Prérequis

- **CLion** (IDE JetBrains)
- **CMake** >= 3.20
- **Compilateur g++** avec support C++17
- **Fichiers SQLite amalgamation** : `sqlite3.h` + `sqlite3.c`

---

## Installation

### 1. Télécharger SQLite

Aller sur **https://www.sqlite.org/download.html** → télécharger `sqlite-amalgamation-XXXX.zip` → extraire `sqlite3.h` et `sqlite3.c` → les coller dans le dossier `src/` du projet.

### 2. Configurer CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(Tp_C___)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

# SQLite inclus directement — pas de lib externe, pas de serveur
add_executable(Tp_C___
        src/sqlite3.c
        src/admin.cpp  src/admin.h
        src/database.cpp src/database.h
        src/filemanager.cpp src/filemanager.h
        src/main.cpp
        src/prof.cpp src/prof.h
        src/student.cpp src/student.h
        src/user.cpp src/user.h)
```

### 3. Compiler et lancer

Dans CLion : **File → Reload CMake Project** → 🔨 **(Build)** → ▶️ **Run 'Tp_C___'**

---

## Base de données

### Pourquoi pas de schema.sql ?

Contrairement à MySQL ou PostgreSQL, SQLite ne nécessite pas de serveur ni de fichier SQL à exécuter manuellement. Tout est géré directement dans le code C++ via la fonction `initSchema()` dans `database.cpp`.

### Comment ça fonctionne

Au lancement du programme, `main.cpp` appelle `db.connect()` qui appelle automatiquement `initSchema()`. Cette fonction crée les tables et insère les données de test si la base est vide :

```
▶️ Run
  └─ main() → db.connect()
                └─ sqlite3_open() → crée student_management.db
                └─ PRAGMA foreign_keys = ON → active les clés étrangères
                └─ initSchema()
                     └─ CREATE TABLE IF NOT EXISTS → 4 tables créées
                     └─ SELECT COUNT(*) → vérifie si la base est vide
                     └─ INSERT → données de test insérées si vide
```

Le fichier `student_management.db` apparaît automatiquement dans `cmake-build-debug/` après le premier lancement.

### Initialisation complète — ce qui est exécuté dans initSchema()

#### Création des tables

```sql
CREATE TABLE IF NOT EXISTS users (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    username   TEXT NOT NULL UNIQUE,
    password   TEXT NOT NULL,
    role       TEXT NOT NULL CHECK(role IN ('admin','prof','student')),
    email      TEXT,
    created_at TEXT DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS students (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    name       TEXT NOT NULL,
    email      TEXT UNIQUE,
    birthdate  TEXT,
    created_at TEXT DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS courses (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL,
    description TEXT,
    credits     INTEGER DEFAULT 3
);

CREATE TABLE IF NOT EXISTS grades (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id    INTEGER NOT NULL,
    course_id     INTEGER NOT NULL,
    grade         REAL NOT NULL CHECK(grade >= 0 AND grade <= 20),
    date_recorded TEXT DEFAULT (date('now')),
    FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,
    FOREIGN KEY (course_id)  REFERENCES courses(id)  ON DELETE CASCADE
);
```

`IF NOT EXISTS` garantit que les tables ne sont pas recréées à chaque lancement — si elles existent déjà, la commande est ignorée.

#### Données de test insérées automatiquement

```sql
-- 5 comptes utilisateurs
INSERT INTO users (username, password, role, email) VALUES
    ('admin',   'admin123',   'admin',   'admin@univ.fr'),
    ('dupont',  'prof456',    'prof',    'dupont@univ.fr'),
    ('alice',   'alice789',   'student', 'alice@etud.fr'),
    ('bob',     'bob101',     'student', 'bob@etud.fr'),
    ('charlie', 'charlie202', 'student', 'charlie@etud.fr');

-- 3 étudiants (liés aux comptes via l'email)
INSERT INTO students (name, email, birthdate) VALUES
    ('Alice Martin',  'alice@etud.fr',   '2002-03-15'),
    ('Bob Dupuis',    'bob@etud.fr',     '2001-07-22'),
    ('Charlie Leroy', 'charlie@etud.fr', '2003-01-10');

-- 5 cours
INSERT INTO courses (name, description, credits) VALUES
    ('Algorithmique',     'Introduction aux algorithmes', 4),
    ('Bases de donnees',  'Conception et requetes SQL',   4),
    ('Programmation C++', 'POO, STL, templates',          5),
    ('Reseaux',           'Protocoles TCP/IP',            3),
    ('Mathematiques',     'Analyse et algebre lineaire',  3);

-- 12 notes
INSERT INTO grades (student_id, course_id, grade) VALUES
    (1,1,15.5),(1,2,17.0),(1,3,14.5),(1,4,12.0),(1,5,16.0),
    (2,1,11.0),(2,2,13.5),(2,3,10.0),(2,4,14.0),
    (3,1,18.0),(3,3,19.5),(3,5,17.5);
```

Ces données ne sont insérées qu'une seule fois — si la table `users` n'est pas vide, l'insertion est ignorée.

### Relations entre tables

```
users ──────────────── students  (liés via email)
                           │
                           │ student_id (ON DELETE CASCADE)
                           ▼
                         grades
                           ▲
                           │ course_id (ON DELETE CASCADE)
                           │
                        courses
```

`ON DELETE CASCADE` signifie que supprimer un étudiant supprime automatiquement toutes ses notes. Idem pour un cours.

### Visualiser la base dans CLion

Pour voir les tables et données directement dans CLion :

1. **View → Tool Windows → Database**
2. **+** → **Data Source → SQLite**
3. Sélectionner `cmake-build-debug/student_management.db`
4. **Test Connection** → **OK**

---

## Comptes de test

| Login   | Mot de passe | Rôle    |
|---------|--------------|---------|
| admin   | admin123     | Admin   |
| dupont  | prof456      | Prof    |
| alice   | alice789     | Student |
| bob     | bob101       | Student |
| charlie | charlie202   | Student |

---

## Fonctionnalités par rôle

### 🔴 ADMIN
- Lister / Ajouter / Modifier / Supprimer des étudiants
- Lister / Ajouter / Supprimer des cours
- Lister / Ajouter / Modifier / Supprimer des notes
- Lister / Ajouter / Supprimer des utilisateurs
- Export complet (étudiants + cours + notes)
- Import complet depuis fichier texte

### 🔵 PROF
- Consulter la liste des étudiants
- Consulter les cours
- Ajouter / Modifier des notes
- Export des notes uniquement
- Import de notes uniquement

### 🟢 STUDENT
- Voir ses informations personnelles
- Voir ses notes
- Calculer sa moyenne générale avec mention
- Export de ses propres données uniquement

---

## Concepts C++ utilisés

| Concept | Où |
|---|---|
| Héritage | `Admin`, `Prof`, `Student` héritent de `User` |
| Polymorphisme | `showMenu()` virtuelle pure dans `User` |
| Classe abstraite | `User` — impossible de créer un objet `User` directement |
| STL `std::vector` | `ResultSet` — résultats SQL |
| STL `std::map` | `Row` — une ligne SQL = `map<string, string>` |
| `std::unique_ptr` | Gestion de l'objet `User` dans `authenticate()` |
| Encapsulation | Attributs `private` / `protected` + getters |
| Fichiers | `std::ifstream` / `std::ofstream` pour export/import |

---

## Format des fichiers import/export

### Format export (pipe-séparé)

```
--- ETUDIANTS ---
ID|Nom|Email|Date de naissance

--- COURS ---
ID|Nom|Description|Credits

--- NOTES ---
ID|Etudiant|Cours|Note|Date
```

### Import notes (Prof)

```
--- NOTES ---
student_id|course_id|note
1|3|14.5
2|3|12.0
```

---

## Commits Git recommandés

```bash
git init

git add src/user.h src/user.cpp
git commit -m "Add: base User class with role enum"

git add src/database.h src/database.cpp src/sqlite3.h src/sqlite3.c
git commit -m "Add: SQLite Database wrapper class with auto schema init"

git add src/admin.h src/admin.cpp
git commit -m "Add: Admin class with full CRUD access"

git add src/prof.h src/prof.cpp
git commit -m "Add: Prof class with grade management"

git add src/student.h src/student.cpp
git commit -m "Add: Student class with read-only access"

git add src/filemanager.h src/filemanager.cpp
git commit -m "Add: FileManager with role-based export/import"

git add src/main.cpp
git commit -m "Add: main entry point and authentication flow"

git add CMakeLists.txt README.md
git commit -m "Docs: CMake config and complete README"
```

---

## Architecture POO

```
           User (classe abstraite)
          /      |         \
       Admin    Prof     Student
         \       |         /
          \      |        /
           Database    FileManager
              |
           SQLite (.db)
```

---

*Projet réalisé dans le cadre du TP Noté C++ 2 — Gestion des données étudiantes*