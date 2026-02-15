# Explication des fichiers — Système de Gestion Étudiant C++

---

## 1️⃣ User.h / User.cpp — La classe de base

C'est le point de départ de tout le projet. Avant de coder Admin, Prof ou Student, il faut définir ce qu'est un utilisateur en général.

---

### User.h

#### L'enum class Role

```cpp
enum class Role {
    ADMIN,
    PROF,
    STUDENT
};
```

`enum class Role` c'est une liste de valeurs possibles pour le rôle. Au lieu d'utiliser des strings comme `"admin"` partout dans le code (ce qui peut provoquer des fautes de frappe), on utilise `Role::ADMIN`, `Role::PROF`, `Role::STUDENT`. C'est plus sûr et plus propre.

---

#### Les attributs protected

```cpp
class User {
protected:
    int id;
    std::string username;
    std::string password;
    Role role;
```

Les attributs sont en `protected` et pas `private` — ça veut dire que les classes enfants (Admin, Prof, Student) peuvent y accéder directement. Si c'était `private`, elles ne pourraient pas.

---

#### La méthode virtuelle pure — cœur du polymorphisme

```cpp
virtual void showMenu() = 0;
```

C'est la ligne la plus importante du fichier. `virtual` + `= 0` ça veut dire que c'est une **méthode virtuelle pure**. Conséquences :

- `User` devient une **classe abstraite** — on ne peut pas créer un objet `User` directement
- Chaque classe enfant est **obligée** de redéfinir `showMenu()`
- C'est le **polymorphisme** — même appel `user->showMenu()`, comportement différent selon le rôle

---

### User.cpp

#### La liste d'initialisation dans le constructeur

```cpp
User::User(int id, const std::string& username, 
           const std::string& password, Role role)
    : id(id), username(username), password(password), role(role) {}
```

C'est le constructeur avec une **liste d'initialisation** (`: id(id), ...`). C'est la bonne façon de faire en C++ plutôt que d'assigner dans le corps du constructeur — c'est plus efficace.

---

#### getRoleName()

```cpp
std::string User::getRoleName() const {
    switch (role) {
        case Role::ADMIN:   return "Admin";
        case Role::PROF:    return "Professeur";
        case Role::STUDENT: return "Etudiant";
    }
}
```

Convertit l'enum en texte lisible pour l'affichage.

---

### En résumé

| Concept | Où | Explication |
|---|---|---|
| `enum class` | `enum class Role` | Définit les rôles proprement |
| `protected` | Attributs `id`, `username`, `role` | Les enfants peuvent accéder aux attributs |
| `virtual = 0` | `virtual void showMenu() = 0` | Force chaque rôle à avoir son propre menu |
| Classe abstraite | — | On ne peut pas faire `User u;` directement |

---

## 2️⃣ Database.h / Database.cpp — La connexion à la base de données

C'est la classe qui fait le lien entre le code C++ et SQLite. Tout le reste du projet (Admin, Prof, Student) passe par elle pour lire et écrire des données.

---

### Database.h

#### Alias STL pour simplifier le code

```cpp
using Row       = std::map<std::string, std::string>;
using ResultSet = std::vector<Row>;
```

Ces deux lignes créent des **alias de types** pour simplifier le code.

- `Row` c'est une ligne SQL représentée comme un dictionnaire : `{"id": "1", "name": "Alice", "email": "alice@etud.fr"}`
- `ResultSet` c'est un tableau de lignes, donc le résultat complet d'un SELECT

Au lieu d'écrire `std::vector<std::map<std::string, std::string>>` partout, on écrit juste `ResultSet`. C'est l'utilisation de la **STL** que le prof veut voir.

---

```cpp
class Database {
private:
    sqlite3*    db;
    std::string dbPath;
```

`sqlite3*` c'est un pointeur vers la connexion SQLite. C'est la bibliothèque qui gère tout le moteur de base de données en interne. `dbPath` c'est juste le chemin vers le fichier `.db`.

---

### Database.cpp

#### connect() — ce qui se passe au démarrage

```cpp
bool Database::connect() {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    execute("PRAGMA foreign_keys = ON;");
    initSchema();
    return true;
}
```

Trois choses se passent à la connexion. D'abord `sqlite3_open` ouvre ou crée le fichier `.db`. Ensuite `PRAGMA foreign_keys = ON` active les clés étrangères dans SQLite (désactivées par défaut). Enfin `initSchema()` crée les tables et insère les données de test si c'est le premier lancement.

---

#### La fonction callback — comment SQLite retourne les données

