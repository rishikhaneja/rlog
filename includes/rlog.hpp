/**
 * @file rlog.hpp
 * @description logger for c++
 *              this file should be treated as read only
 *              configurables lie in a separate file rlog_config.hpp
 * @author Rishi Khaneja
 */

// -----------------------------------------------------------

#ifndef __R_LOG_HPP__
#define __R_LOG_HPP__

// -----------------------------------------------------------
/// own headers

#include "rlog_config.hpp"

// -----------------------------------------------------------
/// external headers

#include <time.h>
#include <cassert>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

// -----------------------------------------------------------
/// private macros
/// all macros starting with _ are for internal use only

#define _R_DISALLOW_COPY_ASSIGN(_name) \
    _name(const _name&) = delete;      \
    _name& operator=(const _name&) = delete;

#define _R_LOG(_level, _tag)                                              \
    if (R_MIN_LEVEL > R::Level::_level) {                                 \
    } else if (R::Level::_level < R::internal::Store::instance().level) { \
    } else                                                                \
        R::internal::Log(R::Level::_level, __FILE__, __LINE__, _tag).stream()

// -----------------------------------------------------------
/// public macros

/**
 * @brief Makes a log with level Info
 * @param tag: const std::string&
 * @usage R_INFO("foo") << "bar";
 */
#define R_INFO(_tag) _R_LOG(Info, _tag)

/**
 * @brief Makes a log with level Warning
 * @param tag: const std::string&
 * @usage R_WARNING("foo") << "bar";
 */
#define R_WARNING(_tag) _R_LOG(Warning, _tag)

/**
 * @brief Makes a log with level Error
 * @param tag: const std::string&
 * @usage R_ERROR("foo") << "bar";
 */
#define R_ERROR(_tag) _R_LOG(Error, _tag)

/**
 * @brief Defines a sink without captures
 * @param identifier for metadata : const R::Metadata&
 * @param identifier for message : const std::string&
 * @usage R_SINK(a, b) { std::cout << b; std::cout.flush(); };
 */
#define R_SINK(_metadata, _message) \
    [](const R::Metadata& _metadata, const std::string& _message)

/**
 * @brief Defines a sink with captures
 * @param identifier for metadata : const R::Metadata&
 * @param identifier for message : const std::string&
 * @param standard lambda captures i.e. &, =, ...
 * @usage R_SINK_W_CAPTURE(a, b, c) { std::cout << b << c;
 *                                    std::cout.flush();
 *                                  };
 */
#define R_SINK_W_CAPTURE(_metadata, _message, _capture) \
    [_capture](const R::Metadata& _metadata, const std::string& _message)

/**
 * @brief Defines Sink compatible call operator ()
 * @param identifier for metadata : const R::Metadata&
 * @param identifier for message : const std::string&
 */
#define R_SINK_OPERATOR(_metadata, _message) \
    void operator()(const R::Metadata& _metadata, const std::string& _message)

/**
 * @brief Defines a formatter without captures
 * @param identifier for metadata : const R::Metadata&
 * @param identifier for message : const std::string&
 * @return formatted message: std::string
 * @usage R_FORMATTER(a, b) { return to_upper(b); };
 */
#define R_FORMATTER(_metadata, _message) \
    [](const R::Metadata& _metadata, const std::string& _message) -> std::string

/**
 * @brief Defines a formatter with captures
 *        Shorter and more readable definition of formatter lambda with captures
 * @param identifier for metadata : const R::Metadata&
 * @param identifier for message : const std::string&
 * @param standard lambda captures i.e. &, =, ...
 * @return formatted message: std::string
 * @usage R_FORMATTER_W_CAPTURE(a, b, c) { return b + c; };
 */
#define R_FORMATTER_W_CAPTURE(_metadata, _message, _capture) \
    [_capture](const R::Metadata& _metadata,                 \
               const std::string& _message) -> std::string

/**
 * @brief Defines a filter without captures
 * @param identifier for metadata : const R::Metadata&
 * @param identifier for message : const std::string&
 * @return bool: can be used by sink to make decisions on logging
 * @usage R_FILTER(a, b) { return a.tag == "XYZ"; };
 */
#define R_FILTER(_metadata, _message) \
    [](const R::Metadata& _metadata, const std::string& _message) -> bool

/**
 * @brief Defines a filter with captures
 * @param identifier for metadata : const R::Metadata&
 * @param identifier for message : const std::string&
 * @param standard lambda captures i.e. &, =, ...
 * @return bool: can be used by sink to make decisions on logging
 * @usage R_FILTER_W_CAPTURE(a, b, c) { return a.tag == c; };
 */
#define R_FILTER_W_CAPTURE(_metadata, _message, _capture) \
    [_capture](const R::Metadata& _metadata,              \
               const std::string& _message) -> bool

/**
 * @brief Declare the << operator for specified type as friend
 * @param type's name
 * @usage struct Shape { R_USERTYPE_DECL(Shape); .... };
 */
#define R_USERTYPE_DECL(_type) \
    friend ostream& operator<<(ostream&, const _type&)

