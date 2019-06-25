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

    extern const int DEFAULT_MAX_ERRORS_DISPLAYED;

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

      bool isError;
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

      void addSourceSections(strVector &sections);

      void addOriginFileSection(strVector &sections);

      void addFileSection(std::stringstream &ss,
                          file_t *file,
                          codeSourceSet &fileSources,
                          int sidebarWidth = -1);

      void addFileSection(strVector &sections,
                          file_t *file,
                          codeSourceSet &fileSources,
                          int sidebarWidth = -1);

      void printSourceLine(strVector &sections,
                           file_t *file,
                           filePosition &linePos,
                           const int offset,
                           const int sidebarWidth);

      std::string getSupressedMessage(const int supressedSources);

      int getSidebarWidth(const int maxLine);

      void addOriginStackSection(strVector &sections);

      void printFilenameLine(std::stringstream &ss,
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
