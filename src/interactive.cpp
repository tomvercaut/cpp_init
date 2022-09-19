#include "cpp_init/interactive.h"

#include <iostream>
#include <optional>

namespace ci {

auto CreateSuperProject() -> std::unique_ptr<CommonParams>;
auto CreateLibraryQuestion() -> std::unique_ptr<CommonParams>;
auto CreateApplicationQuestion() -> std::unique_ptr<CommonParams>;

auto Question(std::string_view question) -> std::string {
  std::cout << question << ": ";
  std::string answer;
  std::cin >> answer;
  return answer;
}

auto QuestionUint8(std::string_view question) -> uint8_t {
  bool is_int{false};
  uint8_t value;
  do {
    std::cout << question << ": ";
    std::string tmp;
    std::cin >> tmp;
    try {
      value = std::stoi(tmp);
      is_int = true;
    } catch (std::invalid_argument&) {
      is_int = false;
    } catch (std::out_of_range&) {
      is_int = false;
    }
  } while (!is_int);
  return value;
}

auto QuestionOptions(std::string_view question,
                     const std::vector<std::string>& options) -> std::size_t {
  const auto size{options.size()};
  auto pos{size};
  bool is_int{false};
  do {
    std::cout << question << std::endl;
    for (std::size_t i{0}; i < size; ++i) {
      std::cout << (i + 1) << " " << options[i] << std::endl;
    }
    std::cout << "Answer [" << 1 << "-" << size << "]: ";
    std::string tmp;
    std::cin >> tmp;
    try {
      pos = std::stoi(tmp);
      if (pos > 0 && pos <= size) {
        --pos;
        is_int = true;
      }
    } catch (std::invalid_argument&) {
      is_int = false;
    } catch (std::out_of_range&) {
      is_int = false;
    }
  } while (!is_int);
  return pos;
}

[[maybe_unused]] auto YesNoQuestion(std::string_view question) -> bool {
  std::optional<bool> answer;
  auto isYesNo = [](const std::string& answer) -> std::optional<bool> {
    if (answer == "Y" || answer == "y" || answer == "Yes" || answer == "YES") {
      return {true};
    }
    if (answer == "N" || answer == "n" || answer == "No" || answer == "NO") {
      return {false};
    }
    return {};
  };
  std::string tmp;
  do {
    std::cout << question << " [y|n]: ";
    std::cin >> tmp;
    answer = isYesNo(tmp);
  } while (!answer.has_value());
  return answer.value();
}

auto CreateProjectQuestions() -> std::vector<std::unique_ptr<CommonParams>> {
  auto vec = std::vector<std::unique_ptr<CommonParams>>{};

  std::vector<std::string> options;
  options.emplace_back("Library");
  options.emplace_back("Application");
  options.emplace_back("Application with library");
  auto pos = QuestionOptions({"What would you like to create?"}, options);
  if (pos == 0) {
    if (auto rv = CreateLibraryQuestion(); rv) {
      vec.push_back(std::move(rv));
    }
  } else if (pos == 1) {
    if (auto rv = CreateApplicationQuestion(); rv) {
      vec.push_back(std::move(rv));
    }
  } else if (pos == 2) {
    auto super_project = CreateSuperProject();
    auto super_project_ptr = static_cast<SuperProjectParams*>(super_project.get());
    if (auto rv = CreateLibraryQuestion(); rv) {
      super_project_ptr->Add(static_cast<CommonParams*>(rv.get()));
      vec.push_back(std::move(rv));
    }
    if (auto rv = CreateApplicationQuestion(); rv) {
      super_project_ptr->Add(static_cast<CommonParams*>(rv.get()));
      vec.push_back(std::move(rv));
    }
    vec.push_back(std::move(super_project));
  }
  return vec;
}
auto CreateSuperProject() -> std::unique_ptr<CommonParams> {
  std::unique_ptr<CommonParams> ptr(new SuperProjectParams());
  auto* param = static_cast<SuperProjectParams*>(ptr.get());
  param->name = Question("Super project name");
  return ptr;
}
auto CreateApplicationQuestion() -> std::unique_ptr<CommonParams> {
  std::unique_ptr<CommonParams> ptr(new AppParams());
  auto* param = static_cast<AppParams*>(ptr.get());
  std::cout << "\n\n";
  param->name = Question("Application name");
  param->output_name = Question("Name of the executable");
  param->cmake_namespace = Question("CMake namespace");
  param->cpp_namespace = Question("C++ namespace");
  param->cpp_standard = QuestionUint8("CXX standard");
  return ptr;
}
auto CreateLibraryQuestion() -> std::unique_ptr<CommonParams> {
  std::unique_ptr<CommonParams> ptr(new LibraryParams());
  auto* param = static_cast<LibraryParams*>(ptr.get());
  std::cout << "\n\n";
  param->name = Question("Library name");
  param->alias = Question("CMake library alias");
  param->cmake_namespace = Question("CMake namespace");
  param->cpp_namespace = Question("C++ namespace");
  param->cpp_standard = QuestionUint8("CXX standard");
  return ptr;
}

}  // namespace ci