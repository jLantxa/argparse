#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <span>

#include "argparse.hpp"

TEST(env, GetArgs) {
  int argc = 5;
  const char* argv[5]{
      "arg0", "1", "two", "tres", "end",
  };

  const std::span<const char*> args = argparse::env::GetArgs(argc, argv);

  ASSERT_EQ(args.size(), 5);
  ASSERT_THAT(args,
              ::testing::ElementsAreArray({"arg0", "1", "two", "tres", "end"}));
}

TEST(PositionalArgument, valid_names) {
  EXPECT_NO_THROW(argparse::Positional pos("pos"));

  EXPECT_THROW(argparse::Positional pos("-pos"), std::runtime_error);
  EXPECT_THROW(argparse::Positional pos(""), std::runtime_error);
}

TEST(PositionalArgument, builder) {
  argparse::Positional pos0("pos0");
  argparse::Positional& pos0_ref = pos0;
  pos0_ref.Help("Positional 0");
  EXPECT_EQ(pos0.name, "pos0");
  EXPECT_EQ(pos0.nargs, argparse::NArgs::NUMERIC);
  EXPECT_EQ(pos0.num_args, 1);
  EXPECT_EQ(pos0.help, "Positional 0");

  argparse::Positional pos1("pos1");
  argparse::Positional& pos1_ref = pos1;
  pos1_ref.NumArgs(argparse::NArgs::OPTIONAL).Help("Positional 1");
  EXPECT_EQ(pos1.name, "pos1");
  EXPECT_EQ(pos1.nargs, argparse::NArgs::OPTIONAL);
  EXPECT_EQ(pos1.help, "Positional 1");

  argparse::Positional pos2("pos2");
  argparse::Positional& pos2_ref = pos2;
  pos2_ref.NumArgs(3)
      .Help("Positional 2")
      .NumArgs(argparse::NArgs::ONE_OR_MORE);
  EXPECT_EQ(pos2.name, "pos2");
  EXPECT_EQ(pos2.nargs, argparse::NArgs::ONE_OR_MORE);
  EXPECT_EQ(pos2.help, "Positional 2");

  argparse::Positional pos3("pos3");
  argparse::Positional& pos3_ref = pos3;
  pos3_ref.NumArgs(3).Help("Positional 3");
  EXPECT_EQ(pos3.name, "pos3");
  EXPECT_EQ(pos3.nargs, argparse::NArgs::NUMERIC);
  EXPECT_EQ(pos3.num_args, 3);
  EXPECT_EQ(pos3.help, "Positional 3");
}

TEST(PositionalArgument, builder_exception) {
  argparse::Positional pos0("pos0");
  argparse::Positional& pos0_ref = pos0;

  // NumArgs cannot be 0 for Positional arguments
  EXPECT_THROW(pos0_ref.NumArgs(0), std::runtime_error);
}

TEST(OptionalArgument, valid_names_and_flags) {
  EXPECT_NO_THROW(argparse::Optional opt({"-o", "--opt"}));
  EXPECT_NO_THROW(argparse::Optional opt({"-o"}));
  EXPECT_NO_THROW(argparse::Optional opt("--opt"));
  EXPECT_NO_THROW(argparse::Optional opt({"-o", "-q", "--flag"}));

  EXPECT_THROW(argparse::Optional opt({"-o", ""}), std::runtime_error);
  EXPECT_THROW(argparse::Optional opt({"", "--opt"}), std::runtime_error);
  EXPECT_THROW(argparse::Optional opt({"o", "opt"}), std::runtime_error);
  EXPECT_THROW(argparse::Optional opt({"-o", "opt"}), std::runtime_error);
  EXPECT_THROW(argparse::Optional opt({"o", "--opt"}), std::runtime_error);
}

