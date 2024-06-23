#include "logging.h"

namespace logging {
std::shared_ptr<Logger> logger;

void set_logger(const Logger& log) {
    logger = std::make_shared<Logger>(log);
}

void log(int channel_id, int level, const std::string& message) {
    if (logger) {
        logger->log(channel_id, level, message);
    }
}
}
