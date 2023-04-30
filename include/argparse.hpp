#pragma once

#include <initializer_list>
#include <list>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace argparse {

namespace env {

[[nodiscard]] const std::span<const char*> GetArgs(int argc,
                                                   const char* argv[]);

}  // namespace env

enum class NArgs {
  NUMERIC,
  OPTIONAL,
  ZERO_OR_MORE,
  ONE_OR_MORE,
};

struct Positional {
  std::string name;
  NArgs nargs = NArgs::NUMERIC;  // Numeric or special
  std::size_t num_args = 1;      // Number if NArgs is numeric
  std::string help;

  Positional(const std::string& name);

  Positional& NumArgs(std::size_t num);
  Positional& NumArgs(NArgs num);
  Positional& Help(const std::string& help);

  [[nodiscard]] std::pair<NArgs, std::size_t> GetNArgs() const;
};

struct Optional final {
  std::vector<std::string> flags;
  bool required = false;
  NArgs nargs = NArgs::OPTIONAL;  // Numeric or special
  std::size_t num_args = 0;       // Number if NArgs is numeric
  std::string help;

  Optional(std::initializer_list<std::string> flags);
  Optional(const std::string& flag);

  Optional& NumArgs(std::size_t num);
  Optional& NumArgs(NArgs num);
  Optional& Required(bool req);
  Optional& Help(const std::string& help);

  [[nodiscard]] std::pair<NArgs, std::size_t> GetNArgs() const;
  [[nodiscard]] bool HasFlag(const std::string& flag) const;
};

class Argument final {
 public:
  Argument(std::span<const char*> values);
  Argument(std::span<const std::string> values);

  [[nodiscard]] std::size_t Size() const;

  template <typename T>
  T As(std::size_t index) const;

  template <typename T>
  T As() const {
    return As<T>(0);
  }

  template <typename T>
  std::vector<T> AsVector() const;

  operator std::vector<std::string>() const;
  std::vector<std::string> operator*() const;

 private:
  std::vector<std::string> m_values;
};

class ArgumentMap final {
 public:
  void Add(const std::string& name, const Argument& arg);

  [[nodiscard]] bool Has(const std::string& name) const;

 private:
  std::unordered_map<std::string, Argument> m_map;
};

class ArgumentParser final {
 public:
  ArgumentParser() = default;
  ArgumentParser(const std::string& program_name,
                 const std::string& description = "");

  void GenerateHelp(std::initializer_list<std::string> flags);

  Positional& AddPositional(const std::string& name);
  Optional& AddOptional(std::initializer_list<std::string> flags);
  Optional& AddOptional(const std::string& flag);

  const ArgumentMap Parse(int argc, const char* argv[]);
  const ArgumentMap Parse(std::span<const char*> args);
  const ArgumentMap Parse(std::span<const std::string> args);

 private:
  std::string m_program_name;
  std::string m_program_description;
  std::unordered_set<std::string> m_help_flags;

  std::list<Positional> m_positionals;
  std::list<Optional> m_optionals;

  std::unordered_set<std::string> m_positional_names;
  std::unordered_map<std::string, Optional&> m_flags_map;
};

}  // namespace argparse
