#include "argparse.hpp"

#include <cstdlib>

namespace argparse {

namespace env {

[[nodiscard]] const std::span<const char *> GetArgs(int argc,
                                                    const char *argv[]) {
  return {argv, static_cast<std::size_t>(argc)};
}

}  // namespace env

}  // namespace argparse
