#include "logger.h"

#include <chrono>
#include <ctime>
#include <sstream>

namespace logging {

std::shared_ptr<std::ofstream> FileManager::get_file_stream(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = file_streams_.find(filename);
    if (it != file_streams_.end()) {
        return it->second;
    }
    auto stream = std::make_shared<std::ofstream>(filename, std::ios::app);
    file_streams_[filename] = stream;
    return stream;
}

Logger::Logger() : use_timestamp_(false), current_channel_(0), file_manager_(std::make_shared<FileManager>()) {}

Logger& Logger::channel(int channel_id) {
    current_channel_ = channel_id;
    if (channels_.find(channel_id) == channels_.end()) {
        channels_[channel_id] = std::make_shared<Channel>();
    }
    return *this;
}

Logger& Logger::file_output(const std::string& filename, int levels) {
    auto stream = file_manager_->get_file_stream(filename);
    channels_[current_channel_]->outputs.push_back(std::make_shared<FileOutput>(stream, levels));
    return *this;
}

Logger& Logger::cout(int levels) {
    channels_[current_channel_]->outputs.push_back(std::make_shared<StreamOutput>(std::cout, levels));
    return *this;
}

Logger& Logger::cerr(int levels) {
    channels_[current_channel_]->outputs.push_back(std::make_shared<StreamOutput>(std::cerr, levels));
    return *this;
}

Logger& Logger::timestamp() {
    use_timestamp_ = true;
    return *this;
}

void Logger::log(int channel_id, int level, const std::string& message) const {
    auto it = channels_.find(channel_id);
    if (it == channels_.end()) {
        return;
    }
    const std::string formatted = format_message(level, message);
    for (const auto& output : it->second->outputs) {
        if (output->should_log(level)) {
            output->log(level, formatted);
        }
    }
}

std::string Logger::format_message(int level, const std::string& message) const {
    std::ostringstream ss;
    if (use_timestamp_) {
        ss << current_timestamp() << " ";
    }
    ss << level_to_string(level) << ": " << message;
    return ss.str();
}

std::string Logger::current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now{};
#if defined(_WIN32)
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_now);
    return {buffer};
}

std::string Logger::level_to_string(int level) const {
    if (level & ERROR)
        return "ERROR";
    if (level & WARNING)
        return "WARNING";
    if (level & INFO)
        return "INFO";
    if (level & DEBUG)
        return "DEBUG";
    return "LOG";
}

} // namespace logging
