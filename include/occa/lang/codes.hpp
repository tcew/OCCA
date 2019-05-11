#ifndef OCCA_LANG_CODES_HEADER
#define OCCA_LANG_CODES_HEADER

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include <occa/lang/file.hpp>

namespace occa {
  namespace lang {
    class codeSource_t;

    typedef std::set<codeSource_t>           codeSourceSet;
    typedef std::vector<codeSource_t>        codeSourceVector;
    typedef std::map<file_t*, codeSourceSet> fileCodeSourceMap;

    class codeSource_t {
    public:
      int index;
      fileOrigin origin;
      std::string message;

      codeSource_t(const fileOrigin &origin_,
                   const std::string &message_ = "");

      codeSource_t(const int index_,
                   const fileOrigin &origin_,
                   const std::string &message_);

      codeSource_t withIndex(const int index_) const;
    };

    bool operator < (const codeSource_t &a,
                     const codeSource_t &b);

    class codePrinter_t {
    private:
      io::output &out;

      const bool isError;
      std::string code;

      fileOrigin origin;
      std::string message;

      codeSourceSet originLineSources;
      fileCodeSourceMap sources;
      int sourceIndex;

      public:
      codePrinter_t(io::output &out_,
                    const bool isError_,
                    const std::string &code_);

      codePrinter_t& operator = (const codePrinter_t &other);

      codePrinter_t& withMessage(const fileOrigin &origin_,
                                const std::string &message_);

      codePrinter_t& withSource(const fileOrigin &origin_,
                               const std::string &message_ = "");

      codePrinter_t& withSource(const codeSource_t &source);

      int supressSources();

      void print();

      void printSources(std::stringstream &ss);

      void printOriginFileSources(std::stringstream &ss);

      void printFileSources(std::stringstream &ss,
                            file_t *file,
                            codeSourceSet &fileSources,
                            int sidebarWidth = -1);

      void printSourceLine(std::stringstream &ss,
                           file_t *file,
                           filePosition &linePos,
                           const int offset,
                           const int sidebarWidth);

      void printSupressedMessage(std::stringstream &ss,
                                 const int supressedSources);

      int getSidebarWidth(const int maxLine);

      void printOriginStack(std::stringstream &ss);

      void printFilename(std::stringstream &ss,
                         file_t *file);

      void printDivider(std::stringstream &ss,
                        const std::string &divider,
                        const int sidebarWidth);
    };

    codePrinter_t errorCode(io::output &out,
                            const std::string &code);

    inline codePrinter_t errorCode(const std::string &code) {
      return errorCode(io::stderr, code);
    }

    codePrinter_t warningCode(io::output &out,
                              const std::string &code);

    inline codePrinter_t warningCode(const std::string &code) {
      return warningCode(io::stderr, code);
    }
  }
}

#endif
