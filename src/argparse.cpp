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

Optional::Optional(std::initializer_list<std::string> flag_list)
    : required(false), nargs(NArgs::OPTIONAL), num_args(0) {
  for (const auto& flag : flag_list) {
    if (!IsValidFlagName(flag)) {
      throw std::runtime_error(
          "Invalid flag name. Flags must start with '-' or '--'");
    }

    flags.push_back(flag);
  }
}

Optional::Optional(const std::string& flag)
    : Optional(std::initializer_list<std::string>{flag}) {}

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

static constexpr auto StrToInt = [](const std::string& str) -> int {
  return std::stoi(str, nullptr);
};

static constexpr auto StrToLong = [](const std::string& str) -> long {
  return std::stol(str, nullptr);
};

static constexpr auto StrToFloat = [](const std::string& str) -> float {
  return std::stof(str, nullptr);
};

static constexpr auto StrToDouble = [](const std::string& str) -> double {
  return std::stod(str, nullptr);
};

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
std::vector<std::string> Argument::AsVector<std::string>() const {
  return m_values;
}

template <>
int Argument::As<int>(std::size_t index) const {
  return StrToInt(m_values[index]);
}

template <>
std::vector<int> Argument::AsVector<int>() const {
  std::vector<int> values;
  std::transform(m_values.cbegin(), m_values.cend(), std::back_inserter(values),
                 StrToInt);
  return values;
}

template <>
long Argument::As<long>(std::size_t index) const {
  return StrToLong(m_values[index]);
}

template <>
std::vector<long> Argument::AsVector<long>() const {
  std::vector<long> values;
  std::transform(m_values.cbegin(), m_values.cend(), std::back_inserter(values),
                 StrToLong);
  return values;
}

template <>
float Argument::As<float>(std::size_t index) const {
  return StrToFloat(m_values[index]);
}

template <>
std::vector<float> Argument::AsVector<float>() const {
  std::vector<float> values;
  std::transform(m_values.cbegin(), m_values.cend(), std::back_inserter(values),
                 StrToFloat);
  return values;
}

template <>
double Argument::As<double>(std::size_t index) const {
  return StrToDouble(m_values[index]);
}

template <>
std::vector<double> Argument::AsVector<double>() const {
  std::vector<double> values;
  std::transform(m_values.cbegin(), m_values.cend(), std::back_inserter(values),
                 StrToDouble);
  return values;
}

void ArgumentMap::Add(const std::string& name, const Argument& arg) {
  m_map.emplace(name, arg);
}

bool ArgumentMap::Has(const std::string& name) const {
  return m_map.contains(name);
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
  if (m_positional_names.contains(name)) {
    throw std::runtime_error("Argument name " + std::string{name} +
                             " redefined.");
  }
  m_positional_names.insert(name);

  Positional& positional = m_positionals.emplace_back(name);
  return positional;
}

Optional& ArgumentParser::AddOptional(
    std::initializer_list<std::string> flags) {
  Optional& optional = m_optionals.emplace_back(flags);

  for (const auto& flag : flags) {
    if (m_flags_map.contains(flag)) {
      throw std::runtime_error("Flag " + std::string{flag} + " redefined.");
    } else {
      m_flags_map.emplace(flag, optional);
    }
  }

  return optional;
}

Optional& ArgumentParser::AddOptional(const std::string& flag) {
  return AddOptional(std::initializer_list<std::string>{flag});
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
