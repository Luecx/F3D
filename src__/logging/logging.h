#ifndef LOGGING_H
#define LOGGING_H

#include <memory>
#include "logger.h"

namespace logging {
extern std::shared_ptr<Logger> logger;

void set_logger(const Logger& log);
void log(int channel_id, int level, const std::string& message);
} // namespace logging

#endif // LOGGING_H
