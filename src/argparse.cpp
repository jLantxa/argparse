#include "argparse.hpp"

#include <cstdlib>

namespace argparse {

namespace env {

[[nodiscard]] const std::span<const char*> GetArgs(int argc,
                                                   const char* argv[]) {
  return {argv, static_cast<std::size_t>(argc)};
}

}  // namespace env

Positional::Positional(const std::string& _name)
    : name(_name), nargs(NArgs::NUMERIC), num_args(1) {
  if (name.empty()) {
    throw std::runtime_error("Arguments cannot have an empty name.");
  } else if (name.starts_with('-')) {
    throw std::runtime_error("Positional arguments cannot start with '-'");
  }
}

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

static bool IsValidFlagName(const std::string& flag) {
  // TODO: Check that flag contains no whitespace
  return flag.starts_with("-");
}

Optional::Optional(const std::string& _name, const std::string& f1,
                   const std::string& f2)
    : name(_name),
      flag1(f1),
      flag2(f2),
      required(false),
      nargs(NArgs::OPTIONAL),
      num_args(0) {
  // HINT: f2 can be optionally empty

  if (_name.empty()) {
    throw std::runtime_error("Arguments cannot have an empty name.");
  }

  if (f1.empty() && !f2.empty()) {
    throw std::runtime_error("Optional flags cannot have an empty name.");
  }

  const bool valid_flag_markers =
      (f1.empty() || (!f1.empty() && IsValidFlagName(f1))) &&
      (f2.empty() || (!f2.empty() && IsValidFlagName(f2)));

  if (!valid_flag_markers) {
    throw std::runtime_error("Optional arguments must start with '-' or '--'");
  }
}

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

Positional& ArgumentParser::AddPositional(const std::string& name) {
  Positional& positional = m_positionals.emplace_back(name);
  return positional;
}

Optional& ArgumentParser::AddOptional(const std::string& name,
                                      const std::string& flag1,
                                      const std::string& flag2) {
  Optional& optional = m_optionals.emplace_back(name, flag1, flag2);
  return optional;
}

}  // namespace argparse
