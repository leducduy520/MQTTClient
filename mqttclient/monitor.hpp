/**
 * @file monitor.hpp
 * @brief Provides a logging and debugging utility via the ddbg::Printer class.
 *
 * This header defines the ddbg::Printer class for formatted logging with
 * support for:
 * - Variable argument formatting using a printf-like interface.
 * - ANSI color coding for console output.
 * - Timestamps and source-line information.
 * - Custom message modifications (above, below, newline).
 *
 * Several preprocessor macros are provided to simplify logging at different
 * detail levels and message types (DEBUG, INFO, ERROR). The logging output is
 * written to stderr.
 *
 * @author duyld15
 * @date February 23, 2025
 */

#include <cstdarg>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace ddbg {
/**
 * @brief Empty structure used as a log output terminator.
 */
struct end {};

/**
 * @brief Empty structure used to indicate a line break in the log output.
 */
struct endl {};

/**
 * @brief Class for formatted logging and debugging output.
 *
 * The Printer class supports building a log message using operator<<
 * and printing it with formatting such as color, timestamp, and source
 * line information. It also provides a static format method that acts like
 * a printf-style formatter.
 */
class Printer {
  string message_;           ///< Accumulated log message.
  string startcolor_;        ///< ANSI color code for starting the output.
  const string finishcolor_; ///< ANSI code to reset the output formatting.
  string modeprefix_; ///< Prefix to denote the log message type (Error, Info,
                      ///< Debug).
  string lineinfo_;   ///< Information about the file and line number.
  string timestamp_;  ///< Timestamp for when the message was logged.

public:
  /**
   * @brief Enumerates the supported message types.
   */
  enum MessageMode {
    ERROR, ///< Indicates an error message.
    INFO,  ///< Indicates an informational message.
    DEBUG, ///< Indicates a debug message.
  };

  /**
   * @brief Constructs a Printer with an initial message.
   * @param message The initial log message.
   */
  explicit Printer(const string &message)
      : message_(message), finishcolor_("\033[0m") {}

  /**
   * @brief Creates a formatted Printer using a printf-style format.
   *
   * Accepts a format string and variable arguments to produce a log message.
   *
   * @param msg The format string.
   * @param ... Variable arguments corresponding to the format.
   * @return A Printer instance with the formatted message.
   */
  static Printer format(const char *msg, ...) {
    // Initialize variable argument list
    va_list args;
    va_start(args, msg);

    // Determine the size of the formatted string
    va_list argsCopy;
    va_copy(argsCopy, args);
    int size =
        vsnprintf(nullptr, 0, msg, argsCopy) + 1; // Include null terminator
    va_end(argsCopy);

    // Allocate a buffer to hold the formatted string
    std::vector<char> buffer(size);

    // Format the string
    vsnprintf(buffer.data(), size, msg, args);

    // Clean up the variable argument list
    va_end(args);

    // Return the formatted string
    auto mes = std::string(buffer.data());
    return Printer(mes);
  }

  /**
   * @brief Sets the message type and corresponding formatting.
   *
   * Adjusts the message prefix and starting color based on the mode.
   *
   * @param mesmode The message type (ERROR, INFO, DEBUG).
   * @return A reference to the modified Printer object.
   */
  Printer &type(MessageMode mesmode) {
    switch (mesmode) {
    case MessageMode::ERROR: {
      modeprefix_ = string("Error: ");
      startcolor_ = "\033[31m";
    } break;
    case MessageMode::INFO: {
      modeprefix_ = string("Info: ");
      startcolor_ = "\033[34m";
    } break;
    case MessageMode::DEBUG: {
      modeprefix_ = string("Debug: ");
      startcolor_ = "\033[32m";
    } break;
    default:
      break;
    }
    return *this;
  }

