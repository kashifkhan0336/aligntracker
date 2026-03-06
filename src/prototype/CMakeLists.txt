include(FetchContent)

# You can configure this for your need, presumbably you want speificy a git tag here instead of a branch
FetchContent_Declare(sqliteOrm
  GIT_REPOSITORY https://github.com/fnc12/sqlite_orm
  GIT_TAG        v1.9.1
)
FetchContent_MakeAvailable(sqliteOrm)
add_library(sqlite3_lib STATIC ${CMAKE_SOURCE_DIR}/vendor/sqlite/sqlite3.c)
add_executable(prototyping prototyping.cpp)
target_link_libraries(prototyping sqlite3_lib sqlite_orm::sqlite_orm)
target_include_directories(prototyping PRIVATE ${CMAKE_SOURCE_DIR}/vendor/sqlite/)