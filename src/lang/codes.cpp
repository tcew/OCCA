#include <iterator>

#include <occa/lang/codes.hpp>

namespace occa {
  namespace lang {
    static const int MAX_SOURCES_PRINTED = 5;

    codeSource_t::codeSource_t(const fileOrigin &origin_,
                               const std::string &message_) :
        index(-1),
        origin(origin_),
        message(message_) {}

    codeSource_t::codeSource_t(const int index_,
                               const fileOrigin &origin_,
                               const std::string &message_) :
        index(index_),
        origin(origin_),
        message(message_) {}

    codeSource_t codeSource_t::withIndex(const int index_) const {
      return codeSource_t(index_, origin, message);
    }

    bool operator < (const codeSource_t &a,
                     const codeSource_t &b) {
      const fileOrigin &ao = a.origin;
      const fileOrigin &bo = b.origin;

      if (ao.file == bo.file) {
        if (ao.position.start < bo.position.start) {
          return -1;
        }
        if (ao.position.start > bo.position.start) {
          return 1;
        }
        if (a.index < b.index) {
          return -1;
        }
        if (a.index > b.index) {
          return 1;
        }
      } else {
        if (ao.file < bo.file) {
          return -1;
        }
        if (ao.file > bo.file) {
          return 1;
        }
      }
      return 0;
    }

    codePrinter_t::codePrinter_t(io::output &out_,
                                 const bool isError_,
                                 const std::string &code_) :
        out(out_),
        isError(isError_),
        code(code_),
        sourceIndex(0) {}

    codePrinter_t& codePrinter_t::operator = (const codePrinter_t &other) {
      if (this != &other) {
        code    = other.code;
        origin  = other.origin;
        message = other.message;

        originLineSources = other.originLineSources;
        sources     = other.sources;
        sourceIndex = other.sourceIndex;
      }
      return *this;
    }

    codePrinter_t& codePrinter_t::withMessage(const fileOrigin &origin_,
                                             const std::string &message_) {
      origin  = origin_;
      message = message_;
      return *this;
    }

    codePrinter_t& codePrinter_t::withSource(const fileOrigin &origin_,
                                            const std::string &message_) {
      return withSource(codeSource_t(origin_, message_));
    }

    codePrinter_t& codePrinter_t::withSource(const codeSource_t &source) {
      if ((source.origin.file == origin.file) &&
          source.origin.onSameLine(origin)) {
        // Sort by line offset
        originLineSources.insert(
          source.withIndex(source.origin.position.lineOffset())
        );
      } else {
        // Sort by insertion order
        sources[source.origin.file].insert(
          source.withIndex(sourceIndex++)
        );
      }
      return *this;
    }

    int codePrinter_t::supressSources() {
      int supressedCount = 0;
      int sourcesAvailable = MAX_SOURCES_PRINTED;

      fileCodeSourceMap::iterator it = sources.begin();
      while (it != sources.end()) {
        codeSourceSet &fileSources = it->second;
        ++it;

        const int fileSourceCount = (int) fileSources.size();

        if (sourcesAvailable >= fileSourceCount) {
          sourcesAvailable -= fileSourceCount;
        }
        else if (sourcesAvailable < fileSourceCount) {
          supressedCount = fileSourceCount - sourcesAvailable;

          // Clear extra sources
          codeSourceSet::iterator fileIt = fileSources.begin();
          std::advance(fileIt, sourcesAvailable);
          fileSources.erase(fileIt, fileSources.end());

          // Erase the rest of the sources
          sources.erase(it, sources.end());
        }
        else {
          supressedCount += fileSourceCount;
        }
      }
      return supressedCount;
    }

    void codePrinter_t::print() {
      OCCA_ERROR((isError ? "Error" : "Warning") << " code is missing its origin",
                 origin.isValid());

      if (isError) {
        occa::printError(out, message, code);
      } else {
        occa::printWarning(out, message, code);
      }

      const int supressedCount = supressSources();

      std::stringstream ss;
      printSources(ss);

      if (supressedCount) {
        ss << '\n';
        printSupressedMessage(ss, supressedCount);
        ss << '\n';
      }

      out << '\n'
          << pad(ss.str(), 4)
          << '\n';
    }

