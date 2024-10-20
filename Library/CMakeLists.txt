# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Define the project name and specify C++ as the language
project(LibraryProject LANGUAGES CXX)

# Add executable and link the source files
add_executable(Library "Library.cpp" ${TARGET_SRC} "Defines.cpp")


if (LINUX)
    configure_file(users users COPYONLY)
    configure_file(books books COPYONLY)
    configure_file(config cofig COPYONLY)
endif()

# If CMake version is greater than 3.12, set the C++ standard to C++14
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Library PROPERTY CXX_STANDARD 14)
endif()

# List all header files here (optional but good practice for IDEs and clarity)
set(TARGET_H
    Book.h
    UserInterface.h
    LibrarySystem.h
    Reader.h
    Server.h
    Defines.h
)

# List all source files here
set(TARGET_SRC
    Book.cpp
    UserInterface.cpp
    LibrarySystem.cpp
    Reader.cpp
    Server.cpp
    Defines.cpp
)

set_source_files_properties(${PROTOBUF_FILES} PROPERTIES 
    COMPILE_DEFINITIONS _SCL_SECURE_NO_WARNINGS 
    COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS
    COMPILE_FLAGS /W2
)

# Add all source files to the target
target_sources(Library PRIVATE ${TARGET_SRC})

# Optionally, you can add tests or installation targets here
