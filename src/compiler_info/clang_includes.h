#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace clang_include_path {

// getIncludePathsForCompiler returns the include paths for the given compiler.
// ClangPath is the path to the clang or clang++ executable.
// If addDashI is true, the include paths are returned with the -I flag.
//
// This is useful for getting the include paths when running a compiler plugin.
std::vector<std::string>
getIncludePathsForCompiler(const std::filesystem::path &ClangPath,
                           bool AddDashI);

} // namespace clang_include_path
