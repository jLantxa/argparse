#include "argparse.hpp"

#include <cstdlib>

namespace argparse {

namespace env {

[[nodiscard]] const std::span<const char *> GetArgs(int argc,
                                                    const char *argv[]) {
  return {argv, static_cast<std::size_t>(argc)};
}

}  // namespace env

Positional::Positional(const std::string& _name)
: name(_name),
  nargs(NArgs::NUMERIC),
  num_args(1)
{}

Positional& Positional::NumArgs(std::size_t num) {
  if (num == 0) {
    throw std::runtime_error("NumArgs cannot be 0 for Positional arguments.");
  }

  nargs = NArgs::NUMERIC;
  num_args = num;

  return *this;
}

Positional& Positional::NumArgs(NArgs num) {
  nargs = num;
  return *this;
}

Positional& Positional::Help(const std::string& help_str) {
  help = help_str;
  return *this;
}

std::pair<NArgs, std::size_t> Positional::GetNArgs() const {
  return {nargs, num_args};
}


Optional::Optional(const std::string& _name, const std::string& f1, const std::string& f2)
: name(_name),
  flag1(f1),
  flag2(f2),
  required(false),
  nargs(NArgs::OPTIONAL),
  num_args(0)
{}

Optional& Optional::NumArgs(std::size_t num) {
  nargs = NArgs::NUMERIC;
  num_args = num;

  return *this;
}

Optional& Optional::NumArgs(NArgs num) {
  nargs = num;
  return *this;
}

Optional& Optional::Required(bool req) {
  required = req;
  return *this;
}

Optional& Optional::Help(const std::string& help_str) {
  help = help_str;
  return *this;
}

std::pair<NArgs, std::size_t> Optional::GetNArgs() const {
  return {nargs, num_args};
}

bool Optional::HasFlag(const std::string& flag) const {
  const bool has_flag = ((flag1 == flag) || (flag2 == flag));
  return has_flag;
}

}  // namespace argparse