```cpp
static int queryCallback(void* data, int argc, 
                         char** argv, char** colNames) {
    auto* results = static_cast<ResultSet*>(data);
    Row row;
    for (int i = 0; i < argc; ++i)
        row[colNames[i]] = argv[i] ? argv[i] : "NULL";
    results->push_back(row);
    return 0;
}
```

C'est une fonction **callback** — SQLite l'appelle automatiquement pour chaque ligne de résultat. Pour chaque ligne, on crée une `Row` (map), on remplit chaque colonne, et on l'ajoute au `ResultSet`. Le `argv[i] ? argv[i] : "NULL"` gère les valeurs nulles en SQL.

---

#### query() — pour les SELECT

```cpp
ResultSet Database::query(const std::string& sql) {
    ResultSet results;
    sqlite3_exec(db, sql.c_str(), queryCallback, &results, &errMsg);
    return results;
}
```

C'est la méthode pour les **SELECT**. Elle exécute la requête, le callback remplit `results`, et on retourne le tout. Exemple d'utilisation :

```cpp
auto rows = db.query("SELECT * FROM students");
// rows[0]["name"]  → "Alice Martin"
// rows[0]["email"] → "alice@etud.fr"
```

---

#### execute() — pour INSERT / UPDATE / DELETE

```cpp
bool Database::execute(const std::string& sql) {
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
}
```

C'est pour les **INSERT, UPDATE, DELETE** — on n'a pas besoin de récupérer des résultats, juste savoir si ça a marché.

---

#### escape() — protection contre les injections SQL

```cpp
std::string Database::escape(const std::string& value) {
    for (char c : value) {
        if (c == '\'') result += "''";
        else result += c;
    }
}
```

Protection basique contre les **injections SQL**. Si un utilisateur tape `alice' OR '1'='1` comme mot de passe, le `'` sera transformé en `''` et la requête ne sera pas cassée.

---

#### initSchema() — création automatique des tables

```cpp
void Database::initSchema() {
    execute(R"(CREATE TABLE IF NOT EXISTS users (...))");
    
    auto rows = query("SELECT COUNT(*) AS nb FROM users;");
    if (rows[0]["nb"] == "0") {
        execute("INSERT INTO users ...");  // données de test
    }
}
```

`IF NOT EXISTS` garantit qu'on ne recrée pas les tables si elles existent déjà. On vérifie si la table `users` est vide avant d'insérer les données de test — comme ça on ne les duplique pas à chaque lancement.

---

### En résumé

| Méthode | Rôle | Utilisée pour |
|---|---|---|
| `connect()` | Ouvre le fichier `.db` | Au démarrage |
| `query()` | SELECT | Lire des données |
| `execute()` | INSERT / UPDATE / DELETE | Modifier des données |
| `escape()` | Sécurité | Toutes les entrées utilisateur |
| `initSchema()` | Crée tables + données test | Premier lancement |

---

## 3️⃣ Admin.h / Admin.cpp — Accès complet

C'est la classe la plus complète du projet. L'Admin peut tout faire : gérer les étudiants, les cours, les notes et les utilisateurs.

---

### Admin.h

#### Héritage et référence à la base

```cpp
class Admin : public User {
private:
    Database& db;
```

Deux choses importantes ici. `public User` signifie qu'Admin **hérite** de User — il récupère automatiquement les attributs `id`, `username`, `password`, `role` et les méthodes `getUsername()`, `getRole()` etc. sans les réécrire.

`Database& db` c'est une **référence** vers la base de données. On utilise `&` et pas une copie car on veut tous travailler sur la même connexion. Si on faisait `Database db` sans `&`, on créerait une deuxième connexion séparée ce qui serait une erreur.

---

```cpp
void showMenu() override;
```

`override` dit au compilateur : "je redéfinis la méthode virtuelle de la classe parent". C'est le **polymorphisme** en action. Si on fait une faute de frappe dans le nom de la méthode, le compilateur le détecte grâce à `override`.

---

### Admin.cpp — Le constructeur

```cpp
Admin::Admin(int id, const std::string& username, 
             const std::string& password, Database& db)
    : User(id, username, password, Role::ADMIN), db(db) {}
```

On appelle le constructeur parent `User(...)` avec `Role::ADMIN` pour initialiser les attributs hérités. Ensuite on initialise `db(db)` avec la référence à la base.

---

### Admin.cpp — Le menu

```cpp
void Admin::showMenu() {
    int choice = 0;
    do {
        std::cout << "  [1] Gerer les etudiants\n";
        std::cout << "  [2] Gerer les cours\n";
        // ...
        std::cin >> choice;

        switch (choice) {
            case 1: listStudents(); break;
            case 2: addStudent();   break;
            // ...
        }
    } while (choice != 0);
}
```

