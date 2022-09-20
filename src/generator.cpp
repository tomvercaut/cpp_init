#include "cpp_init/generator.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string_view>

namespace ci {

auto WriteProjectConfigCmake(const std::filesystem::path& cmake_path,
                             std::string_view name) -> uint8_t;

auto WriteCmakeHelpers(const std::filesystem::path& cmake_path) -> uint8_t;
auto WriteClangFormat(const std::filesystem::path& project_path,
                      bool has_parent) -> uint8_t;
auto WriteClangTidy(const std::filesystem::path& project_path, bool has_parent)
    -> uint8_t;
auto WriteAppNameHeader(const std::filesystem::path& project_path,
                        std::string_view ns) -> uint8_t;
auto WriteSrcMain(const std::filesystem::path& src_path) -> uint8_t;
auto WriteAppCMakeLists(const std::filesystem::path& project_path,
                        const AppParams* param,
                        const SuperProjectParams* parent) -> uint8_t;
auto WriteLibraryCMakeLists(const std::filesystem::path& ch,
                            const LibraryParams* param,
                            const SuperProjectParams* parent) -> uint8_t;
auto WriteSuperCMakeLists(const std::filesystem::path& project_path,
                          const SuperProjectParams* param) -> uint8_t;
auto WriteLibraryTestCMakeLists(const std::filesystem::path& test_path,
                                const LibraryParams* param) -> uint8_t;
auto WriteLibraryTestSrcMain(const std::filesystem::path& test_src_path,
                             const LibraryParams* param) -> uint8_t;
auto BuildTestOption(const CommonParams* param) -> std::string;

auto GenerateProject(const std::filesystem::path& working_dir,
                     CommonParams* param, const SuperProjectParams* parent)
    -> int32_t {
  int code{0};
  if (!is_directory(working_dir)) {
    std::cerr << "GenerateProject: working directory doesn't exist."
              << std::endl;
    return 1;
  }
  if (!param) {
    std::cerr << "GenerateProject: pointer to CommonParams is a nullptr."
              << std::endl;
    return 2;
  }

  // Create project directories
  std::vector<std::filesystem::path> dirs;

  std::filesystem::path project_path{working_dir};
  project_path.append(param->name);
  dirs.push_back(project_path);

  std::filesystem::path include_path{project_path};
  include_path.append("include");
  if (!param->IsSuper()) {
    dirs.push_back(include_path);
  }

  std::filesystem::path nested_include_path{include_path};
  nested_include_path.append(param->name);
  if (!param->IsSuper()) {
    dirs.push_back(nested_include_path);
  }

  std::filesystem::path src_path{project_path};
  src_path.append("src");
  if (!param->IsSuper()) {
    dirs.push_back(src_path);
  }

  std::filesystem::path cmake_path{project_path};
  cmake_path.append("cmake");
  if (param->IsSuper() || !param->has_parent) {
    dirs.push_back(cmake_path);
  }

  std::filesystem::path test_path{project_path};
  test_path.append("tests");
  if (param->IsLibrary()) {
    dirs.push_back(test_path);
  }

  std::filesystem::path test_src_path{test_path};
  test_src_path.append("src");
  if (param->IsLibrary()) {
    dirs.push_back(test_src_path);
  }

  for (const auto& dir : dirs) {
    if (!std::filesystem::exists(dir)) {
      std::error_code error_code;
      if (!std::filesystem::create_directories(dir, error_code)) {
        std::cerr << "GenerateProject: failed to create directory: " << dir
                  << "\n  Code: " << error_code.value()
                  << "\n  Message: " << error_code.message() << std::endl;
        return 3;
      }
    }
  }

  if (param->IsSuper() || !param->has_parent) {
    if (const auto rv = WriteProjectConfigCmake(cmake_path, param->name);
        rv != 0) {
      std::cerr << "GenerateProject: failed to generate project " << param->name
                << std::endl;
      return 4;
    }

    if (const auto rv = WriteCmakeHelpers(cmake_path); rv != 0) {
      std::cerr << "GenerateProject: failed to generate project " << param->name
                << std::endl;
      return 5;
    }
  }

  if (const auto rv = WriteClangFormat(project_path, param->has_parent);
      rv != 0) {
    std::cerr << "GenerateProject: failed to generate project " << param->name
              << std::endl;
    return 6;
  }

  if (const auto rv = WriteClangTidy(project_path, param->has_parent);
      rv != 0) {
    std::cerr << "GenerateProject: failed to generate project " << param->name
              << std::endl;
    return 7;
  }

  if (param->IsApplication()) {
    const auto* app_params = static_cast<const AppParams*>(param);
    if (const auto rv =
            WriteAppNameHeader(project_path, app_params->cpp_namespace);
        rv != 0) {
      std::cerr << "GenerateProject: failed to generate project " << param->name
                << std::endl;
      return 8;
    }
    if (const auto rv = WriteSrcMain(src_path); rv != 0) {
      std::cerr << "GenerateProject: failed to generate project " << param->name
                << std::endl;
      return 9;
    }
    if (const auto rv = WriteAppCMakeLists(project_path, app_params, parent); rv != 0) {
      std::cerr << "GenerateProject: failed to generate project " << param->name
                << std::endl;
      return 10;
    }
  }
  if (param->IsLibrary()) {
    const auto* lib_params = static_cast<const LibraryParams*>(param);
    if (const auto rv =
            WriteLibraryCMakeLists(project_path, lib_params, parent);
        rv != 0) {
      std::cerr << "GenerateProject: failed to generate project " << param->name
                << std::endl;
      return 11;
    }
    if (const auto rv = WriteLibraryTestCMakeLists(test_path, lib_params);
        rv != 0) {
      std::cerr << "GenerateProject: failed to generate project " << param->name
                << std::endl;
      return 12;
    }
    if (const auto rv = WriteLibraryTestSrcMain(test_src_path, lib_params);
        rv != 0) {
      std::cerr << "GenerateProject: failed to generate project " << param->name
                << std::endl;
      return 13;
    }
  }
  if (param->IsSuper()) {
    const auto* super_params = static_cast<const SuperProjectParams*>(param);
    if (const auto rv = WriteSuperCMakeLists(project_path, super_params);
        rv != 0) {
      std::cerr << "GenerateProject: failed to generate project " << param->name
                << std::endl;
      return 12;
    }
  }

  return code;
}

auto WriteProjectConfigCmake(const std::filesystem::path& cmake_path,
                             std::string_view name) -> uint8_t {
  if (!is_directory(cmake_path)) {
    std::cerr << "WriteProjectConfigCmake: CMake directory doesn't exist."
              << std::endl;
    return 1;
  }
  std::string tmp{name};
  tmp += "-config.cmake.in";
  std::filesystem::path cmake_file{cmake_path};
  cmake_file.append(tmp);

  std::ofstream out(cmake_file.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteProjectConfigCmake: failed to open " << cmake_file
              << std::endl;
    return 1;
  }
  out << R"(@PACKAGE_INIT@

if (NOT TARGET )";
  out << name;
  out << ")\n";
  out << "    include(${CMAKE_CURRENT_LIST_DIR}/";
  out << name;
  out << "-targets.cmake)\nendif()";
  out.close();
  if (out.is_open()) {
    std::cerr << "WriteProjectConfigCmake: failed to close " << cmake_file
              << std::endl;
    return 1;
  }
  return 0;
}

auto WriteCmakeHelpers(const std::filesystem::path& cmake_path) -> uint8_t {
  if (!is_directory(cmake_path)) {
    std::cerr << "WriteCmakeHelpers: CMake directory doesn't exist."
              << std::endl;
    return 1;
  }
  std::filesystem::path cmake_file{cmake_path};
  cmake_file.append("cmake_helpers.cmake");

  std::ofstream out(cmake_file.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteCmakeHelpers: failed to open " << cmake_file
              << std::endl;
    return 1;
  }
  out << R"(include(FetchContent)

FetchContent_Declare(
        ext_cmake_helpers
        GIT_REPOSITORY https://github.com/tomvercaut/cmake_helpers.git
        GIT_TAG main
)
FetchContent_MakeAvailable(ext_cmake_helpers)
)";
  out.close();
  if (out.is_open()) {
    std::cerr << "WriteCmakeHelpers: failed to close " << cmake_file
              << std::endl;
    return 1;
  }

