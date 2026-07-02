#include "clang_includes.h"

#include "compiler_info/command_line_stream.h"
#include "util/logging.h"
#include "util/string_util.h"

using std::string;
using std::vector;

namespace clang_include_path {

vector<string>
getIncludePathsForCompiler(const std::filesystem::path &ClangPath,
                           bool AddDashI) {
  // Construct the command to get include paths.
  std::string Cmd;

  std::string Basename = ClangPath.filename().string();
  if (Basename == "clang") {
    Cmd = ClangPath.string() + " -v -E -";
  } else if (Basename == "clang++") {
    Cmd = ClangPath.string() + " -v -E -x c++ -";
  } else {
    FATAL << "Unreachable; basename = " << Basename;
  }

  // Redirect stdin to /dev/null to simulate empty stdin for the command.
  // Redirect stderr to stdout to get both as one stream of lines.
  Cmd += " < /dev/null 2>&1";

  compiler_info::CommandLineStream Stream(Cmd);
  std::vector<std::string> IncludePaths;
  bool InSearch = false;

  // Extract the include paths from command output.
  for (const std::string &Line : Stream) {
    // Skip everything before "search starts here:"
    if (Line.find("search starts here:") != std::string::npos) {
      InSearch = true;
      continue;
    }
    // Stop after "End of search list"
    if (InSearch && Line.find("End of search list") != std::string::npos) {
      break;
    }

    if (InSearch) {
      std::string Trimmed = trimWhitespace(Line);
      if (!Trimmed.empty() && Trimmed[0] == '/') {
        if (AddDashI) {
          Trimmed = "-I " + Trimmed;
        }
        IncludePaths.push_back(Trimmed);
      }
    }
  }
  return IncludePaths;
}

} // namespace clang_include_path
