#ifndef LORE_SOURCE_POS_H
#define LORE_SOURCE_POS_H

#include <string>
#include <sstream>
#include <stdexcept>

namespace lore {

struct SourcePos {
    std::string filename;
    int line = 1;
    int col = 1;

    std::string to_string() const {
        std::ostringstream ss;
        ss << filename << " line " << line << ", col " << col;
        return ss.str();
    }
};

class CompileError : public std::runtime_error {
public:
    SourcePos pos;
    std::string friendly_message;

    CompileError(const SourcePos& p, const std::string& msg)
        : std::runtime_error(format(p, msg)), pos(p), friendly_message(msg) {}

private:
    static std::string format(const SourcePos& p, const std::string& msg) {
        std::ostringstream ss;
        ss << p.to_string() << ": " << msg;
        return ss.str();
    }
};

} // namespace lore

#endif // LORE_SOURCE_POS_H