  return 0;
}
auto WriteClangFormat(const std::filesystem::path& project_path,
                      bool has_parent) -> uint8_t {
  if (has_parent) {
    return 0;
  }
  if (!is_directory(project_path)) {
    std::cerr << "WriteClangFormat: project directory doesn't exist."
              << std::endl;
    return 1;
  }
  std::filesystem::path clang_file{project_path};
  clang_file.append(".clang-format");

  std::ofstream out(clang_file.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteClangFormat: failed to open " << clang_file << std::endl;
    return 1;
  }
  out << R"(
# Use the Google style in this project.
BasedOnStyle: Google

# Some folks prefer to write "int& foo" while others prefer "int &foo".  The
# Google Style Guide only asks for consistency within a project, we chose
# "int& foo" for this project:
DerivePointerAlignment: false
PointerAlignment: Left

# The Google Style Guide only asks for consistency w.r.t. "east const" vs.
# "const west" alignment of cv-qualifiers. In this project we use "const west".
QualifierAlignment: Left
)";
  out.close();
  if (out.is_open()) {
    std::cerr << "WriteClangFormat: failed to close " << clang_file
              << std::endl;
    return 1;
  }
  return 0;
}

auto WriteClangTidy(const std::filesystem::path& project_path, bool has_parent)
    -> uint8_t {
  if (!is_directory(project_path)) {
    std::cerr << "WriteClangTidy: project directory doesn't exist."
              << std::endl;
    return 1;
  }
  std::filesystem::path clang_file{project_path};
  clang_file.append(".clang-tidy");

  std::ofstream out(clang_file.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteClangTidy: failed to open " << clang_file << std::endl;
    return 1;
  }
  if (!has_parent) {
    out << R"(
Checks: >
  -*,
  abseil-*,
  bugprone-*,
  google-*,
  misc-*,
  modernize-*,
  performance-*,
  portability-*,
  readability-*
  # -google-readability-braces-around-statements,
  # -google-readability-namespace-comments,
  # -google-runtime-references,
  # -misc-non-private-member-variables-in-classes,
  # -modernize-return-braced-init-list,
  # -modernize-use-trailing-return-type,
  # -modernize-avoid-c-arrays,
  # -performance-move-const-arg,
  # -readability-braces-around-statements,
  # -readability-identifier-length,
  # -readability-magic-numbers,
  # -readability-named-parameter,
  # -readability-redundant-declaration,
  # -readability-function-cognitive-complexity,
  # -bugprone-narrowing-conversions,
  # -bugprone-easily-swappable-parameters,
  # -bugprone-implicit-widening-of-multiplication-result
# Turn all the warnings from the checks above into errors.
WarningsAsErrors: "*"

CheckOptions:
  - key:             readability-identifier-naming.ClassCase
    value:           CamelCase
  - key:             readability-identifier-naming.ClassMemberCase
    value:           lower_case
  - key:             readability-identifier-naming.ConstexprVariableCase
    value:           CamelCase
  - key:             readability-identifier-naming.ConstexprVariablePrefix
    value:           k
  - key:             readability-identifier-naming.EnumCase
    value:           CamelCase
  - key:             readability-identifier-naming.EnumConstantCase
    value:           CamelCase
  - key:             readability-identifier-naming.EnumConstantPrefix
    value:           k
  - key:             readability-identifier-naming.FunctionCase
    value:           CamelCase
  - key:             readability-identifier-naming.GlobalConstantCase
    value:           CamelCase
  - key:             readability-identifier-naming.GlobalConstantPrefix
    value:           k
  - key:             readability-identifier-naming.StaticConstantCase
    value:           CamelCase
  - key:             readability-identifier-naming.StaticConstantPrefix
    value:           k
  - key:             readability-identifier-naming.StaticVariableCase
    value:           lower_case
  - key:             readability-identifier-naming.MacroDefinitionCase
    value:           UPPER_CASE
  - key:             readability-identifier-naming.MacroDefinitionIgnoredRegexp
    value:           '^[A-Z]+(_[A-Z]+)*_$'
  - key:             readability-identifier-naming.MemberCase
    value:           lower_case
  - key:             readability-identifier-naming.PrivateMemberSuffix
    value:           _
  - key:             readability-identifier-naming.PublicMemberSuffix
    value:           ''
  - key:             readability-identifier-naming.NamespaceCase
    value:           lower_case
  - key:             readability-identifier-naming.ParameterCase
    value:           lower_case
  - key:             readability-identifier-naming.TypeAliasCase
    value:           CamelCase
  - key:             readability-identifier-naming.TypedefCase
    value:           CamelCase
  - key:             readability-identifier-naming.VariableCase
    value:           lower_case
  - key:             readability-identifier-naming.IgnoreMainLikeFunctions
    value:           1
)";
  } else {
    out << "InheritParentConfig: true\n";
  }
  out.close();
  if (out.is_open()) {
    std::cerr << "WriteClangTidy: failed to close " << clang_file << std::endl;
    return 1;
  }
  return 0;
}

