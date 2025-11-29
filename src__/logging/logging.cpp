/**
 * @file logging.cpp
 * @brief Implementation of the global logging interface declared in logging.h.
 *
 * This source file defines the global logger instance and implements the functions
 * for setting and using the logger. Detailed documentation is provided for each function.
 */

#include "logging.h"

namespace logging {
/**
 * @brief Global logger instance.
 *
 * The shared pointer to the Logger instance that is used across the application.
 */
std::shared_ptr<Logger> logger;

/**
 * @brief Sets the global logger.
 *
 * This function initializes the global logger to a new instance created from the provided
 * Logger object.
 *
 * @param log The Logger object used to initialize the global logger.
 */
void set_logger(const Logger& log) { logger = std::make_shared<Logger>(log); }

/**
 * @brief Logs a message using the global logger.
 *
 * If the global logger has been set, this function logs the message to the specified
 * channel and logging level.
 *
 * @param channel_id The ID of the logging channel.
 * @param level The logging level (bitmask).
 * @param message The message to log.
 */
void log(int channel_id, int level, const std::string& message) {
    if (logger) {
        logger->log(channel_id, level, message);
    }
}
} // namespace logging
