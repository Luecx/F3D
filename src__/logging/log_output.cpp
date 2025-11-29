/**
 * @file log_output.cpp
 * @brief Implementation of the logging classes declared in log_output.h.
 *
 * This source file implements the logging classes defined in log_output.h. It provides concrete
 * implementations for the logging functions in both FileOutput and StreamOutput. Detailed documentation
 * is provided for each function to explain their purpose, parameters, and behavior.
 */

#include "log_output.h"

namespace logging {

/**
 * @brief Constructs a LogOutput object with the specified logging levels.
 *
 * @param levels Bitmask representing the logging levels.
 */
LogOutput::LogOutput(int levels) : _levels(levels) {}

/**
 * @brief Determines if a message should be logged based on its logging level.
 *
 * This function checks whether the bit corresponding to the provided logging level is set
 * in the _levels bitmask.
 *
 * @param level The logging level to check.
 * @return True if the message should be logged; false otherwise.
 */
bool LogOutput::should_log(int level) const { return (_levels & level) != 0; }

/**
 * @brief Constructs a FileOutput object.
 *
 * Initializes the FileOutput object with a file stream and logging levels.
 *
 * @param file_stream Shared pointer to the output file stream.
 * @param levels Bitmask representing the logging levels.
 */
FileOutput::FileOutput(std::shared_ptr<std::ofstream> file_stream, int levels)
    : LogOutput(levels), _file_stream(file_stream) {}

/**
 * @brief Logs a message to the file stream.
 *
 * This function writes the provided message to the file stream. It uses a mutex to ensure
 * that the operation is thread-safe. The function first checks if the file stream is valid
 * and open before writing the message.
 *
 * @param level The logging level of the message.
 * @param message The message to log.
 */
void FileOutput::log(int level, const std::string& message) const {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_file_stream && _file_stream->is_open()) {
        *_file_stream << message << std::endl;
    }
}

/**
 * @brief Constructs a StreamOutput object.
 *
 * Initializes the StreamOutput object with an output stream and logging levels.
 *
 * @param stream Reference to the output stream.
 * @param levels Bitmask representing the logging levels.
 */
StreamOutput::StreamOutput(std::ostream& stream, int levels) : LogOutput(levels), _stream(stream) {}

/**
 * @brief Logs a message to the output stream.
 *
 * This function writes the provided message to the output stream. A mutex is used to ensure
 * that the write operation is thread-safe.
 *
 * @param level The logging level of the message.
 * @param message The message to log.
 */
void StreamOutput::log(int level, const std::string& message) const {
    std::lock_guard<std::mutex> lock(_mtx);
    _stream << message << std::endl;
}

} // namespace logging
