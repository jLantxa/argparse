#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <span>

#include "argparse.hpp"

TEST(argparse, GetArgs) {
  int argc = 5;
  const char *argv[5]{
      "arg0", "1", "two", "tres", "end",
  };

  const std::span<const char *> args = argparse::env::GetArgs(argc, argv);

  ASSERT_EQ(args.size(), 5);
  ASSERT_THAT(args,
              ::testing::ElementsAreArray({"arg0", "1", "two", "tres", "end"}));
}
