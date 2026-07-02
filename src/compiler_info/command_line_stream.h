#pragma once

#include <cstdio>
#include <cstring>
#include <string>

#include "util/logging.h"

namespace compiler_info {

// Class CommandLineStream runs a shell command and provides an iterator over
// the lines of output.
//
// For now it is hard-coded with the settings needed to run clang to get include
// paths.  Stdin is an empty file.  Standard error is merged with standard out.
// It would be easy to make these settings configurable.
class CommandLineStream {
public:
  explicit CommandLineStream(const std::string &Cmd)
      : pipe_(nullptr), ended_(false) {
    pipe_ = popen(Cmd.c_str(), "r");
  }

  // Implements a method to explicitly close the pipe if needed.
  void close() {
    if (pipe_) {
      pclose(pipe_);
      pipe_ = NULL;
      ended_ = true;
    }
  }

  ~CommandLineStream() { close(); }

  // Iterator over the lines of output.
  class Iterator {
  public:
    Iterator() : stream_(NULL), valid_(false) {}
    Iterator(CommandLineStream *Stream) : stream_(Stream), valid_(true) {
      ++(*this); // Load the first line.
    }

    Iterator &operator++() {
      if (!stream_ || !stream_->pipe_ || stream_->ended_) {
        valid_ = false;
        return *this;
      }
      current_line_.clear();
      bool SawNewline = false;
      do {
        char Buffer[4096];
        if (fgets(Buffer, sizeof(Buffer), stream_->pipe_) == nullptr) {

          // No more input.
          valid_ = false;
          stream_->ended_ = true;
          if (current_line_.empty()) {
            return *this;
          }
          break;
        }
        current_line_ += Buffer;
        // Check if last char is newline
        size_t Len = strlen(Buffer);
        if (Len > 0 && Buffer[Len - 1] == '\n') {
          SawNewline = true;
          break;
        }
      } while (true);

      // Remove trailing '\n' if present
      if (!current_line_.empty() && current_line_.back() == '\n') {
        current_line_.pop_back();
      }

      valid_ = !current_line_.empty();
      return *this;
    }

    std::string operator*() const { return current_line_; }

    bool operator!=(const Iterator &Other) const {
      // End condition: either one is invalid
      return valid_ != Other.valid_;
    }

  private:
    CommandLineStream *stream_;
    std::string current_line_;
    bool valid_ = false;
  };

  Iterator begin() { return Iterator(this); }
  Iterator end() { return Iterator(); }

  bool good() const { return pipe_ != nullptr && !ended_; }

private:
  FILE *pipe_;
  bool ended_;
};

} // namespace compiler_info