auto WriteAppNameHeader(const std::filesystem::path& project_path,
                        std::string_view ns) -> uint8_t {
  if (!is_directory(project_path)) {
    std::cerr << "WriteAppNameHeader: project directory doesn't exist."
              << std::endl;
    return 1;
  }
  std::filesystem::path path{project_path};
  path.append("app_name.h.in");

  std::ofstream out(path.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteAppNameHeader: failed to open " << path << std::endl;
    return 1;
  }
  out << "namespace " << ns << "{\n";
  out << "    const char* APP_NAME{\"@F_APP_NAME@\"};\n}\n";
  out.close();
  if (out.is_open()) {
    std::cerr << "WriteAppNameHeader: failed to close " << path << std::endl;
    return 1;
  }
  return 0;
}

auto WriteSrcMain(const std::filesystem::path& src_path) -> uint8_t {
  if (!is_directory(src_path)) {
    std::cerr << "WriteSrcMain: source directory doesn't exist." << std::endl;
    return 1;
  }
  std::filesystem::path path{src_path};
  path.append("main.cpp");

  std::ofstream out(path.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteSrcMain: failed to open " << path << std::endl;
    return 1;
  }
  out << R"(#include <iostream>

int main(int argc, char** argv) {
  return EXIT_SUCCESS;
}
)";
  out.close();
  if (out.is_open()) {
    std::cerr << "WriteSrcMain: failed to close " << path << std::endl;
    return 1;
  }
  return 0;
}

