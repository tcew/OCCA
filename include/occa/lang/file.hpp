#ifndef OCCA_LANG_FILE_HEADER
#define OCCA_LANG_FILE_HEADER

#include <iostream>

#include <occa/io/output.hpp>
#include <occa/tools/gc.hpp>
#include <occa/lang/operator.hpp>

namespace occa {
  namespace lang {
    class file_t : public gc::withRefs {
    public:
      std::string filename;
      std::string expandedFilename;
      std::string content;

      file_t(const std::string &filename_);

      file_t(const std::string &filename_,
             const std::string &content_);

      // Used for originSource
      file_t(const bool,
             const std::string &name);
    };

    namespace originSource {
      extern file_t builtin;
      extern file_t string;
    }

    class filePosition {
    public:
      int line;
      const char *lineStart;
      const char *start, *end;

      filePosition();

      filePosition(const char *root);

      filePosition(const int line_,
                   const char *lineStart_,
                   const char *start_,
                   const char *end_);

      filePosition(const filePosition &other);

      size_t size() const;

      int lineOffset() const;

      std::string str() const;
    };

    class fileOrigin : public gc::withRefs {
    public:
      bool fromInclude;
      file_t *file;
      filePosition position;
      fileOrigin *up;

      fileOrigin();

      fileOrigin(file_t &file_);

      fileOrigin(const filePosition &position_);

      fileOrigin(file_t &file_,
                 const filePosition &position_);

      fileOrigin(const fileOrigin &other);

      fileOrigin& operator = (const fileOrigin &other);

      virtual ~fileOrigin();

      void clear();

      bool isValid() const;

      void setFile(file_t &file_);
      void setUp(fileOrigin *up_);

      void push(const bool fromInclude_,
                const fileOrigin &other);

      void push(const bool fromInclude_,
                file_t &file_,
                const filePosition &position_);

      void pop();

      fileOrigin from(const bool fromInclude_,
                      const fileOrigin &other) const;

      dim_t distanceTo(const fileOrigin &other) const;

      bool onSameLine(const fileOrigin &other) const;

      void printStack(io::output &out,
                      const bool root = true) const;

      void printWarning(const std::string &message,
                        const std::string &code = "") const;

      void printWarning(io::output &out,
                        const std::string &message,
                        const std::string &code = "") const;

      void printError(const std::string &message,
                      const std::string &code = "") const;

      void printError(io::output &out,
                      const std::string &message,
                      const std::string &code = "") const;
    };
  }
}

#endif
