# cpp_init

`cpp_init` is a basic C++ project creator. The commandline application generates a directory structure specific for that type of project. It supports 3 types of projects:
- Library
- Application
- Application with a separate library

A CMakeLists.txt file is added to the project in function of the type of project that was selected. The configuration uses CMake functions from the [cmake_helpers](https://github.com/tomvercaut/cmake_helpers) project to reduce boilerplate code.

In case a library project is selected, a separate test project using Catch2 v3 is added.