La boucle `do...while` fait tourner le menu jusqu'à ce que l'admin choisisse `0` pour se déconnecter. Le `switch` redirige vers la bonne méthode selon le choix.

---

### Admin.cpp — Les opérations SQL

#### Lister les étudiants (SELECT)

```cpp
void Admin::listStudents() {
    auto rows = db.query(
        "SELECT id, name, email, birthdate FROM students ORDER BY name");
    
    for (auto& row : rows) {
        std::cout << row["id"]    << " | " 
                  << row["name"]  << " | "
                  << row["email"] << "\n";
    }
}
```

`auto& row` c'est une référence vers chaque ligne du résultat. On accède aux colonnes par leur nom comme un dictionnaire. `ORDER BY name` trie par ordre alphabétique.

---

#### Ajouter un étudiant (INSERT)

```cpp
void Admin::addStudent() {
    std::string name, email, birthdate;
    std::cout << "Nom : "; std::getline(std::cin, name);
    std::cout << "Email : "; std::getline(std::cin, email);

    std::string sql = "INSERT INTO students (name, email, birthdate) VALUES ('"
        + db.escape(name) + "', '"
        + db.escape(email) + "', '"
        + db.escape(birthdate) + "')";

    if (db.execute(sql))
        std::cout << "Etudiant ajoute (ID=" << db.getLastInsertId() << ")\n";
}
```

On utilise `db.escape()` sur chaque valeur entrée par l'utilisateur pour éviter les injections SQL. `getLastInsertId()` retourne l'ID généré automatiquement par le `AUTOINCREMENT`.

---

#### Modifier un étudiant (UPDATE)

```cpp
void Admin::updateStudent() {
    int id;
    std::cout << "ID etudiant a modifier : "; std::cin >> id;

    std::string sql = "UPDATE students SET name='" + db.escape(name)
        + "' WHERE id=" + std::to_string(id);

    db.execute(sql);
}
```

`std::to_string(id)` convertit l'entier en string pour construire la requête SQL. Le `WHERE id=` garantit qu'on modifie uniquement le bon étudiant.

---

#### Supprimer un étudiant (DELETE)

```cpp
void Admin::deleteStudent() {
    int id;
    std::cout << "ID a supprimer : "; std::cin >> id;

    db.execute("DELETE FROM students WHERE id=" + std::to_string(id));
}
```

Grâce au `ON DELETE CASCADE` défini dans le schéma SQL, supprimer un étudiant supprime automatiquement toutes ses notes dans `grades`. Pas besoin de le faire manuellement.

---

### En résumé

| Concept | Où dans Admin |
|---|---|
| Héritage | `class Admin : public User` |
| Polymorphisme | `showMenu() override` |
| Référence | `Database& db` |
| SELECT | `listStudents()`, `listCourses()`, `listGrades()` |
| INSERT | `addStudent()`, `addCourse()`, `addGrade()` |
| UPDATE | `updateStudent()`, `updateGrade()` |
| DELETE | `deleteStudent()`, `deleteCourse()`, `deleteGrade()` |

---

## 4️⃣ Prof.h / Prof.cpp — Accès limité aux notes

La classe Prof ressemble à Admin mais avec des **permissions réduites**. C'est là qu'on voit concrètement la gestion des rôles.

---

### Prof.h

```cpp
class Prof : public User {
private:
    Database& db;

public:
    void showMenu() override;

    void listStudents();   // Consulter seulement
    void listCourses();    // Consulter seulement
    void listGrades();     // Consulter seulement
    void addGrade();       // Autorisé
    void updateGrade();    // Autorisé
};
```

Comparé à Admin, le Prof n'a **pas** de méthodes comme `addStudent()`, `deleteStudent()`, `addCourse()`, `deleteUser()`. Cette restriction est imposée directement dans le code — même si quelqu'un essaie de bidouiller, les méthodes n'existent tout simplement pas.

---

### Prof.cpp — Le constructeur

```cpp
Prof::Prof(int id, const std::string& username,
           const std::string& password, Database& db)
    : User(id, username, password, Role::PROF), db(db) {}
```

Identique à Admin mais avec `Role::PROF`. C'est la seule différence dans le constructeur.

---

### Prof.cpp — Le menu

```cpp
void Prof::showMenu() {
    int choice = 0;
    do {
        std::cout << "  [1] Voir les etudiants\n";
        std::cout << "  [2] Voir les cours\n";
        std::cout << "  [3] Voir les notes\n";
        std::cout << "  [4] Ajouter une note\n";
        std::cout << "  [5] Modifier une note\n";
        std::cout << "  [0] Deconnexion\n";
    } while (choice != 0);
}
```

