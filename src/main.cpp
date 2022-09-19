#include <iostream>

#include "cpp_init/generator.h"
#include "cpp_init/interactive.h"

int main(int argc, char** argv) {
  const auto projects = ci::CreateProjectQuestions();
  const auto current_path = std::filesystem::current_path();
  using opt_type = std::optional<const ci::SuperProjectParams*>;
  opt_type parent{};
  for (const auto & project : projects) {
    if(project && project->IsSuper()) {
      if(!parent) {
        const auto* ptr = static_cast<ci::SuperProjectParams*>(project.get());
        parent = opt_type {ptr};
      }
      ci::GenerateProject(current_path, project.get(), nullptr);
    }
  }
  for (const auto & project : projects) {
    if(project && project->IsLibrary()) {
      std::filesystem::path tmp{current_path};
      if (parent.has_value()) {
        tmp /= parent.value()->name;
      }
      ci::GenerateProject(tmp, project.get(), parent.value_or(nullptr));
    }
  }
  for (const auto & project : projects) {
    if(project && project->IsApplication()) {
      std::filesystem::path tmp{current_path};
      if (parent.has_value()) {
        tmp /= parent.value()->name;
      }
      ci::GenerateProject(tmp, project.get(), parent.value_or(nullptr));
    }
  }
  return EXIT_SUCCESS;
}