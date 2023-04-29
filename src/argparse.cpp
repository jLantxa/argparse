#include "argparse.hpp"

#include <algorithm>
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
  return (!flag.empty() && flag.starts_with("-"));
}

Optional::Optional(const std::string& _name,
                   std::initializer_list<std::string> flag_list)
    : name(_name), required(false), nargs(NArgs::OPTIONAL), num_args(0) {
  if (_name.empty()) {
    throw std::runtime_error("Arguments cannot have an empty name.");
  }

  for (const auto& flag : flag_list) {
    if (!IsValidFlagName(flag)) {
      throw std::runtime_error(
          "Invalid flag name. Flags must start with '-' or '--'");
    }

    flags.push_back(flag);
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
  bool has_flag =
      (std::find(flags.cbegin(), flags.cend(), flag) != flags.cend());
  return has_flag;
}

ArgumentParser::ArgumentParser(const std::string& program_name,
                               const std::string& description)
    : m_program_name(program_name), m_program_description(description) {}

Positional& ArgumentParser::AddPositional(const std::string& name) {
  if (m_names.contains(name)) {
    throw std::runtime_error("Argument name " + std::string{name} +
                             " redefined.");
  }
  m_names.insert(name);

  Positional& positional = m_positionals.emplace_back(name);
  return positional;
}

Optional& ArgumentParser::AddOptional(
    const std::string& name, std::initializer_list<std::string> flags) {
  if (m_names.contains(name)) {
    throw std::runtime_error("Argument name " + std::string{name} +
                             " redefined.");
  }

  Optional& optional = m_optionals.emplace_back(name, flags);
  m_names.insert(name);


  for (const auto& flag : flags) {
    if (m_flags_set.contains(flag)) {
      throw std::runtime_error("Flag " + std::string{flag} + " redefined.");
    } else {
      m_flags_set.insert(flag);
      m_flags_map.emplace(flag, optional);
    }
  }

  return optional;
}

}  // namespace argparse
