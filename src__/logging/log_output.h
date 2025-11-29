#pragma once
/**
 * @file log_output.h
 * @brief Declaration of logging classes for file and stream outputs.
 *
 * This header file declares a simple logging framework that provides two derived classes for logging:
 * - logging::LogOutput: An abstract base class defining the logging interface.
 * - logging::FileOutput: A derived class that logs messages to a file stream.
 * - logging::StreamOutput: A derived class that logs messages to an output stream.
 *
 * The design ensures thread safety using mutex locks and follows naming conventions where all member
 * variables are prefixed with an underscore (_).
 *
 * @note This file uses #pragma once to ensure it is included only once during compilation.
 */

#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>

namespace logging {

/**
 * @class LogOutput
 * @brief Abstract base class for logging output.
 *
 * This class provides the common interface and functionality for logging messages. It holds a bitmask
 * of logging levels that determine which messages should be logged. Derived classes must implement
 * the pure virtual log() function.
 */
class LogOutput {
  public:
    /**
     * @brief Constructs a LogOutput object.
     * @param levels Bitmask representing the logging levels.
     */
    LogOutput(int levels);

    /**
     * @brief Virtual destructor.
     */
    virtual ~LogOutput() = default;

    /**
     * @brief Determines whether a message should be logged based on the given level.
     * @param level The level of the message.
     * @return True if the message should be logged; false otherwise.
     */
    bool should_log(int level) const;

    /**
     * @brief Logs a message at the specified logging level.
     * @param level The logging level.
     * @param message The message to log.
     *
     * This is a pure virtual function that must be implemented by derived classes.
     */
    virtual void log(int level, const std::string& message) const = 0;

  protected:
    int _levels; ///< Bitmask representing active logging levels.
};

/**
 * @class FileOutput
 * @brief Derived class that logs messages to a file stream.
 *
 * This class implements logging functionality that writes messages to a file stream. It ensures
 * thread safety through the use of a mutex.
 */
class FileOutput : public LogOutput {
  public:
    /**
     * @brief Constructs a FileOutput object.
     * @param file_stream Shared pointer to an output file stream.
     * @param levels Bitmask representing the logging levels.
     */
    FileOutput(std::shared_ptr<std::ofstream> file_stream, int levels);

    /**
     * @brief Logs a message to the file stream.
     * @param level The logging level.
     * @param message The message to log.
     */
    void log(int level, const std::string& message) const override;

  private:
    std::shared_ptr<std::ofstream> _file_stream; ///< Shared pointer to the file stream.
    mutable std::mutex _mtx;                     ///< Mutex to ensure thread-safe writes.
};

/**
 * @class StreamOutput
 * @brief Derived class that logs messages to an output stream.
 *
 * This class implements logging functionality that writes messages to a generic output stream
 * (such as std::cout). Thread safety is maintained using a mutex.
 */
class StreamOutput : public LogOutput {
  public:
    /**
     * @brief Constructs a StreamOutput object.
     * @param stream Reference to an output stream.
     * @param levels Bitmask representing the logging levels.
     */
    StreamOutput(std::ostream& stream, int levels);

    /**
     * @brief Logs a message to the output stream.
     * @param level The logging level.
     * @param message The message to log.
     */
    void log(int level, const std::string& message) const override;

  private:
    std::ostream& _stream;   ///< Reference to the output stream.
    mutable std::mutex _mtx; ///< Mutex to ensure thread-safe writes.
};

} // namespace logging
