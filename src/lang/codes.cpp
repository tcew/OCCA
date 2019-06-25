#include <iterator>

#include <occa/lang/codes.hpp>
#include <occa/tools/lex.hpp>

namespace occa {
  namespace lang {
    const int DEFAULT_MAX_ERRORS_DISPLAYED = 5;

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
        isError = other.isError;
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
      int sourcesAvailable = DEFAULT_MAX_ERRORS_DISPLAYED;

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

      // Supress warnings/errors if there are too many
      const int supressedCount = supressSources();

      strVector sections;
      addSourceSections(sections);
      sections.push_back(
        getSupressedMessage(supressedCount)
      );

      if (isError) {
        occa::printError(out, message, code, sections);
      } else {
        occa::printWarning(out, message, code, sections);
      }
    }

    void codePrinter_t::addSourceSections(strVector &sections) {
      if (!sources.size()) {
        return;
      }
      // Print issues from the original file first
      addOriginFileSection(sections);

      fileCodeSourceMap::iterator it = sources.begin();
      while (it != sources.end()) {
        file_t *file = it->first;
        codeSourceSet &fileSources = it->second;

        if (file != origin.file) {
          addFileSection(sections, file, fileSources);
        }

        ++it;
      }
    }

    void codePrinter_t::addOriginFileSection(strVector &sections) {
      codeSourceSet &fileSources = sources[origin.file];

      bool originIsFirstSource = true;
      int maxLine = origin.position.line;

      // Get the maximum line number for padding reasons
      codeSourceSet::iterator it = fileSources.begin();
      while (it != fileSources.end()) {
        const codeSource_t &source = *(it++);

        if (maxLine < source.origin.position.line) {
          maxLine = source.origin.position.line;
        }
        if (!source.origin.onSameLine(origin) &&
            (origin.distanceTo(source.origin) > 0)) {
          originIsFirstSource = false;
        }
      }

      std::stringstream ss;
      if (origin.up) {
        io::output io_ss(ss);
        origin.up->printStack(io_ss, false);
      }

      const int sidebarWidth = getSidebarWidth(maxLine);
      const bool hasOriginSources = originLineSources.size();
      const bool hasFileSources = fileSources.size();

      // Add origin line sources first, followed by the rest of the origin file
      if (hasOriginSources) {
        ss << '\n';
        addFileSection(sections,
                       origin.file,
                       originLineSources,
                       sidebarWidth);
      }

      // If the origin message is the first line, no need to add a divider
      // splitting the origin message with other messages in the same file
      if (!originIsFirstSource
          && hasOriginSources
          && hasFileSources) {
        ss << '\n';
        printDivider(ss, "^^^", sidebarWidth);
      }

      if (hasFileSources) {
        ss << '\n';
        addFileSection(sections,
                       origin.file,
                       fileSources,
                       sidebarWidth);
      }

      // Push back stacktrace combined with file sections
      sections.push_back(ss.str());
    }

    void codePrinter_t::addFileSection(std::stringstream &ss,
                                       file_t *file,
                                       codeSourceSet &fileSources,
                                       int sidebarWidth) {
      if (!fileSources.size()) {
        return;
      }

      printFilenameLine(ss, file);

      if (sidebarWidth < 0) {
        // fileSources is an already sorted set, use the last line
        // as the max line number to determine sidebar width
        sidebarWidth = getSidebarWidth(
          (*fileSources.rbegin()).origin.position.line
        );
      }

      codeSourceSet::iterator it = fileSources.begin();
      while (it != fileSources.end()) {
        const codeSource_t &source = *(it++);
        const filePosition &position = source.origin.position;

        const char *lineEnd = position.lineStart;
        lex::skipTo(lineEnd, '\n');

        const std::string line(position.lineStart,
                               lineEnd - position.lineStart - 1);
        const std::string space(position.start - position.lineStart, ' ');

        ss << line << '\n'
           << space << green("^") << '\n';
      }
    }

    void codePrinter_t::addFileSection(strVector &sections,
                                       file_t *file,
                                       codeSourceSet &fileSources,
                                       int sidebarWidth) {
      std::stringstream ss;
      addFileSection(ss, file, fileSources, sidebarWidth);
      sections.push_back(ss.str());
    }

    std::string codePrinter_t::getSupressedMessage(const int supressedCount) {
      if (supressedCount <= 0) {
        return "";
      }

      std::stringstream ss;
      ss << "Supressed " << supressedCount << " additional ";
      if (supressedCount > 1) {
        ss << (isError ? "errors" : "warnings");
      } else {
        ss << (isError ? "error"  : "warning");
      }

      return yellow(ss.str());
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

    void codePrinter_t::addOriginStackSection(strVector &sections) {
      std::stringstream ss;
      io::output ioss(ss);

      if (origin.up) {
        origin.up->printStack(ioss, false);
      }
    }

    void codePrinter_t::printFilenameLine(std::stringstream &ss,
                                          file_t *file) {
      ss << blue(file->filename) << '\n';
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
