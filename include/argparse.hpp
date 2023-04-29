#pragma once

#include <span>

namespace argparse {

namespace env {

[[nodiscard]] const std::span<const char *> GetArgs(int argc,
                                                    const char *argv[]);

}  // namespace env

}  // namespace argparse