  /**
   * @brief Appends a timestamp to the log message.
   *
   * Formats the current time and adds it to the log output.
   *
   * @return A reference to the modified Printer object.
   */
  Printer &timestamp() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    string timestamp = asctime(timeinfo);
    timestamp.pop_back(); // Remove newline character
    timestamp_ = "[" + timestamp + "]\n";
    return *this;
  }

  /**
   * @brief Adds file and line information to the log message.
   *
   * Formats the file name and line number where the log message was generated.
   *
   * @param file The source file name.
   * @param line The line number in the source file.
   * @return A reference to the modified Printer object.
   */
  Printer &lineinfo(const char *file, unsigned int line) {
    stringstream ss;
    ss << "at " << file << ":" << line;
    lineinfo_ = "(" + ss.str() + ")\n";
    return *this;
  }

  /**
   * @brief Prepends a string above the main log message.
   *
   * @param str The string to place above the message.
   * @return A reference to the modified Printer object.
   */
  Printer &above(const string &str) {
    message_ = str + "\n" + message_;
    return *this;
  }

  /**
   * @brief Appends a string below the main log message.
   *
   * @param str The string to place below the message.
   * @return A reference to the modified Printer object.
   */
  Printer &below(const string &str) {
    message_ += "\n" + str;
    return *this;
  }

  /**
   * @brief Appends a newline to the log message.
   *
   * @return A reference to the modified Printer object.
   */
  Printer &endl() {
    message_ += "\n";
    return *this;
  }

  /**
   * @brief Overloaded operator to append data to the log message.
   *
   * Uses a stringstream to convert data to string and appends it to the log
   * message.
   *
   * @tparam T The type of data to append.
   * @param data The data to append.
   * @return A reference to the modified Printer object.
   */
  template <typename T> Printer &operator<<(T &&data) {
    stringstream ss;
    ss << data;
    message_.append(ss.str());
    return *this;
  }

  /**
   * @brief Overloaded operator to print the log message and end output.
   *
   * @param data A ddbg::end structure indicating the termination of the log
   * output.
   */
  void operator<<(struct ddbg::end data) { this->print(); }

  /**
   * @brief Overloaded operator to add a newline and print the log message.
   *
   * @param data A ddbg::endl structure indicating a newline and termination of
   * the log output.
   */
  void operator<<(struct ddbg::endl data) {
    this->endl();
    this->print();
  }

  /**
   * @brief Overloaded operator for stream manipulators.
   *
   * Applies the provided stream manipulator, prints the log message, and
   * outputs it.
   *
   * @param manip A stream manipulator function.
   */
  void operator<<(std::ostream &(*manip)(std::ostream &)) {
    this->print();
    manip(std::cerr);
  }

  /**
   * @brief Prints the accumulated log message to stderr with formatting.
   */
  void print() {
    fprintf(stderr, "\n%s%s%s%s%s%s", startcolor_.c_str(), timestamp_.c_str(),
            lineinfo_.c_str(), modeprefix_.c_str(), message_.c_str(),
            finishcolor_.c_str());
  }
};

/// Macro alias for ddbg::Printer.
#define DDBG_PRINTER ddbg::Printer

/// Macro for providing file and line details.
#define DDBG_DETAIL_LINE __FILE__, __LINE__

/// Macro to signal the end of a logged message.
#define DDBG_END ddbg::end()
/// Macro to signal the end of a line in logged messages.
#define DDBG_ENDL ddbg::endl()

/// Macro for basic timestamp detail.
#define DDBG_DETAIL_LEVEL_1 timestamp()
/// Macro for timestamp with additional line information.
#define DDBG_DETAIL_LEVEL_2 timestamp().lineinfo(DDBG_DETAIL_LINE)

/// Macro to set the message type to DEBUG.
#define DDBG_AS_DEBUG type(DDBG_PRINTER::MessageMode::DEBUG)
/// Macro to set the message type to ERROR.
#define DDBG_AS_ERROR type(DDBG_PRINTER::MessageMode::ERROR)
/// Macro to set the message type to INFO.
#define DDBG_AS_INFO type(DDBG_PRINTER::MessageMode::INFO)

#ifndef NDEBUG
/// Macro for debug-level logging.
#define ddebug(msg, ...) DDBG_PRINTER::format(msg, ##__VA_ARGS__).DDBG_AS_DEBUG
/// Macro for debug-level logging with timestamp.
#define ddebug1(msg, ...)                                                      \
  DDBG_PRINTER::format(msg, ##__VA_ARGS__).DDBG_AS_DEBUG.DDBG_DETAIL_LEVEL_1
/// Macro for debug-level logging with timestamp and line info.
#define ddebug2(msg, ...)                                                      \
  DDBG_PRINTER::format(msg, ##__VA_ARGS__).DDBG_AS_DEBUG.DDBG_DETAIL_LEVEL_2
#else
#define ddebug(msg, ...) DDBG_PRINTER::format("[MDEBUG]")
#define ddebug1(msg, ...) DDBG_PRINTER::format("[MDEBUG1]")
#define ddebug2(msg, ...) DDBG_PRINTER::format("[MDEBUG2]")
#endif

/// Macro for informational logging.
#define dinfo(msg, ...) DDBG_PRINTER::format(msg, ##__VA_ARGS__).DDBG_AS_INFO
/// Macro for informational logging with timestamp.
#define dinfo1(msg, ...)                                                       \
  DDBG_PRINTER::format(msg, ##__VA_ARGS__).DDBG_AS_INFO.DDBG_DETAIL_LEVEL_1
/// Macro for informational logging with timestamp and line info.
#define dinfo2(msg, ...)                                                       \
  DDBG_PRINTER::format(msg, ##__VA_ARGS__).DDBG_AS_INFO.DDBG_DETAIL_LEVEL_2

/// Macro for error logging.
#define derror(msg, ...) DDBG_PRINTER::format(msg, ##__VA_ARGS__).DDBG_AS_ERROR
/// Macro for error logging with timestamp.
#define derror1(msg, ...)                                                      \
  DDBG_PRINTER::format(msg, ##__VA_ARGS__).DDBG_AS_ERROR.DDBG_DETAIL_LEVEL_1
/// Macro for error logging with timestamp and line info.
#define derror2(msg, ...)                                                      \
  DDBG_PRINTER::format(msg, ##__VA_ARGS__).DDBG_AS_ERROR.DDBG_DETAIL_LEVEL_2

} // namespace ddbg
