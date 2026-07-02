#pragma once

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>

class TempDir {
public:
  TempDir();

  // Not copyable or movable
  TempDir(const TempDir &) = delete;
  TempDir &operator=(const TempDir &) = delete;

  ~TempDir();

  // Call this to keep the temp directory after destruction
  void keepTempDir() { keep_ = true; }

  const std::filesystem::path &path() const { return path_; }

private:
  std::filesystem::path path_;
  bool keep_;
};
