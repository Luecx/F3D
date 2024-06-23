#include "Logger.h"
#include <ctime>
#include <sstream>
#include <stdexcept>

std::shared_ptr<std::ofstream> FileManager::get_file_stream(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (file_streams_.find(filename) == file_streams_.end()) {
        file_streams_[filename] = std::make_shared<std::ofstream>(filename, std::ios::app);
    }
    return file_streams_[filename];
}

Logger::Logger() : use_timestamp_(false), current_channel_(-1), file_manager_(std::make_shared<FileManager>()) {}

Logger& Logger::channel(int channel_id) {
    if (channel_id < 0 || channel_id >= 16) {
        throw std::invalid_argument("Channel ID must be between 0 and 15");
    }
    current_channel_ = channel_id;
    if (channels_.find(channel_id) == channels_.end()) {
        channels_[channel_id] = std::make_shared<Channel>();
    }
    return *this;
}

Logger& Logger::file_output(const std::string& filename, int levels) {
    if (current_channel_ < 0) {
        throw std::logic_error("No channel selected");
    }
    auto file_stream = file_manager_->get_file_stream(filename);
    channels_[current_channel_]->outputs.emplace_back(std::make_shared<FileOutput>(file_stream, levels));
    return *this;
}

Logger& Logger::cout(int levels) {
    if (current_channel_ < 0) {
        throw std::logic_error("No channel selected");
    }
    channels_[current_channel_]->outputs.emplace_back(std::make_shared<StreamOutput>(std::cout, levels));
    return *this;
}

Logger& Logger::cerr(int levels) {
    if (current_channel_ < 0) {
        throw std::logic_error("No channel selected");
    }
    channels_[current_channel_]->outputs.emplace_back(std::make_shared<StreamOutput>(std::cerr, levels));
    return *this;
}

Logger& Logger::timestamp() {
    use_timestamp_ = true;
    return *this;
}

void Logger::log(int channel_id, int level, const std::string& message) const {
    if (channels_.find(channel_id) != channels_.end()) {
        const auto& channel = channels_.at(channel_id);
        for (const auto& output : channel->outputs) {
            if (output->should_log(level)) {
                output->log(level, format_message(level, message));
            }
        }
    }
}

std::string Logger::format_message(int level, const std::string& message) const {
    std::stringstream ss;
    if (use_timestamp_) {
        ss << current_timestamp() << " ";
    }
    ss << level_to_string(level) << ": " << message;
    return ss.str();
}

std::string Logger::current_timestamp() const {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}

std::string Logger::level_to_string(int level) const {
    switch (level) {
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERROR: return "ERROR";
        case DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}
