#include <occa/tools/testing.hpp>

#include <occa/lang/codes.hpp>
#include <occa/tools/env.hpp>

using namespace occa::lang;

const std::string fileSource = (
  "blah2();\n"
  "blah3();\n"
  "\n"
  "// test\n"
  "const int_ blah() {\n"
  "  return 1;\n"
  "}\n"
);
const char *c_fileSource = fileSource.c_str();

const char *lineStarts[8] = {
  c_fileSource + 0,
  c_fileSource + 0,
  c_fileSource + 8,
  c_fileSource + 17,
  c_fileSource + 18,
  c_fileSource + 26,
  c_fileSource + 46,
  c_fileSource + 58
};

#define MAKE_FILE_ORIGINS(name, line, start, length)        \
  MAKE_FILE_ORIGIN(1, builtin, name, line, start, length);  \
  MAKE_FILE_ORIGIN(2, string, name, line, start, length)

#define MAKE_FILE_ORIGIN(num, source, name, line, start, length)       \
  const fileOrigin name##Origin##num(                                  \
    occa::lang::originSource::source,                                  \
    filePosition(                                                      \
      line + 100 * (num - 1),                                          \
      lineStarts[line],                                                \
      lineStarts[line] + start,                                        \
      lineStarts[line] + start + length                                \
    ))

MAKE_FILE_ORIGINS(blah2, 1, 0 , 5);
MAKE_FILE_ORIGINS(blah3, 2, 0 , 5);
MAKE_FILE_ORIGINS(const, 5, 0 , 5);
MAKE_FILE_ORIGINS(int  , 5, 6 , 4);
MAKE_FILE_ORIGINS(blah , 5, 10, 4);
MAKE_FILE_ORIGINS(one  , 6, 8 , 1);
MAKE_FILE_ORIGINS(block, 5, 17, 15);

#undef MAKE_FILE_ORIGIN
#undef MAKE_FILE_ORIGINS

std::stringstream ss;
occa::io::output out(ss);

codePrinter_t getCodePrinter(const bool isError,
                             const std::string &code) {
  return (isError
          ? errorCode(code)
          : warningCode(code));
}

codePrinter_t makeErrorCode(const std::string &code) {
  ss.str("");
  return errorCode(out, code);
}

codePrinter_t makeWarningCode(const std::string &code) {
  ss.str("");
  return warningCode(out, code);
}

#define ASSERT_OUTPUT(out)                      \
  ASSERT_EQ(ss.str(), out)

int main(const int argc, const char **argv) {
  const std::string CODE = "E1234";
  const std::string MESSAGE = "Error message goes here";
  const std::string SOURCE1 = "a/b/source.cpp";
  const std::string SOURCE2 = "../source.hpp";
  const std::string SOURCE3 = "source.tpp";

  for (int isError = 0; isError < 2; ++isError) {
    // No sources
    std::cout << 1 << '\n';
    codePrinter_t ec1 = getCodePrinter(isError, CODE);
    ec1.withMessage(intOrigin1, MESSAGE)
        .print();

    // Source same as origin
    std::cout << 2 << '\n';
    codePrinter_t ec2 = getCodePrinter(isError, CODE);
    ec2.withMessage(intOrigin1, MESSAGE)
        .withSource(intOrigin1, SOURCE1)
        .print();

    // Multiple messages for the same source
    std::cout << 3 << '\n';
    codePrinter_t ec3 = getCodePrinter(isError, CODE);
    ec3.withMessage(intOrigin1, MESSAGE)
        .withSource(intOrigin1, SOURCE1)
        .withSource(intOrigin1, SOURCE2)
        .print();

    // Multiple sources in the same line
    std::cout << 4 << '\n';
    codePrinter_t ec4 = getCodePrinter(isError, CODE);
    ec4.withMessage(intOrigin1, MESSAGE)
        .withSource(intOrigin1, SOURCE1)
        .withSource(constOrigin1, SOURCE2)
        .withSource(blahOrigin1, SOURCE3)
        .print();

    // Source above origin
    std::cout << 5 << '\n';
    codePrinter_t ec5 = getCodePrinter(isError, CODE);
    ec5.withMessage(blahOrigin1, MESSAGE)
        .withSource(blahOrigin1, SOURCE1)
        .withSource(blah2Origin1, SOURCE2)
        .print();

    // Source way below origin
    std::cout << 6 << '\n';
    codePrinter_t ec6 = getCodePrinter(isError, CODE);
    ec6.withMessage(blah2Origin1, MESSAGE)
        .withMessage(blah2Origin1, SOURCE1)
        .withSource(intOrigin1, SOURCE2)
        .print();

    // Multiple files
    std::cout << 7 << '\n';
    codePrinter_t ec7 = getCodePrinter(isError, CODE);
    ec7.withMessage(blahOrigin1, MESSAGE)
        .withSource(blahOrigin1, SOURCE1)
        .withSource(blahOrigin2, SOURCE2)
        .print();

    // Supressed sources
    std::cout << 8 << '\n';
    codePrinter_t ec8 = getCodePrinter(isError, CODE);
    ec8.withMessage(intOrigin1, MESSAGE);
    for (int i = 0; i < 100; ++i) {
      ec8.withSource(intOrigin1, SOURCE1);
    }
    ec8.print();

    // Multi-line message
    std::cout << 9 << '\n';
    codePrinter_t ec9 = getCodePrinter(isError, CODE);
    ec9.withMessage(blockOrigin1, MESSAGE)
        .print();
  }

  return 0;
}
