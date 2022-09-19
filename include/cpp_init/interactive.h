#ifndef CXX_PROJECT_CREATOR_INCLUDE_CPP_INIT_INTERACTIVE_H
#define CXX_PROJECT_CREATOR_INCLUDE_CPP_INIT_INTERACTIVE_H

#include <memory>
#include <string_view>
#include <vector>

#include "cpp_init/params.h"

namespace ci {

auto Question(std::string_view question) -> std::string;
auto QuestionUint8(std::string_view string) -> uint8_t;
auto QuestionOptions(std::string_view question, const std::vector<std::string>& options) -> std::size_t;
[[maybe_unused]] auto YesNoQuestion(std::string_view question) -> bool;

auto CreateProjectQuestions() -> std::vector<std::unique_ptr<CommonParams>>;

}

#endif  // CXX_PROJECT_CREATOR_INCLUDE_CXX_PROJECT_CREATOR_INTERACTIVE_H
