#include <occa/io.hpp>
#include <occa/tools/string.hpp>
#include <occa/tools/lex.hpp>

#include <occa/lang/codes.hpp>
#include <occa/lang/file.hpp>
#include <occa/lang/tokenizer.hpp>

namespace occa {
  namespace lang {
    file_t::file_t(const std::string &filename_) :
      filename(filename_),
      expandedFilename(io::filename(filename_)),
      content(io::read(filename_)) {}

    file_t::file_t(const std::string &filename_,
                   const std::string &content_) :
      filename(filename_),
      expandedFilename(io::filename(filename_)),
      content(content_) {}

    file_t::file_t(const bool,
                   const std::string &name) :
      filename(name),
      expandedFilename(name),
      content("") {
      dontUseRefs();
    }

    namespace originSource {
      file_t builtin(true, "(builtin)");
      file_t string(true, "(source)" );
    }

    //---[ File Origin ]----------------
    filePosition::filePosition() :
      line(1),
      lineStart(NULL),
      start(NULL),
      end(NULL) {}

    filePosition::filePosition(const char *root) :
      line(1),
      lineStart(root),
      start(root),
      end(root) {}

    filePosition::filePosition(const int line_,
                               const char *lineStart_,
                               const char *start_,
                               const char *end_) :
      line(line_),
      lineStart(lineStart_),
      start(start_),
      end(end_) {}

    filePosition::filePosition(const filePosition &other) :
      line(other.line),
      lineStart(other.lineStart),
      start(other.start),
      end(other.end) {}

    size_t filePosition::size() const {
      return (end - start);
    }

    int filePosition::lineOffset() const {
      return (int) (start - lineStart);
    }

    std::string filePosition::str() const {
      if (!start) {
        return "";
      }
      return std::string(start, end - start);
    }
    //==================================

    //---[ File Origin ]----------------
    fileOrigin::fileOrigin() :
      fromInclude(true),
      file(&originSource::string),
      position(),
      up(NULL) {
      file->addRef();
    }

    fileOrigin::fileOrigin(file_t &file_) :
      fromInclude(true),
      file(&file_),
      position(file_.content.c_str()),
      up(NULL) {
      file->addRef();
    }

    fileOrigin::fileOrigin(const filePosition &position_) :
      fromInclude(true),
      file(&originSource::string),
      position(position_),
      up(NULL) {
      file->addRef();
    }

    fileOrigin::fileOrigin(file_t &file_,
                           const filePosition &position_) :
      fromInclude(true),
      file(&file_),
      position(position_),
      up(NULL) {
      file->addRef();
    }

    fileOrigin::fileOrigin(const fileOrigin &other) :
      fromInclude(other.fromInclude),
      file(other.file),
      position(other.position),
      up(other.up) {
      file->addRef();
      if (up) {
        up->addRef();
      }
    }

    fileOrigin& fileOrigin::operator = (const fileOrigin &other) {
      fromInclude = other.fromInclude;
      position    = other.position;

      setFile(*other.file);
      setUp(other.up);
      return *this;
    }

    fileOrigin::~fileOrigin() {
      clear();
    }

    void fileOrigin::clear() {
      if (file && !file->removeRef()) {
        delete file;
      }
      if (up && !up->removeRef()) {
        delete up;
      }
      file = NULL;
      up   = NULL;
    }

    bool fileOrigin::isValid() const {
      return file;
    }

    void fileOrigin::setFile(file_t &file_) {
      file_.addRef();
      if (file && !file->removeRef()) {
        delete file;
      }
      file = &file_;
    }

    void fileOrigin::setUp(fileOrigin *up_) {
      if (up_) {
        up_->addRef();
      }
      if (up && !up->removeRef()) {
        delete up;
      }
      up = up_;
    }

    void fileOrigin::push(const bool fromInclude_,
                          const fileOrigin &other) {
      push(fromInclude_,
           *other.file,
           other.position);
    }

    void fileOrigin::push(const bool fromInclude_,
                          file_t &file_,
                          const filePosition &position_) {

      setUp(new fileOrigin(*this));
      up->fromInclude = fromInclude_;
      position = position_;

      setFile(file_);
    }

    void fileOrigin::pop() {
      OCCA_ERROR("Unable to call fileOrigin::pop()",
                 up != NULL);

      fromInclude = up->fromInclude;
      position    = up->position;
      setFile(*(up->file));
      setUp(up->up);
    }

    fileOrigin fileOrigin::from(const bool fromInclude_,
                                const fileOrigin &other) const {
      fileOrigin fo = other;
      fo.push(fromInclude_, *this);
      return fo;
    }

    dim_t fileOrigin::distanceTo(const fileOrigin &other) const {
      if (file != other.file) {
        // TDOO: Return max dim_t as distance
        return -1;
      }
      return (other.position.start - position.end);
    }

    bool fileOrigin::onSameLine(const fileOrigin &other) const {
      if (file != other.file) {
        return false;
      }
      return (position.lineStart == other.position.lineStart);
    }

    void fileOrigin::printStack(io::output &out,
                                const bool root) const {
      if (up) {
        up->printStack(out, false);
      }

      if (!root) {
        // Print file location
        out << blue(file->filename) << ':';
        if (file != &originSource::builtin) {
          out << position.line;
        }

        if (fromInclude) {
          out << " Included file:\n";
        } else {
          out << " Expanded from macro '" << position.str() << "':\n";
        }
      }
    }

    void fileOrigin::printWarning(const std::string &message,
                                  const std::string &code) const {
      printWarning(io::stderr, message, code);
    }

    void fileOrigin::printWarning(io::output &out,
                                  const std::string &message,
                                  const std::string &code) const {
      warningCode(out, code)
          .withMessage(*this, message)
          .print();
    }

    void fileOrigin::printError(const std::string &message,
                                const std::string &code) const {
      printError(io::stderr, message, code);
    }

    void fileOrigin::printError(io::output &out,
                                const std::string &message,
                                const std::string &code) const {
      errorCode(out, code)
          .withMessage(*this, message)
          .print();
    }
    //==================================
  }
}