TEST(OptionalArgument, builder) {
  argparse::Optional opt0({"-f", "--flag1", "-q"});
  argparse::Optional& opt0_ref = opt0;
  opt0_ref.Help("Optional 0");
  EXPECT_EQ(opt0.nargs, argparse::NArgs::OPTIONAL);
  EXPECT_EQ(opt0.num_args, 0);
  EXPECT_EQ(opt0.help, "Optional 0");
  EXPECT_EQ(opt0.required, false);
  EXPECT_TRUE(opt0.HasFlag("-f"));
  EXPECT_TRUE(opt0.HasFlag("--flag1"));
  EXPECT_TRUE(opt0.HasFlag("-q"));
  EXPECT_FALSE(opt0.HasFlag("--no-flag"));

  argparse::Optional opt1({"-f"});
  argparse::Optional& opt1_ref = opt1;
  opt1_ref.Required(false).Help("Optional 1");
  EXPECT_EQ(opt1.nargs, argparse::NArgs::OPTIONAL);
  EXPECT_EQ(opt1.num_args, 0);
  EXPECT_EQ(opt1.required, false);
  EXPECT_EQ(opt1.help, "Optional 1");
  EXPECT_TRUE(opt1.HasFlag("-f"));
  EXPECT_FALSE(opt1.HasFlag("--flag1"));

  argparse::Optional opt2({"--long_flag"});
  argparse::Optional& opt2_ref = opt2;
  opt2_ref.Required(true).NumArgs(1).Help("Optional 2");
  EXPECT_EQ(opt2.nargs, argparse::NArgs::NUMERIC);
  EXPECT_EQ(opt2.num_args, 1);
  EXPECT_EQ(opt2.required, true);
  EXPECT_EQ(opt2.help, "Optional 2");
  EXPECT_TRUE(opt2.HasFlag("--long_flag"));
  EXPECT_FALSE(opt2.HasFlag("--something"));
}

TEST(Argument, AsString) {
  const char* args[] = {"first", "second"};
  argparse::Argument arg(args);
  EXPECT_EQ(arg.Size(), 2);
  EXPECT_EQ(arg.As<std::string>(), "first");
  EXPECT_EQ(arg.As<std::string>(1), "second");
  EXPECT_THAT(arg.AsVector<std::string>(),
              ::testing::ElementsAreArray({"first", "second"}));
}

TEST(Argument, AsInt) {
  const char* args[] = {"-1", "13"};
  argparse::Argument arg(args);
  EXPECT_EQ(arg.Size(), 2);
  EXPECT_EQ(arg.As<int>(), -1);
  EXPECT_EQ(arg.As<int>(1), 13);
  EXPECT_THAT(arg.AsVector<int>(), ::testing::ElementsAreArray({-1, 13}));
}

TEST(Argument, AsLong) {
  const char* args[] = {"-10", "130"};
  argparse::Argument arg(args);
  EXPECT_EQ(arg.Size(), 2);
  EXPECT_EQ(arg.As<int>(), -10);
  EXPECT_EQ(arg.As<int>(1), 130);
  EXPECT_THAT(arg.AsVector<int>(), ::testing::ElementsAreArray({-10, 130}));
}

TEST(Argument, AsFloat) {
  const std::string args[] = {std::to_string(3.14f), std::to_string(-0.5f)};
  argparse::Argument arg(args);
  EXPECT_EQ(arg.Size(), 2);
  EXPECT_EQ(arg.As<float>(), 3.14f);
  EXPECT_EQ(arg.As<float>(1), -0.5f);
  EXPECT_THAT(arg.AsVector<float>(),
              ::testing::ElementsAreArray({3.14f, -0.5f}));
}

TEST(Argument, AsDouble) {
  const std::string args[] = {std::to_string(3.14), std::to_string(-0.5)};
  argparse::Argument arg(args);
  EXPECT_EQ(arg.Size(), 2);
  EXPECT_EQ(arg.As<double>(), 3.14);
  EXPECT_EQ(arg.As<double>(1), -0.5);
  EXPECT_THAT(arg.AsVector<double>(),
              ::testing::ElementsAreArray({3.14, -0.5}));
}

TEST(ArgumentParser, create_parser_with_arguments) {
  argparse::ArgumentParser parser;
  EXPECT_NO_THROW(
      parser.AddPositional("positional").Help("Positional argument");
      parser.AddOptional({"-o"}).Help("Optional argument"););
}

TEST(ArgumentParser, redefined_names_throw) {
  argparse::ArgumentParser parser;
  parser.AddPositional("positional").Help("Positional argument");
  parser.AddOptional({"-o"}).Help("Positional argument");

  EXPECT_THROW(parser.AddPositional("positional").Help("Redefined argument"),
               std::runtime_error);
  EXPECT_THROW(parser.AddOptional({"-o"}).Help("Redefined argument"),
               std::runtime_error);
}

TEST(ArgumentParser, redefined_flags_throw) {
  argparse::ArgumentParser parser;
  parser.AddOptional({"-o"}).Help("Positional argument");

  EXPECT_THROW(parser.AddOptional({"-o"}).Help(
                   "Same flag with under different argument name"),
               std::runtime_error);
}
