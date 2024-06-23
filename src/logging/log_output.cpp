#include "log_output.h"

logging::LogOutput::LogOutput(int levels) : levels_(levels) {}

bool logging::LogOutput::should_log(int level) const {
    return (levels_ & level) != 0;
}

logging::FileOutput::FileOutput(std::shared_ptr<std::ofstream> file_stream, int levels)
    : LogOutput(levels), file_stream_(file_stream) {}

void logging::FileOutput::log(int level, const std::string& message) const {
    std::lock_guard<std::mutex> lock(mtx_);
    if (file_stream_->is_open()) {
        *file_stream_ << message << std::endl;
    }
}

logging::StreamOutput::StreamOutput(std::ostream& stream, int levels)
    : LogOutput(levels), stream_(stream) {}

void logging::StreamOutput::log(int level, const std::string& message) const {
    std::lock_guard<std::mutex> lock(mtx_);
    stream_ << message << std::endl;
}