Le menu du Prof a **5 options** contre 4 sous-menus complets pour l'Admin. Il peut voir les étudiants et cours mais pas les modifier. Il peut seulement ajouter et modifier des notes.

---

### Prof.cpp — Consulter les étudiants

```cpp
void Prof::listStudents() {
    auto rows = db.query(
        "SELECT id, name, email FROM students ORDER BY name");

    for (auto& row : rows) {
        std::cout << row["id"]    << " | "
                  << row["name"]  << " | "
                  << row["email"] << "\n";
    }
}
```

Le Prof voit les étudiants mais la requête SELECT ne récupère **pas** la date de naissance contrairement à l'Admin. C'est une restriction supplémentaire au niveau des données affichées.

---

### Prof.cpp — Ajouter une note

```cpp
void Prof::addGrade() {
    listStudents();   // Affiche la liste pour choisir
    int sId;
    std::cout << "ID etudiant : "; std::cin >> sId;

    listCourses();    // Affiche les cours pour choisir
    int cId;
    std::cout << "ID cours : "; std::cin >> cId;

    std::string grade;
    std::cout << "Note (0-20) : "; std::getline(std::cin, grade);

    std::string sql = "INSERT INTO grades (student_id, course_id, grade) VALUES ("
        + std::to_string(sId) + ", "
        + std::to_string(cId) + ", "
        + db.escape(grade) + ")";

    if (db.execute(sql))
        std::cout << "Note ajoutee.\n";
}
```

On affiche d'abord la liste des étudiants et des cours pour que le Prof puisse choisir les bons IDs. C'est une bonne pratique UX — l'utilisateur ne doit pas deviner les IDs de mémoire.

---

### Prof.cpp — Modifier une note

```cpp
void Prof::updateGrade() {
    listGrades();   // Affiche toutes les notes avec leurs IDs
    int id;
    std::cout << "ID de la note a modifier : "; std::cin >> id;

    std::string grade;
    std::cout << "Nouvelle note : "; std::getline(std::cin, grade);

    std::string sql = "UPDATE grades SET grade=" + db.escape(grade)
        + " WHERE id=" + std::to_string(id);

    db.execute(sql);
}
```

Le Prof peut modifier n'importe quelle note, pas seulement les siennes. Si le prof voulait voir seulement ses propres cours, il faudrait ajouter un `WHERE prof_id = ...` mais ça nécessiterait une table de liaison cours-prof qu'on a pas dans ce projet.

---

### Comparaison Admin vs Prof

| Action | Admin | Prof |
|---|---|---|
| Voir étudiants | ✅ complet | ✅ limité |
| Ajouter étudiant | ✅ | ❌ |
| Supprimer étudiant | ✅ | ❌ |
| Voir notes | ✅ | ✅ |
| Ajouter note | ✅ | ✅ |
| Modifier note | ✅ | ✅ |
| Supprimer note | ✅ | ❌ |
| Gérer utilisateurs | ✅ | ❌ |

---

### En résumé

La classe Prof démontre deux niveaux de restriction. Le premier au niveau des **méthodes** — certaines n'existent tout simplement pas dans la classe. Le second au niveau des **requêtes SQL** — les SELECT ne retournent pas toutes les colonnes. C'est exactement ce que le prof veut voir comme gestion des permissions.

---

## 5️⃣ Student.h / Student.cpp — Lecture seule

C'est la classe la plus restrictive. L'étudiant ne peut que **consulter** ses propres données, rien d'autre. C'est le troisième niveau de permission du système.

---

### Student.h

```cpp
class Student : public User {
private:
    Database& db;
    int studentId;  // ID dans la table students (≠ ID dans users)

public:
    Student(int userId, const std::string& username, 
            const std::string& password,
            Database& db, int studentId);

    void showMenu() override;

    void viewMyInfo();     // Voir ses infos personnelles
    void viewMyGrades();   // Voir ses notes
    void viewMyAverage();  // Calculer sa moyenne
};
```

La grosse différence avec Admin et Prof c'est `int studentId`. Un étudiant a **deux IDs différents** dans la base :

- `id` dans la table `users` — c'est son ID de connexion
- `studentId` dans la table `students` — c'est son ID académique

Ces deux IDs sont liés via l'email. On a besoin des deux car les notes sont stockées avec `student_id` qui référence la table `students`, pas `users`.

---

### Student.cpp — Le constructeur

```cpp
Student::Student(int userId, const std::string& username,
                 const std::string& password,
                 Database& db, int studentId)
    : User(userId, username, password, Role::STUDENT), 
      db(db), studentId(studentId) {}
```

