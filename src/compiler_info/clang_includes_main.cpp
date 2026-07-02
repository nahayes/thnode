#include <string.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "compiler_info/clang_includes.h"
#include "util/logging.h"

using std::string;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr
        << "Error: provide a path to clang or clang++ as the only argument."
        << std::endl;
    return 1;
  }
  string CompilerPathStr = argv[1];
  std::filesystem::path CompilerPath(CompilerPathStr);
  string Basename = CompilerPath.filename().string();

  if (Basename != "clang" && Basename != "clang++") {
    std::cerr
        << "Error: first argument must be a path to clang or clang++, got '"
        << Basename << "'." << std::endl;
    return 1;
  }

  if (!CompilerPath.is_absolute()) {
    std::cerr << "Error: clang path must be an absolute path. Got '"
              << CompilerPath << "'." << std::endl;
    return 1;
  }

  if (!std::filesystem::exists(CompilerPath)) {
    std::cerr << "Error: the provided clang path '" << CompilerPath
              << "' does not exist." << std::endl;
    return 1;
  }

  auto IncludePaths =
      clang_include_path::getIncludePathsForCompiler(CompilerPath, false);
  for (const auto &Path : IncludePaths) {
    std::cout << Path << std::endl;
  }

  return 0;
}