auto WriteAppCMakeLists(const std::filesystem::path& project_path,
                        const AppParams* param, const SuperProjectParams* parent) -> uint8_t {
  if (!is_directory(project_path)) {
    std::cerr << "WriteAppCMakeLists: source directory doesn't exist."
              << std::endl;
    return 1;
  }
  if (param == nullptr) {
    std::cerr << "WriteAppCMakeLists: param is a nullptr" << std::endl;
    return 2;
  }
 if (param->has_parent && parent == nullptr) {
    std::cerr << "WriteLibraryCMakeLists: parent is a nullptr" << std::endl;
    return 3;
  }

  std::filesystem::path path{project_path};
  path.append("CMakeLists.txt");

  std::string test_option;
  if (param->has_parent) {
    test_option = BuildTestOption(parent);
  } else {
    test_option = BuildTestOption(param);
  }

  std::ofstream out(path.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteAppCMakeLists: failed to open " << path << std::endl;
    return 4;
  }
  if (!param->has_parent) {
    out << "cmake_minimum_required(VERSION ${CMAKE_VERSION})\n\n";
  }
  out << "project(" << param->name << "\n";
  out << "        LANGUAGES CXX\n";
  out << "        VERSION 0.0.1\n";
  out << "        )\n\n";

  if (!param->has_parent) {
    out << "include(cmake/cmake_helpers.cmake)\n\n";
  }

  if(!param->has_parent) {
    out << "option(" << test_option
      << " \"Build project tests\" ON)\n\n";
  }


  out << "add_app(\n";
  out << "        APP_NAME " << param->name << '\n';
  out << "        APP_CMAKE_NAMESPACE " << param->cmake_namespace << '\n';
  out << "        CXX_STANDARD " << static_cast<uint32_t>(param->cpp_standard) << '\n';
  out << "        APP_OUTPUT_NAME " << param->output_name << '\n';
  out << "        APP_VERSION ${PROJECT_VERSION}\n";
  out << "        APP_PRIVATE_INCLUDE_DIR\n            " << param->name << '\n';
  out << "        APP_PRIVATE_SOURCES\n            src/main.cpp\n";
  out << "        # APP_PUBLIC_SOURCES\n";
  out << "        # APP_PUBLIC_LIBRARIES\n";
  out << "        # APP_PRIVATE_LIBRARIES\n";
  out << "        # APP_PRIVATE_HEADERS\n";
  out << "        # APP_DEPENDENCIES\n";
  out << ")\n";

  out.close();
  if (out.is_open()) {
    std::cerr << "WriteAppCMakeLists: failed to close " << path << std::endl;
    return 4;
  }
  return 0;
}