On passe `studentId` en plus des paramètres habituels. C'est récupéré dans `main.cpp` au moment de l'authentification via une requête SQL sur l'email.

---

### Student.cpp — Le menu

```cpp
void Student::showMenu() {
    int choice = 0;
    do {
        std::cout << "  [1] Mes informations\n";
        std::cout << "  [2] Mes notes\n";
        std::cout << "  [3] Ma moyenne generale\n";
        std::cout << "  [0] Deconnexion\n";
    } while (choice != 0);
}
```

Seulement **3 options** contre 5 pour le Prof et des dizaines pour l'Admin. C'est la démonstration la plus claire de la gestion des rôles — le menu lui-même est restreint.

---

### Student.cpp — Voir ses informations

```cpp
void Student::viewMyInfo() {
    auto rows = db.query(
        "SELECT name, email, birthdate FROM students "
        "WHERE id=" + std::to_string(studentId));

    auto& row = rows[0];
    std::cout << "Nom       : " << row["name"]      << "\n";
    std::cout << "Email     : " << row["email"]     << "\n";
    std::cout << "Naissance : " << row["birthdate"] << "\n";
}
```

Le `WHERE id=" + std::to_string(studentId)` est la clé de sécurité ici. L'étudiant ne peut voir **que ses propres données** — il est impossible d'accéder aux infos d'un autre étudiant car le `studentId` est fixé à la connexion et ne peut pas être changé.

---

### Student.cpp — Voir ses notes

```cpp
void Student::viewMyGrades() {
    auto rows = db.query(
        "SELECT c.name AS course, g.grade, g.date_recorded "
        "FROM grades g "
        "JOIN courses c ON g.course_id = c.id "
        "WHERE g.student_id = " + std::to_string(studentId) +
        " ORDER BY c.name");

    for (auto& row : rows) {
        std::cout << row["course"] << " | "
                  << row["grade"]  << " | "
                  << row["date_recorded"] << "\n";
    }
}
```

C'est la requête SQL la plus complexe du projet — une **jointure** entre deux tables. Sans le `JOIN`, on aurait juste les IDs des cours, pas leurs noms. Le `JOIN courses c ON g.course_id = c.id` dit : "pour chaque note, va chercher le nom du cours correspondant dans la table courses".

---

### Student.cpp — Calculer sa moyenne

```cpp
void Student::viewMyAverage() {
    auto rows = db.query(
        "SELECT AVG(grade) AS avg FROM grades "
        "WHERE student_id=" + std::to_string(studentId));

    double avg = std::stod(rows[0]["avg"]);
    std::cout << "Moyenne : " << std::fixed 
              << std::setprecision(2) << avg << " / 20\n";

    if (avg >= 16)      std::cout << "Mention : Tres Bien\n";
    else if (avg >= 14) std::cout << "Mention : Bien\n";
    else if (avg >= 12) std::cout << "Mention : Assez Bien\n";
    else if (avg >= 10) std::cout << "Mention : Passable\n";
    else                std::cout << "Mention : Insuffisant\n";
}
```

`AVG(grade)` c'est une fonction d'agrégation SQL — elle calcule la moyenne directement en base sans avoir à le faire en C++. `std::stod()` convertit le résultat string en `double`. `std::fixed` et `std::setprecision(2)` affichent exactement 2 décimales.

---

### Comparaison des trois rôles

| Action | Admin | Prof | Student |
|---|---|---|---|
| Voir tous les étudiants | ✅ | ✅ | ❌ |
| Voir ses propres infos | ✅ | ✅ | ✅ |
| Modifier des données | ✅ | ✅ notes | ❌ |
| Supprimer des données | ✅ | ❌ | ❌ |
| Voir toutes les notes | ✅ | ✅ | ❌ |
| Voir ses propres notes | ✅ | ✅ | ✅ |
| Calculer sa moyenne | ✅ | ✅ | ✅ |

---

### En résumé

La classe Student démontre trois choses importantes. D'abord le **double ID** qui lie les tables `users` et `students`. Ensuite la **restriction par WHERE** dans chaque requête SQL qui garantit qu'un étudiant ne voit que ses propres données. Enfin la **jointure SQL** dans `viewMyGrades()` qui montre la maîtrise des relations entre tables.

---

## 6️⃣ FileManager.h / FileManager.cpp — Export / Import de fichiers

C'est la classe qui gère la lecture et l'écriture de fichiers texte. Le contenu exporté/importé dépend du rôle de l'utilisateur connecté.

---

### FileManager.h

