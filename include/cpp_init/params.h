#ifndef CXX_PROJECT_CREATOR_INCLUDE_CXX_PROJECT_CREATOR_PARAMS_H
#define CXX_PROJECT_CREATOR_INCLUDE_CXX_PROJECT_CREATOR_PARAMS_H

#include <string>

namespace ci {

class CommonParams {
 public:
  virtual ~CommonParams() = default;

  virtual auto IsLibrary() const -> bool { return false; }
  virtual auto IsApplication() const -> bool { return false; }
  virtual auto IsSuper() const -> bool { return false; }

  std::string name;
  uint8_t cpp_standard{0};
  bool has_parent{false};
};

class SuperProjectParams : public CommonParams {
 public:
  ~SuperProjectParams() override = default;

  auto IsSuper() const -> bool { return true; }
  auto Add(CommonParams* param) {
    if (param == nullptr) {
      return;
    }
    param->has_parent = true;
    sub_projects.push_back(param->name);
  }

  std::vector<std::string> sub_projects;
};

class LibraryParams : public CommonParams {
 public:
  ~LibraryParams() override = default;

  auto IsLibrary() const -> bool { return true; }

  std::string cmake_namespace;
  std::string cpp_namespace;
  std::string alias;
};

class AppParams : public CommonParams {
 public:
  ~AppParams() override = default;

  auto IsApplication() const -> bool { return true; }

  std::string cmake_namespace;
  std::string cpp_namespace;
  std::string output_name;
};

}  // namespace ci

#endif  // CXX_PROJECT_CREATOR_INCLUDE_CXX_PROJECT_CREATOR_PARAMS_H