auto WriteLibraryCMakeLists(const std::filesystem::path& project_path,
                            const LibraryParams* param,
                            const SuperProjectParams* parent) -> uint8_t {
  if (!is_directory(project_path)) {
    std::cerr << "WriteLibraryCMakeLists: project directory doesn't exist."
              << std::endl;
    return 1;
  }
  if (param == nullptr) {
    std::cerr << "WriteLibraryCMakeLists: param is a nullptr" << std::endl;
    return 2;
  }
  if (param->has_parent && parent == nullptr) {
    std::cerr << "WriteLibraryCMakeLists: parent is a nullptr" << std::endl;
    return 3;
  }
  std::filesystem::path path{project_path};
  path.append("CMakeLists.txt");

  std::string test_option;
  if (param->has_parent) {
    test_option = BuildTestOption(parent);
  } else {
    test_option = BuildTestOption(param);
  }

  std::ofstream out(path.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteLibraryCMakeLists: failed to open " << path << std::endl;
    return 4;
  }

  if (!param->has_parent) {
    out << "cmake_minimum_required(VERSION ${CMAKE_VERSION})\n\n";
  }
  out << "project(" << param->name << "\n";
  out << "        LANGUAGES CXX\n";
  out << "        VERSION 0.0.1\n";
  out << "        )\n\n";

  if (!param->has_parent) {
    out << "include(cmake/cmake_helpers.cmake)\n\n";
  }

  if(!param->has_parent) {
    out << "option(" << test_option
      << " \"Build project tests\" ON)\n\n";
  }

  out << "add_lib(\n";
  out << "        LIB_NAME " << param->name << "\n";
  out << "        LIB_CMAKE_NAMESPACE " << param->cmake_namespace << "\n";
  out << "        CXX_STANDARD " << static_cast<uint32_t>(param->cpp_standard) << '\n';
  out << "        LIB_ALIAS_NAME " << param->alias << "\n";
  out << "        LIB_VERSION ${PROJECT_VERSION}\n";
  out << "        # LIB_PUBLIC_HEADERS\n";
  out << "        # LIB_PRIVATE_SOURCES\n";
  out << "        # LIB_PUBLIC_LIBRARIES\n";
  out << "        # LIB_PRIVATE_LIBRARIES\n";
  out << "        # LIB_PRIVATE_HEADERS\n";
  out << ")\n";
  out << "if (${" << test_option << "})\n";
  out << "    add_subdirectory(tests)\nendif ()\n";
  out.close();
  if (out.is_open()) {
    std::cerr << "WriteLibraryCMakeLists: failed to close " << path
              << std::endl;
    return 4;
  }
  return 0;
}

auto WriteLibraryTestCMakeLists(const std::filesystem::path& test_path,
                                const LibraryParams* param) -> uint8_t {
  if (!is_directory(test_path)) {
    std::cerr << "WriteLibraryTestCMakeLists: tests directory doesn't exist."
              << std::endl;
    return 1;
  }
  if (param == nullptr) {
    std::cerr << "WriteLibraryTestCMakeLists: param is a nullptr" << std::endl;
    return 2;
  }
  std::filesystem::path path{test_path};
  path.append("CMakeLists.txt");
  std::ofstream out(path.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteLibraryTestCMakeLists: failed to open " << path
              << std::endl;
    return 3;
  }

  out << "find_package(Catch2 3 REQUIRED)\n\n";
  out << "add_catch2_test(\n";
  out << "        APP_NAME " << param->name << "_tests\n";
  out << "        CXX_STANDARD " << static_cast<uint32_t>(param->cpp_standard) << '\n';
  out << "        APP_DEPENDENCIES\n";
  out << "            " << param->cmake_namespace << "::" << param->alias
      << '\n';
  out << "        APP_PRIVATE_SOURCES\n";
  out << "            src/main.cpp\n";
  out << "        APP_PRIVATE_LIBRARIES\n";
  out << "            " << param->cmake_namespace << "::" << param->alias
      << '\n';
  out << "            Catch2::Catch2\n";
  out << ")\n";
  out.close();
  if (out.is_open()) {
    std::cerr << "WriteLibraryTestCMakeLists: failed to close " << path
              << std::endl;
    return 4;
  }
  return 0;
}