    void codePrinter_t::printSources(std::stringstream &ss) {
      printOriginFileSources(ss);

      fileCodeSourceMap::iterator it = sources.begin();
      while (it != sources.end()) {
        file_t *file = it->first;
        codeSourceSet &fileSources = it->second;

        if (file != origin.file) {
          printFileSources(ss, file, fileSources);
        }

        ++it;
      }
    }

    void codePrinter_t::printOriginFileSources(std::stringstream &ss) {
      codeSourceSet &fileSources = sources[origin.file];

      bool originIsFirstSource = true;
      int maxLine = origin.position.line;

      codeSourceSet::iterator it = fileSources.begin();
      while (it != fileSources.end()) {
        const codeSource_t &source = *it;

        if (maxLine < source.origin.position.line) {
          maxLine = source.origin.position.line;
        }
        if (!source.origin.onSameLine(origin) &&
            (origin.distanceTo(source.origin) > 0)) {
          originIsFirstSource = false;
        }

        ++it;
      }

      if (originIsFirstSource) {
        printFileSources(ss, origin.file, fileSources);
        return;
      }

      printOriginStack(ss);

      const int sidebarWidth = getSidebarWidth(maxLine);
      printFileSources(ss,
                       origin.file,
                       originLineSources,
                       sidebarWidth);

      printDivider(ss, "^^^", sidebarWidth);
      ss << '\n';

      printFileSources(ss,
                       origin.file,
                       fileSources,
                       sidebarWidth);
    }

    void codePrinter_t::printFileSources(std::stringstream &ss,
                                         file_t *file,
                                         codeSourceSet &fileSources,
                                         int sidebarWidth) {
      const int fileSourceCount = (int) fileSources.size();
      if (!fileSourceCount) {
        return;
      }

      // fileSources is already sorted, don't sort in getSourceLines
      printFilename(ss, file);

      if (sidebarWidth < 0) {
        // Use the last line as the max line number to determine sidebar width
        sidebarWidth = getSidebarWidth(
          (*fileSources.rbegin()).origin.position.line
        );
      }

#if 0
      const char *lineEnd = position.lineStart;
      lex::skipTo(lineEnd, '\n');

      const std::string line(position.lineStart,
                             lineEnd - position.lineStart);
      const std::string space(position.start - position.lineStart, ' ');

      ss << line << '\n'
          << space << green("^") << '\n';
#endif
    }

    void codePrinter_t::printSupressedMessage(std::stringstream &ss,
                                              const int supressedCount) {
      if (supressedCount <= 0) {
        return;
      }

      std::stringstream sOut;
      sOut << "Supressed " << supressedCount << " additional ";
      if (supressedCount > 1) {
        sOut << (isError ? "errors" : "warnings");
      } else {
        sOut << (isError ? "error"  : "warning");
      }

      ss << yellow(sOut.str());
    }

    int codePrinter_t::getSidebarWidth(const int maxLine) {
      int width = 3; // left-padding: 1, number: 1, right-padding: 1
      int line = maxLine / 10;
      while (line) {
        ++width;
        line /= 10;
      }
      return width;
    }

    void codePrinter_t::printOriginStack(std::stringstream &ss) {
      io::output ioss(ss);
      if (origin.up) {
        origin.up->printStack(ioss, false);
      }

      printFilename(ss, origin.file);

      if (isError) {
        occa::printError(ioss, message, code);
      } else {
        occa::printWarning(ioss, message, code);
      }
    }

    void codePrinter_t::printFilename(std::stringstream &ss,
                                      file_t *file) {
      ss << blue(file->filename);
    }

    void codePrinter_t::printDivider(std::stringstream &ss,
                                     const std::string &divider,
                                     const int sidebarWidth) {
      // Center divider along the sidebar border
      int padding = sidebarWidth - (int) (divider.size() / 2);
      ss << std::string(padding, ' ') << divider;
    }

    codePrinter_t errorCode(io::output &out,
                            const std::string &code) {
      return codePrinter_t(out, true, code);
    }

    codePrinter_t warningCode(io::output &out,
                              const std::string &code) {
      return codePrinter_t(out, false, code);
    }
  }
}