/**
 * @brief Defines the << operator for specified type
 * @param type's name
 * @usage R_USERTYPE_DEF(Shape, s, "[" << s.name << "]");
 */
#define R_USERTYPE_DEF(_type, _name, _code)                \
    ostream& operator<<(ostream& os, const _type& _name) { \
        os << _code;                                       \
        return os;                                         \
    }

// -----------------------------------------------------------
/// internal config

/**
 * @brief Sets Level to Off when R_ACTIVE is false
 */
#if R_ACTIVE == false
#ifdef R_MIN_LEVEL
#undef R_MIN_LEVEL
#endif
#define R_MIN_LEVEL (R::Level::Off)
#endif

// -----------------------------------------------------------

namespace R {

// -----------------------------------------------------------

/**
 * @brief Severity levels
 */
enum Level { Info, Warning, Error, Off };

// -----------------------------------------------------------

/**
 * @brief Holds metadata of a log, i.e.
 *          level, filename, line, timestamp & tag
 *        Gets passed to the filters, formatters & sinks
 */
struct Metadata {
    Metadata(Level level,
             const std::string& filename,
             long line,
             const std::string& tag = "")
        : level(level),
          filename([&] {
              // separate filename from full path
              return filename.substr(filename.find_last_of("/\\") + 1);
          }()),
          line(line),
          timestamp([] {
              // capture current time
              auto now = std::chrono::system_clock::now();
              auto in_time_t = std::chrono::system_clock::to_time_t(now);
              std::ostringstream os;
              os << std::put_time(std::localtime(&in_time_t), "%H-%M-%S");
              return os.str();
          }()),
          tag(tag) {}
    Level level = Level::Info;
    std::string filename;
    long line;
    std::string timestamp;
    std::string tag;
};  // Metadata

/**
 * @brief A type that outputs given metadata and message
 *        Being a std::function, it can capture any callable
 *        with signature void(const Metadata&, const std::string&)
 */
using Sink = std::function<void(const Metadata&, const std::string&)>;

// -----------------------------------------------------------

namespace internal {

/// all things in namespace internal are for internal use only

// -----------------------------------------------------------

/**
 * @brief Singleton that holds all global state of RLog
 *          i.e. severity level and active sinks
 *        Write accesses to these members is mutex protected
 */
struct Store {
    // ------------------------------
    // locks every write access to any store members
    std::recursive_mutex mutex;
    /**/ std::vector<Sink> sinks;
    /**/ Level level;
    // ------------------------------
    /**
     * @brief getter for store singleton
     * @return Store&
     */
    static Store& instance() {
        // init of static function locals is threadsafe in c++11
        static Store store;
        return store;
    }
};  // Store

// -----------------------------------------------------------

}  // namespace internal

// -----------------------------------------------------------

/**
 * @brief Inits / resets all global state of RLog
 *        Sets level to specified
 *        Clears all existing global Sinks
 *        Best called atleast once from a single-threaded init context
 * @param global level. default: Info
 */
static void reset(Level level = Level::Info) {
    std::lock_guard<std::recursive_mutex> lock(
        internal::Store::instance().mutex);
    internal::Store::instance().level = level;
    internal::Store::instance().sinks.clear();
}

// -----------------------------------------------------------

/**
 * @brief Adds a new global Sink
 * @param sink: copyable Sink instance
 */
static void addSink(const Sink& sink) {
    std::lock_guard<std::recursive_mutex> lock(
        internal::Store::instance().mutex);
    internal::Store::instance().sinks.push_back(sink);
}

// -----------------------------------------------------------

/**
 * @brief Returns global level
 * @return Level
 */
static inline Level level() { return internal::Store::instance().level; }

// -----------------------------------------------------------

namespace internal {

/// all things in namespace internal are for internal use only

// -----------------------------------------------------------

/**
 * @brief Single Log entry
 *        Every time a log is made, an instance of this class is created
 *          and the member ostringstream is filled
 *        Destructor passes the stream to all the active Sinks
 */
struct Log {
    Log(Level level,
        const std::string& filename,
        long line,
        const std::string& tag = "")
        : metadata(level, filename, line, tag) {
        metadata.level = level;
    }
    std::ostringstream& stream() { return os; }
    ~Log() {
        // prevent concurrent use
        std::lock_guard<std::recursive_mutex> lock(Store::instance().mutex);
        for (auto& sink : Store::instance().sinks) {
            sink(metadata, os.str());
        }
    }
    std::ostringstream os;
    Metadata metadata;
};  // Log

// -----------------------------------------------------------

}  // namespace internal

// -----------------------------------------------------------

/**
 * @brief Returns string name of specified level
 *
 * @param level
 * @return std::string
 */
static std::string to_string(Level level) {
    switch (level) {
        case Level::Info:
            return "Info";
        case Level::Warning:
            return "Warning";
        case Level::Error:
            return "Error";
        case Level::Off:
            return "Off";
        default:
            assert(false);
            return "None";
    }
}

// -----------------------------------------------------------

namespace internal {

/// all things in namespace internal are for internal use only

// -----------------------------------------------------------

/**
 * @brief Function: string_replace
 *        Replaces a pattern in a string with another string
 * @param in: string where the search and replacement is done: std::string&
 * @param pattern: const std:string&
 * @param with: string replaced with: const std:string&
 */
static void string_replace(std::string& in,
                           const std::string& pattern,
                           const std::string& with) {
    size_t pos = in.find(pattern);
    if (pos != std::string::npos) {
        in.replace(pos, pattern.length(), with);
    }
}

// -----------------------------------------------------------

}  // namespace internal

// -----------------------------------------------------------

/**
 * @brief A type that helps a sink filter logging based on metadata and
 * message Being a std::function, it can capture any callable with signature
 * bool(const Metadata&, const std::string&) Returned value can be used by a
 * Sink to filter logging
 */
using Filter = std::function<bool(const Metadata&, const std::string&)>;

// -----------------------------------------------------------

/**
 * @brief A utility function to compose a given filter and sink to form a
 * new sink
 */
static const auto makeFilteredSink = [](const Sink& sink,
                                        const Filter& filter) -> Sink {
    return R_SINK_W_CAPTURE(metadata, message, =) {
        if (filter(metadata, message)) {
            sink(metadata, message);
        }
    };
};

// -----------------------------------------------------------

/**
 * @brief A type that helps a sink format its logs based on metadata and
 * message Being a std::function, it can capture any callable with signature
 * std::string(const Metadata&, const std::string&) Returned value can be
 * used by a Sink instead of raw message
 */
using Formatter =
    std::function<std::string(const Metadata&, const std::string&)>;

// -----------------------------------------------------------

/**
 * @brief A utility function to compose a given filter and formatter to form
 * a new sink
 */
static const auto makeFormattedSink = [](const Sink& sink,
                                         const Formatter& formatter) -> Sink {
    return R_SINK_W_CAPTURE(metadata, message, =) {
        sink(metadata, formatter(metadata, message));
    };
};

// -----------------------------------------------------------

/**
 * @brief A built-in basic cout sink
 * @note Adds an endl and therefore a flush after every log
 */
static const Sink CoutSink = R_SINK(metadata, message) {
    std::cout << message << std::endl;
};

/**
 * @brief A built-in basic file sink
 * @note Adds an endl and therefore a flush after every log
 * @param std::ofstream&: A pre-opened output filestream
 */
static const auto FileSink = [](std::ofstream& fs) -> Sink {
    return R_SINK_W_CAPTURE(metadata, message, &) {
        fs << message << std::endl;
    };
};

// -----------------------------------------------------------

/**
 * @brief Default format of SmartFormatter
 */
static const auto defaultSmartFormat =
    "[R] #timestamp [#level] #tag (#filename:#line) #message";

/**
 * @brief A utility to make a built-in intelligent formatter
 *        Using a string format, allows puting together metadata values in
 *          custom fashion
 * @param format: const std::string& : default: defaultSmartFormat
 */
static const auto makeSmartFormatter = [](const std::string& format =
                                              defaultSmartFormat) -> Formatter {
    return R_FORMATTER_W_CAPTURE(metadata, message, format) {
        std::string result = format;
        internal::string_replace(result, "#timestamp", metadata.timestamp);
        internal::string_replace(result, "#level", to_string(metadata.level));
        internal::string_replace(
            result,
            "#tag",
            metadata.tag.empty() ? "" : std::string("#") + metadata.tag);
        internal::string_replace(result, "#filename", metadata.filename);
        internal::string_replace(
            result, "#line", std::to_string(metadata.line));
        internal::string_replace(result, "#message", message);
        return result;
    };
};

// -----------------------------------------------------------

/**
 * @brief A utility to make a built-in formatted cout sync
 * @param format: const std::string& : default: defaultSmartFormat
 */
static const auto makeSmartFormattedCoutSink =
    [](const std::string& format = defaultSmartFormat) -> Sink {
    return makeFormattedSink(CoutSink, makeSmartFormatter(format));
};

// -----------------------------------------------------------

/**
 * @brief A built-in json formatter
 *        Simply gives a special format for SmartFormatter :)
 */
static const Formatter JsonFormatter = makeSmartFormatter(
    R"(
    {
        "timestamp": "#timestamp",
        "level": "#level",
        "tag": "#tag",
        "filename": "#filename",
        "line": #line,
        "message": "#message"
    })");

// -----------------------------------------------------------

/**
 * @brief A built-in json sink
 *        Uses JsonFormatter
 * @note Should be passed to RLog only as a reference, using std::ref
 * @usage R::JsonSink json(fs);
 *        R::addSink(std::ref(json));
 */
struct JsonSink {
    std::ofstream& fs;
    bool first = true;
    JsonSink(std::ofstream& fs) : fs(fs) { fs << "["; }
    ~JsonSink() { fs << "\n]"; }
    _R_DISALLOW_COPY_ASSIGN(JsonSink);
    R_SINK_OPERATOR(metadata, message) {
        if (first) {
            first = false;
        } else {
            fs << ",";
        }
        fs << JsonFormatter(metadata, message);
    }
};  // JsonSink

// -----------------------------------------------------------

}  // namespace R

// -----------------------------------------------------------

#endif  // __R_LOG_HPP__

// -----------------------------------------------------------
// EOF