auto WriteSuperCMakeLists(const std::filesystem::path& project_path,
                          const SuperProjectParams* param) -> uint8_t {
  if (!is_directory(project_path)) {
    std::cerr << "WriteSuperCMakeLists: project directory doesn't exist."
              << std::endl;
    return 1;
  }
  if (param == nullptr) {
    std::cerr << "WriteSuperCMakeLists: param is a nullptr" << std::endl;
    return 2;
  }
  std::filesystem::path path{project_path};
  path.append("CMakeLists.txt");
  std::ofstream out(path.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteSuperCMakeLists: failed to open " << path << std::endl;
    return 3;
  }
  out << "cmake_minimum_required(VERSION ${CMAKE_VERSION})\n\n";
  out << "project(" << param->name << "\n";
  out << "        LANGUAGES CXX\n";
  out << "        VERSION 0.0.1\n";
  out << "        )\n\n";

  out << "include(cmake/cmake_helpers.cmake)\n\n";
  out << "option(" << BuildTestOption(param)
      << " \"Build project tests\" ON)\n\n";

  for (const auto& dir_name : param->sub_projects) {
    out << "add_subdirectory(" << dir_name << ")\n";
  }

  out.close();
  if (out.is_open()) {
    std::cerr << "WriteSuperCMakeLists: failed to close " << path << std::endl;
    return 4;
  }
  return 0;
}

auto WriteLibraryTestSrcMain(const std::filesystem::path& test_src_path,
                             const LibraryParams* param) -> uint8_t {
  if (!is_directory(test_src_path)) {
    std::cerr << "WriteLibraryTestSrcMain: tests src directory doesn't exist."
              << std::endl;
    return 1;
  }
  if (param == nullptr) {
    std::cerr << "WriteLibraryTestSrcMain: param is a nullptr" << std::endl;
    return 2;
  }
  std::filesystem::path path{test_src_path};
  path.append("main.cpp");
  std::ofstream out(path.string(), std::ios_base::out);
  if (!out.is_open()) {
    std::cerr << "WriteLibraryTestSrcMain: failed to open " << path
              << std::endl;
    return 3;
  }
  out << R"(#include <catch2/catch_session.hpp>

auto main(int argc, char* argv[]) -> int {
  Catch::Session session;  // There must be exactly one instance

  // writing to session.configData() here sets defaults
  // this is the preferred way to set them

  int returnCode = session.applyCommandLine(argc, argv);
  if (returnCode != 0) {  // Indicates a command line error
    return returnCode;
  }

  // writing to session.configData() or session.Config() here
  // overrides command line args
  // only do this if you know you need to

  int numFailed = session.run();

  // numFailed is clamped to 255 as some unices only use the lower 8 bits.
  // This clamping has already been applied, so just return it here
  // You can also do any post run clean-up here
  return numFailed;
}
)";
  out.close();
  if (out.is_open()) {
    std::cerr << "WriteLibraryTestSrcMain: failed to close " << path
              << std::endl;
    return 4;
  }
  return 0;
}
auto BuildTestOption(const CommonParams* param) -> std::string {
  if (param == nullptr) {
    return "";
  }
  std::string upper{param->name};
  std::transform(upper.cbegin(), upper.cend(), upper.begin(),
                 [](const auto c) { return std::toupper(c); });
  std::string s{"BUILD_"};
  s.append(upper);
  s.append("_TESTS");
  return s;
}

}  // namespace ci