```cpp
class FileManager {
private:
    Database& db;

    // Méthodes privées — utilisées en interne uniquement
    void exportAll(const std::string& filename);
    void exportGradesOnly(const std::string& filename, int studentId = -1);
    void exportStudentInfo(const std::string& filename, int studentId);

    void importAll(const std::string& filename);
    void importGradesOnly(const std::string& filename);

public:
    explicit FileManager(Database& db);

    // Méthodes publiques — appelées depuis main.cpp
    void exportData(User& user, int studentId = -1);
    void importData(User& user);
};
```

La structure est en deux niveaux. Les méthodes **publiques** `exportData()` et `importData()` sont le point d'entrée — elles reçoivent l'utilisateur et décident quoi faire selon son rôle. Les méthodes **privées** font le vrai travail — elles sont cachées car personne d'autre n'a besoin de les appeler directement.

`int studentId = -1` c'est un **paramètre par défaut** — si on ne passe pas de studentId, il vaut -1 par défaut. C'est utile pour les cas Admin et Prof qui n'ont pas besoin de studentId.

---

### FileManager.cpp — Le routage selon le rôle

```cpp
void FileManager::exportData(User& user, int studentId) {
    std::string filename;
    std::cout << "Nom du fichier d'export : ";
    std::getline(std::cin, filename);

    switch (user.getRole()) {
        case Role::ADMIN:
            exportAll(filename);
            break;
        case Role::PROF:
            exportGradesOnly(filename);
            break;
        case Role::STUDENT:
            exportStudentInfo(filename, studentId);
            break;
    }
}
```

C'est le **polymorphisme sans héritage** — on utilise le rôle de l'objet `User` pour décider quelle méthode appeler. Le `switch` sur `user.getRole()` redirige vers la bonne fonction selon qui est connecté. L'Admin exporte tout, le Prof exporte les notes, l'étudiant exporte seulement ses données.

---

### FileManager.cpp — Export complet (Admin)

```cpp
void FileManager::exportAll(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier.\n";
        return;
    }

    file << "--- ETUDIANTS ---\n";
    file << "ID|Nom|Email|Date de naissance\n";
    auto students = db.query("SELECT id, name, email, birthdate FROM students");
    for (auto& row : students) {
        file << row["id"]        << "|"
             << row["name"]      << "|"
             << row["email"]     << "|"
             << row["birthdate"] << "\n";
    }

    file << "\n--- COURS ---\n";
    // ... même principe pour les cours

    file << "\n--- NOTES ---\n";
    // ... même principe pour les notes

    file.close();
    std::cout << "Export complet OK\n";
}
```

`std::ofstream` c'est la classe C++ pour écrire dans un fichier. `file.is_open()` vérifie que le fichier a bien été créé avant d'écrire dedans. Le format **pipe-séparé** `|` est choisi car c'est un caractère rare dans les noms et emails — moins de risques de conflit qu'une virgule.

---

### FileManager.cpp — Export notes seulement (Prof)

```cpp
void FileManager::exportGradesOnly(const std::string& filename, int studentId) {
    std::ofstream file(filename);

    file << "--- NOTES ---\n";
    file << "ID|Etudiant|Cours|Note|Date\n";

    std::string sql =
        "SELECT g.id, s.name AS student, c.name AS course, "
        "g.grade, g.date_recorded "
        "FROM grades g "
        "JOIN students s ON g.student_id = s.id "
        "JOIN courses  c ON g.course_id  = c.id";

    if (studentId > 0)
        sql += " WHERE g.student_id = " + std::to_string(studentId);

    auto grades = db.query(sql);
    for (auto& row : grades) {
        file << row["id"]            << "|"
             << row["student"]       << "|"
             << row["course"]        << "|"
             << row["grade"]         << "|"
             << row["date_recorded"] << "\n";
    }

    file.close();
}
```

La condition `if (studentId > 0)` permet de réutiliser cette méthode pour deux cas différents. Si `studentId` est -1 (valeur par défaut), on exporte toutes les notes. Si c'est un ID valide, on filtre sur un étudiant précis. C'est ce qu'on appelle la **réutilisabilité du code**.

---

### FileManager.cpp — Export infos étudiant (Student)

```cpp
void FileManager::exportStudentInfo(const std::string& filename, int studentId) {
    std::ofstream file(filename);

    auto info = db.query(
        "SELECT name, email, birthdate FROM students "
        "WHERE id=" + std::to_string(studentId));

    file << "=== MES INFORMATIONS ===\n";
    file << "Nom   : " << info[0]["name"]  << "\n";
    file << "Email : " << info[0]["email"] << "\n";

    file << "\n=== MES NOTES ===\n";
    auto grades = db.query(
        "SELECT c.name AS course, g.grade FROM grades g "
        "JOIN courses c ON g.course_id = c.id "
        "WHERE g.student_id = " + std::to_string(studentId));

    for (auto& row : grades)
        file << row["course"] << "|" << row["grade"] << "\n";

    file.close();
}
```

