#include "argparse.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace argparse {

namespace env {

[[nodiscard]] const std::span<const char*> GetArgs(int argc,
                                                   const char* argv[]) {
  return {argv, static_cast<std::size_t>(argc)};
}

}  // namespace env

static bool IsValidFlagName(const std::string& flag) {
  // TODO: Check that flag contains no whitespace
  return (!flag.empty() && flag.starts_with("-"));
}

static bool IsNumber(const std::string& str) {
  char* pEnd;
  std::strtod(str.c_str(), &pEnd);

  return (pEnd == nullptr);
}

static bool IsOption(const std::string& str) {
  return (str.starts_with("-") && !IsNumber(str));
}

static NArgs GetNArgsFromString(const std::string& str) {
  if (str == "?") {
    return NArgs::OPTIONAL;
  } else if (str == "*") {
    return NArgs::ZERO_OR_MORE;
  } else if (str == "+") {
    return NArgs::ONE_OR_MORE;
  }

  throw std::runtime_error(str + " is not a valid number of arguments");
}

Positional::Positional(const std::string& _name) : name(_name) {
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

Positional& Positional::NumArgs(const std::string& num) {
  const NArgs nargs = GetNArgsFromString(num);
  return NumArgs(nargs);
}

Positional& Positional::Help(const std::string& help_str) {
  help = help_str;
  return *this;
}

std::pair<NArgs, std::size_t> Positional::GetNArgs() const {
  return {nargs, num_args};
}

Optional::Optional(std::initializer_list<std::string> flag_list) {
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
  if (required &&
      ((nargs == NArgs::OPTIONAL) || (nargs == NArgs::ZERO_OR_MORE))) {
    throw std::runtime_error("An required optional cannot be made required.");
  }

  nargs = num;
  return *this;
}

Optional& Optional::NumArgs(const std::string& num) {
  const NArgs nargs = GetNArgsFromString(num);
  return NumArgs(nargs);
}

Optional& Optional::Required(bool req) {
  if ((req == true) &&
      ((nargs == NArgs::OPTIONAL) || nargs == NArgs::ZERO_OR_MORE)) {
    throw std::runtime_error("An optional argument cannot be made required.");
  }

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
  m_map.insert_or_assign(name, arg);
}

bool ArgumentMap::Contains(const std::string& name) const {
  return m_map.contains(name);
}

const Argument& ArgumentMap::operator[](const std::string& name) const {
  const auto it = m_map.find(name);
  if (it == m_map.end()) {
    throw std::runtime_error("Undefined argument " + std::string{name} + ".");
  }

  return it->second;
}

ArgumentParser::ArgumentParser(const std::string& description)
    : m_program_description(description) {}

void ArgumentParser::IgnoreFirstArgument(bool ignore) {
  m_ignore_first_argument = ignore;
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

const ArgumentMap ArgumentParser::Parse(std::span<const std::string> in_args) {
  const std::size_t first_argument = m_ignore_first_argument ? 1 : 0;
  const auto args = in_args.subspan(first_argument);

  const std::size_t args_size = args.size();
  std::size_t num_positionals = 0;
  for (std::size_t i = 0; i < args_size; ++i) {
    if (IsOption(args[i])) {
      break;
    }
    ++num_positionals;
  }

  const std::span<const std::string> positionals =
      args.subspan(0, num_positionals);
  const std::span<const std::string> optionals = args.subspan(num_positionals);

  ValidateRequiredOptionals(optionals);

  ArgumentMap map;
  ParsePositionals(positionals, map);
  ParseOptionals(optionals, map);

  return map;
}

void ArgumentParser::ValidateRequiredOptionals(
    std::span<const std::string> args) const {
  for (const auto& optional : m_optionals) {
    if (optional.required == false) {
      continue;
    }

    bool found_required = false;
    for (const auto& flag : optional.flags) {
      const auto it = std::find(args.begin(), args.end(), flag);
      if (it != args.end()) {
        found_required = true;
        break;
      }
    }

    if (found_required) {
      continue;
    }

    std::stringstream ss;
    ss << "Option ";
    if (optional.flags.size() == 1) {
      ss << optional.flags[0];
    } else {
      ss << "{";
      const std::size_t num_flags = optional.flags.size();
      for (std::size_t i = 0; i < (num_flags - 1); ++i) {
        ss << optional.flags[i] << ", ";
      }
      ss << optional.flags.back() << "}";
    }
    ss << " is required.";

    throw std::runtime_error(ss.str());
  }
}

std::size_t ArgumentParser::GetMinNumberOfArguments(
    std::list<Positional>::const_iterator begin,
    std::list<Positional>::const_iterator end) const {
  std::size_t count = 0;
  for (auto it = begin; it != end; ++it) {
    const auto [nargs, num_args] = it->GetNArgs();
    switch (nargs) {
      case NArgs::NUMERIC:
        count += num_args;
        break;

      case NArgs::OPTIONAL:
      case NArgs::ZERO_OR_MORE:
        break;

      case NArgs::ONE_OR_MORE:
        ++count;
        break;
    }
  }

  return count;
}

void ArgumentParser::ParsePositionals(std::span<const std::string> args,
                                      ArgumentMap& map) const {
  const std::size_t num_args = args.size();
  std::size_t current_arg_index = 0;

  for (auto pos_it = m_positionals.begin(); pos_it != m_positionals.end();
       ++pos_it) {
    const auto [pos_nargs, pos_num_args] = pos_it->GetNArgs();
    const std::size_t min_num_other_positionals =
        GetMinNumberOfArguments(std::next(pos_it), m_positionals.end());
    const std::size_t num_remaining_args =
        num_args - current_arg_index - min_num_other_positionals;
    const auto& name = pos_it->name;

    std::size_t num_matched_args;
    switch (pos_nargs) {
      case NArgs::NUMERIC: {
        if (num_remaining_args < pos_num_args) {
          throw std::runtime_error("Positional argument " + name +
                                   " requires " + std::to_string(pos_num_args) +
                                   " values but found " +
                                   std::to_string(num_remaining_args) + ".");
        }
        num_matched_args = pos_num_args;
        break;
      }

      case NArgs::ONE_OR_MORE: {
        if (num_remaining_args < 1) {
          throw std::runtime_error(
              "Positional argument " + name +
              " requires one or more values but found none.");
        }
        num_matched_args = num_remaining_args;
        break;
      }

      case NArgs::ZERO_OR_MORE: {
        num_matched_args = num_remaining_args;
        break;
      }

      case NArgs::OPTIONAL: {
        num_matched_args = (num_remaining_args > 0) ? 1 : 0;
        break;
      }
    }
    const auto subspan = args.subspan(current_arg_index, num_matched_args);
    current_arg_index += num_matched_args;
    map.Add(name, subspan);
  }

  if (current_arg_index < num_args) {
    throw std::runtime_error("Unmatched positional arguments.");
  }
}

void ArgumentParser::ParseOptionals(std::span<const std::string> args,
                                    ArgumentMap& map) const {
  std::size_t current_index = 0;
  const std::size_t args_size = args.size();
  while (current_index < args_size) {
    const auto subspan = args.subspan(current_index);
    current_index += TryMatchOptional(subspan, map);
  }
}

std::size_t ArgumentParser::TryMatchOptional(std::span<const std::string> args,
                                             ArgumentMap& map) const {
  const std::string& token = args[0];

  if (!IsOption(token)) {
    return 1;
  }

  const auto optional_it = m_flags_map.find(token);
  const bool token_not_found = (optional_it == m_flags_map.end());
  if (token_not_found) {
    throw std::runtime_error("Undefined option " + std::string{token} + ".");
  }

  // Find index of next option
  const std::size_t args_size = args.size();
  std::size_t num_option_values = 0;
  for (std::size_t i = 1; i < args_size; ++i) {
    if (IsOption(args[i])) {
      break;
    }
    ++num_option_values;
  }

  const Optional& optional = optional_it->second;
  const std::span<const std::string> option_values =
      args.subspan(1, num_option_values);
  switch (optional.nargs) {
    // N
    case NArgs::NUMERIC: {
      if (num_option_values != optional.num_args) {
        throw std::runtime_error("Option " + std::string{token} + " expected " +
                                 std::to_string(optional.num_args) +
                                 " arguments but found " +
                                 std::to_string(num_option_values) + ".");
      }
      break;
    }

    // ?
    case NArgs::OPTIONAL: {
      if (num_option_values > 1) {
        throw std::runtime_error("Option " + std::string{token} +
                                 " expected zero or one arguments but found " +
                                 std::to_string(num_option_values) + ".");
      }
      break;
    }

    // *
    case NArgs::ZERO_OR_MORE: {
      // Everything
      break;
    }

    // +
    case NArgs::ONE_OR_MORE: {
      if (num_option_values < 1) {
        throw std::runtime_error(
            "Option " + std::string{token} +
            " expected one or more arguments but found 0.");
      }
      break;
    }

    default:
      throw std::runtime_error(
          "Unknown number of required optional arguments for " +
          std::string{token} + ".");
  }

  /* TODO: Implement a second map for optional flags to avoid duplicating
   * arguments in the map.
   */
  for (const auto& flag : optional.flags) {
    map.Add(flag, option_values);
  }

  return (num_option_values + 1);
}

static std::string PrettyNArgs(std::pair<NArgs, std::size_t> nargs) {
  const auto [nargs_flag, num_args] = nargs;

  switch (nargs_flag) {
    case NArgs::NUMERIC: {
      if (num_args > 1) {
        return "[" + std::to_string(num_args) + "]";
      } else {
        return "";
      }
    }

    case NArgs::OPTIONAL:
      return "[?]";

    case NArgs::ZERO_OR_MORE:
      return "[*]";

    case NArgs::ONE_OR_MORE:
      return "[+]";
  }
}

void ArgumentParser::PrintHelp() const {
  if (!m_program_description.empty()) {
    std::cout << m_program_description << "\n\n";
  }

  std::cout << "positional arguments:\n";
  for (const auto& positional : m_positionals) {
    std::cout << positional.name << " " << PrettyNArgs(positional.GetNArgs())
              << "\t" << positional.help << "\n";
  }

  std::cout << "\n";

  // TODO: Print NArgs
  std::cout << "optional arguments:\n";
  for (const auto& optional : m_optionals) {
    std::cout << " ";
    std::stringstream ss;

    const std::size_t num_flags = optional.flags.size();
    for (std::size_t i = 0; i < (num_flags - 1); ++i) {
      ss << optional.flags[i] << ", ";
    }
    if (num_flags > 0) {
      ss << optional.flags.back();
    }

    std::cout << ss.str() << " ";
    std::cout << PrettyNArgs(optional.GetNArgs());
    std::cout << " \t" << optional.help << "\n";
  }
}

}  // namespace argparse
