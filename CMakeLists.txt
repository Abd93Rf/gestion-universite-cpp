cmake_minimum_required(VERSION 3.20)
project(Tp_C___)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

# SQLite est inclus directement dans le projet (sqlite3.h + sqlite3.c)
add_executable(Tp_C___
        src/sqlite3.c
        src/admin.cpp
        src/admin.h
        src/database.cpp
        src/database.h
        src/filemanager.cpp
        src/filemanager.h
        src/main.cpp
        src/prof.cpp
        src/prof.h
        src/student.cpp
        src/student.h
        src/user.cpp
        src/user.h)