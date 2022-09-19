#ifndef CXX_PROJECT_CREATOR_INCLUDE_CPP_INIT_GENERATOR_H
#define CXX_PROJECT_CREATOR_INCLUDE_CPP_INIT_GENERATOR_H

#include <cstdint>
#include <filesystem>

#include "cpp_init/params.h"

namespace ci {

auto GenerateProject(const std::filesystem::path& working_dir,
                     CommonParams* param, const SuperProjectParams* parent)
    -> int32_t;

}  // namespace ci

#endif  // CXX_PROJECT_CREATOR_INCLUDE_CPP_INIT_GENERATOR_H
