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

const char *lineStarts[9] = {
  c_fileSource + 0,
  c_fileSource + 0,
  c_fileSource + 9,
  c_fileSource + 18,
  c_fileSource + 19,
  c_fileSource + 27,
  c_fileSource + 47,
  c_fileSource + 59,
  c_fileSource + 61
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
MAKE_FILE_ORIGINS(blah , 5, 11, 4);
MAKE_FILE_ORIGINS(one  , 6, 8 , 1);
MAKE_FILE_ORIGINS(block, 5, 17, 15);

#undef MAKE_FILE_ORIGIN
#undef MAKE_FILE_ORIGINS

codePrinter_t getCodePrinter(const bool isError,
                             const std::string &code) {
  return (isError
          ? errorCode(code)
          : warningCode(code));
}

int main(const int argc, const char **argv) {
  const std::string CODE = "E1234";
  const std::string MESSAGE = "Error message goes here";
  const std::string SOURCE1 = "a/b/source.cpp";
  const std::string SOURCE2 = "../source.hpp";
  const std::string SOURCE3 = "source.tpp";

  codePrinter_t ec = getCodePrinter(true, "");
  for (int isError = 0; isError < 2; ++isError) {
    // No sources
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(intOrigin1, MESSAGE)
        .print();

    // Source same as origin
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(intOrigin1, MESSAGE)
        .withSource(intOrigin1, SOURCE1)
        .print();

    // Multiple messages for the same source
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(intOrigin1, MESSAGE)
        .withSource(intOrigin1, SOURCE1)
        .withSource(intOrigin1, SOURCE2)
        .print();

    // Multiple sources in the same line
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(intOrigin1, MESSAGE)
        .withSource(intOrigin1, SOURCE1)
        .withSource(constOrigin1, SOURCE2)
        .withSource(blahOrigin1, SOURCE3)
        .print();

    // Source above origin
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(blahOrigin1, MESSAGE)
        .withSource(blahOrigin1, SOURCE1)
        .withSource(blah2Origin1, SOURCE2)
        .print();

    // Source way below origin
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(blah2Origin1, MESSAGE)
        .withMessage(blah2Origin1, SOURCE1)
        .withSource(intOrigin1, SOURCE2)
        .print();

    // Multiple files
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(blahOrigin1, MESSAGE)
        .withSource(blahOrigin1, SOURCE1)
        .withSource(blahOrigin2, SOURCE2)
        .print();

    // Supressed sources (1)
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(intOrigin1, MESSAGE);
    for (int i = 0; i < (occa::lang::DEFAULT_MAX_ERRORS_DISPLAYED + 1); ++i) {
      fileOrigin origin = blah2Origin1;
      origin.position.line = i + 1;
      ec.withSource(origin, SOURCE1);
    }
    ec.print();

    // Supressed sources (1+)
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(intOrigin1, MESSAGE);
    for (int i = 0; i < (occa::lang::DEFAULT_MAX_ERRORS_DISPLAYED + 100); ++i) {
      fileOrigin origin = blah2Origin1;
      origin.position.line = i + 1;
      ec.withSource(origin, SOURCE1);
    }
    ec.print();

    // Multi-line message
    ec = getCodePrinter(isError, CODE);
    ec.withMessage(blockOrigin1, MESSAGE)
        .print();
  }

  return 0;
}
