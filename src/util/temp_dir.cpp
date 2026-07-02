#include "temp_dir.h"
#include <vector>

TempDir::TempDir() : keep_(false) {
  // Create a unique temporary directory.

  // Get the system temporary directory.
  std::filesystem::path SysTemp = std::filesystem::temp_directory_path();
  std::string Tmpl = (SysTemp / "temp-thnode-XXXXXX").string();

  // mkdtemp mutates a char array in-place.
  std::vector<char> Buf(Tmpl.begin(), Tmpl.end());
  Buf.push_back('\0');

  char *Result = mkdtemp(Buf.data());
  if (!Result) {
    throw std::runtime_error("Failed to create temporary directory");
  }

  path_ = Result;
}

TempDir::~TempDir() {
  if (!keep_) {
    // Remove the temp dir and all its contents.
    std::error_code Ec;
    std::filesystem::remove_all(path_, Ec);
  }
}