L'étudiant n'exporte **que ses propres données** — le `WHERE id=studentId` garantit qu'il ne peut pas exporter les données d'un autre étudiant, même en bidouillant.

---

### FileManager.cpp — Import complet (Admin)

```cpp
void FileManager::importAll(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Fichier introuvable.\n";
        return;
    }

    std::string section, line;
    int imported = 0;

    while (std::getline(file, line)) {
        if (line == "--- ETUDIANTS ---") { section = "students"; continue; }
        if (line == "--- COURS ---")     { section = "courses";  continue; }
        if (line == "--- NOTES ---")     { section = "grades";   continue; }

        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, '|')) 
            fields.push_back(field);

        if (section == "students" && fields.size() == 4) {
            db.execute("INSERT OR IGNORE INTO students "
                       "(id, name, email, birthdate) VALUES (...)");
            ++imported;
        }
    }

    std::cout << imported << " ligne(s) importee(s).\n";
}
```

`std::ifstream` c'est la classe pour lire un fichier. `std::getline(file, line)` lit ligne par ligne. `std::stringstream` avec `std::getline(ss, field, '|')` découpe chaque ligne selon le séparateur `|` — c'est l'équivalent d'un `split()` en Python. `INSERT OR IGNORE` évite les doublons si la donnée existe déjà.

---

### Résumé des permissions d'export/import

| Action | Admin | Prof | Student |
|---|---|---|---|
| Export étudiants | ✅ | ❌ | ❌ |
| Export cours | ✅ | ❌ | ❌ |
| Export notes | ✅ toutes | ✅ toutes | ✅ les siennes |
| Export infos perso | ✅ | ✅ | ✅ |
| Import complet | ✅ | ❌ | ❌ |
| Import notes | ✅ | ✅ | ❌ |

---

### En résumé

FileManager démontre trois concepts importants. D'abord la **lecture/écriture de fichiers** avec `ifstream` et `ofstream`. Ensuite le **routage par rôle** avec le `switch` sur `user.getRole()`. Enfin le **parsing de fichier texte** avec `stringstream` pour découper les lignes.

---

## 7️⃣ main.cpp — Le chef d'orchestre

C'est le point d'entrée du programme. Il ne fait pas grand chose lui-même — son rôle c'est de coordonner tous les autres : ouvrir la base, authentifier l'utilisateur, créer le bon objet et lancer le bon menu.

---

### Les includes

```cpp
#include <iostream>
#include <string>
#include <memory>

#include "database.h"
#include "user.h"
#include "admin.h"
#include "prof.h"
#include "student.h"
#include "filemanager.h"
```

Les premiers includes sont des bibliothèques **standard C++**. `iostream` pour `cout/cin`, `string` pour les chaînes de caractères, `memory` pour `unique_ptr`. Les suivants sont nos propres classes — on les inclut tous car `main.cpp` doit pouvoir créer n'importe quel type d'objet.

---

### L'authentification — la fonction la plus importante

```cpp
std::unique_ptr<User> authenticate(Database& db) {
    std::string username, password;
    std::cout << "Login    : "; std::getline(std::cin, username);
    std::cout << "Password : "; std::getline(std::cin, password);
```

Le type de retour `std::unique_ptr<User>` est très important. C'est un **pointeur intelligent** — il gère automatiquement la mémoire. Quand l'objet n'est plus utilisé, il est supprimé automatiquement sans qu'on ait besoin de faire `delete`. On retourne un `User*` et pas directement `Admin`, `Prof` ou `Student` car on ne sait pas encore quel type créer — ça dépend du rôle dans la base.

---

```cpp
    std::string sql = 
        "SELECT id, username, role FROM users "
        "WHERE username='" + db.escape(username) + 
        "' AND password='" + db.escape(password) + "'";

    auto rows = db.query(sql);

    if (rows.empty()) {
        std::cout << "Identifiants incorrects.\n";
        return nullptr;
    }
```

La requête vérifie simultanément le login **et** le mot de passe en une seule requête SQL. Si aucune ligne n'est retournée, les identifiants sont mauvais et on retourne `nullptr`. `nullptr` c'est le pointeur nul en C++ — ça signifie "aucun objet".

---

```cpp
    int uid = std::stoi(rows[0]["id"]);
    std::string role = rows[0]["role"];
```

