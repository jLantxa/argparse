#pragma once

#include <list>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>

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

  std::pair<NArgs, std::size_t> GetNArgs() const;
};

struct Optional final {
  std::string name;
  std::string flag1;
  std::string flag2;
  bool required = false;
  NArgs nargs = NArgs::OPTIONAL;  // Numeric or special
  std::size_t num_args = 0;       // Number if NArgs is numeric
  std::string help;

  Optional(const std::string& name, const std::string& flag1,
           const std::string& flag2 = "");

  Optional& NumArgs(std::size_t num);
  Optional& NumArgs(NArgs num);
  Optional& Required(bool req);
  Optional& Help(const std::string& help);

  std::pair<NArgs, std::size_t> GetNArgs() const;
  bool HasFlag(const std::string& flag) const;
};

class ArgumentParser final {
 public:
  Positional& AddPositional(const std::string& name);
  Optional& AddOptional(const std::string& name);

 private:
  std::list<Positional> m_positionals;
  std::list<Optional> m_optionals;
};

}  // namespace argparse
