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

Argument::Argument(std::span<const char*> values) {
  std::copy(values.begin(), values.end(), std::back_inserter(m_values));
}

Argument::Argument(std::span<const std::string> values) {
  std::copy(values.begin(), values.end(), std::back_inserter(m_values));
}

std::size_t Argument::Size() const { return m_values.size(); }

template <>
std::string Argument::As<std::string>(std::size_t index) const {
  return m_values[index];
}

template <>
std::string Argument::As<std::string>() const {
  return As<std::string>(0);
}

template <>
int Argument::As<int>(std::size_t index) const {
  return std::stoi(m_values[index], nullptr);
}

template <>
int Argument::As<int>() const {
  return As<int>(0);
}

template <>
long Argument::As<long>(std::size_t index) const {
  return std::stoi(m_values[index], nullptr);
}

template <>
long Argument::As<long>() const {
  return As<long>(0);
}

template <>
float Argument::As<float>(std::size_t index) const {
  return std::stof(m_values[index], nullptr);
}

template <>
float Argument::As<float>() const {
  return As<float>(0);
}

template <>
double Argument::As<double>(std::size_t index) const {
  return std::stod(m_values[index], nullptr);
}

template <>
double Argument::As<double>() const {
  return As<double>(0);
}

void ArgumentMap::Add(const std::string& name, const Argument& arg) {
  m_map.emplace(name, arg);
}

ArgumentParser::ArgumentParser(const std::string& program_name,
                               const std::string& description)
    : m_program_name(program_name), m_program_description(description) {}

void ArgumentParser::GenerateHelp(std::initializer_list<std::string> flags) {
  for (const auto& flag : flags) {
    m_help_flags.insert(flag);
  }
}

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
    if (m_flags_map.contains(flag)) {
      throw std::runtime_error("Flag " + std::string{flag} + " redefined.");
    } else {
      m_flags_map.emplace(flag, optional);
    }
  }

  return optional;
}

const ArgumentMap ArgumentParser::Parse(int argc, const char* argv[]) {
  const auto args = env::GetArgs(argc, argv);
  return Parse(args);
}

const ArgumentMap ArgumentParser::Parse(std::span<const char*> args) {
  std::vector<std::string> str_args;
  std::copy(args.begin(), args.end(), std::back_inserter(str_args));
  return Parse(str_args);
}

const ArgumentMap ArgumentParser::Parse(std::span<const std::string> args) {
  ArgumentMap map;

  // TODO: Parse!
  (void)args;

  return map;
}

}  // namespace argparse
