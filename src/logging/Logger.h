#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include "LogOutput.h"

enum LogLevel {
    INFO = 1 << 0,
    WARNING = 1 << 1,
    ERROR = 1 << 2,
    DEBUG = 1 << 3,
    ALL = INFO | WARNING | ERROR | DEBUG
};

class FileManager {
    public:
    std::shared_ptr<std::ofstream> get_file_stream(const std::string& filename);

    private:
    std::map<std::string, std::shared_ptr<std::ofstream>> file_streams_;
    std::mutex mtx_;
};

class Logger {
    public:
    Logger();

    Logger& channel(int channel_id);
    Logger& file_output(const std::string& filename, int levels);
    Logger& cout(int levels);
    Logger& cerr(int levels);
    Logger& timestamp();

    void log(int channel_id, int level, const std::string& message) const;

    private:
    struct Channel {
        std::vector<std::shared_ptr<LogOutput>> outputs;
    };

    std::string format_message(int level, const std::string& message) const;
    std::string current_timestamp() const;
    std::string level_to_string(int level) const;

    std::map<int, std::shared_ptr<Channel>> channels_;
    bool use_timestamp_;
    int current_channel_;
    std::shared_ptr<FileManager> file_manager_;
};

#endif // LOGGER_H