`std::stoi()` convertit la string `"1"` en entier `1`. Les résultats SQL sont toujours des strings dans notre système, donc il faut convertir quand on a besoin d'un nombre.

---

```cpp
    if (role == "admin")
        return std::make_unique<Admin>(uid, username, password, db);

    if (role == "prof")
        return std::make_unique<Prof>(uid, username, password, db);

    if (role == "student") {
        auto sRows = db.query(
            "SELECT id FROM students WHERE email=("
            "SELECT email FROM users WHERE id=" 
            + std::to_string(uid) + ")");

        int studentId = sRows.empty() ? -1 : std::stoi(sRows[0]["id"]);
        return std::make_unique<Student>(uid, username, password, db, studentId);
    }
```

C'est le cœur du **polymorphisme**. Selon le rôle lu dans la base, on crée un objet différent — `Admin`, `Prof` ou `Student`. Mais tous sont retournés comme `unique_ptr<User>`. C'est possible grâce à l'héritage — Admin, Prof et Student **sont** des User.

Pour le Student, on fait une **sous-requête SQL** pour récupérer son `studentId` via l'email. La requête imbriquée `SELECT email FROM users WHERE id=...` récupère l'email, puis la requête externe cherche l'étudiant avec cet email.

`sRows.empty() ? -1 : std::stoi(sRows[0]["id"])` c'est un **opérateur ternaire** — raccourci pour un if/else : si vide retourne -1, sinon retourne l'ID.

---

### La boucle principale

```cpp
int main() {
    showBanner();

    Database db("student_management.db");

    if (!db.connect()) {
        std::cerr << "Impossible d'ouvrir la base.\n";
        return 1;
    }

    FileManager fm(db);
    bool running = true;

    while (running) {
        std::cout << "[1] Se connecter   [0] Quitter\nChoix : ";
        int choice; std::cin >> choice; std::cin.ignore();

        if (choice == 0) {
            running = false;
        } else if (choice == 1) {
            auto user = authenticate(db);
            if (user) user->showMenu();
        }
    }

    db.disconnect();
    return 0;
}
```

`Database db("student_management.db")` crée la base SQLite — juste un fichier, pas de serveur. `if (!db.connect())` avec `return 1` arrête proprement le programme si la base ne s'ouvre pas — `return 1` signifie "erreur" en C++, `return 0` signifie "succès".

`auto user = authenticate(db)` retourne un `unique_ptr<User>` qui pointe vers un Admin, Prof ou Student selon le login. La ligne suivante `user->showMenu()` c'est le **polymorphisme en action** — un seul appel, trois comportements différents selon le type réel de l'objet.

`std::cin.ignore()` après `std::cin >> choice` c'est nécessaire pour vider le buffer du clavier — sinon le `\n` restant perturbe le `getline()` suivant.

`db.disconnect()` à la fin ferme proprement la connexion SQLite avant que le programme se termine.

---

### Le flux complet du programme

```
main() démarre
    ↓
Database::connect() — ouvre student_management.db
    ↓
initSchema() — crée tables + données si premier lancement
    ↓
Boucle principale
    ↓
authenticate() — vérifie login/password dans users
    ↓
Crée Admin / Prof / Student selon le rôle
    ↓
user->showMenu() — polymorphisme, chaque rôle a son menu
    ↓
Actions SQL selon les droits
    ↓
Déconnexion → retour à la boucle
```

---

### En résumé

| Concept | Où dans main.cpp | Explication |
|---|---|---|
| `unique_ptr<User>` | Retour de `authenticate()` | Pointeur intelligent, gestion mémoire auto |
| `make_unique<Admin>()` | Création selon le rôle | Polymorphisme à la création |
| `user->showMenu()` | Boucle principale | Un appel, trois comportements |
| Sous-requête SQL | Récupération du `studentId` | Lien entre `users` et `students` via email |
| `return 0 / 1` | `main()` | Convention succès/erreur en C++ |
| `cin.ignore()` | Après `cin >> choice` | Vider le buffer clavier |

---

## 🎯 Vision globale du projet

Maintenant que tous les fichiers sont expliqués, voilà comment ils s'articulent ensemble :

```
main.cpp
  │
  ├── Database ←──────────────────┐
  │     └── SQLite (fichier .db)  │
  │                               │
  ├── authenticate()              │
  │     └── crée Admin/Prof/Student
  │                               │
  ├── User (classe abstraite)     │
  │     ├── Admin ────────────────┤
  │     ├── Prof  ────────────────┤
  │     └── Student ──────────────┘
  │
  └── FileManager
        └── export/import selon le rôle
```