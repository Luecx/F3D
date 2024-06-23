#ifndef LOGOUTPUT_H
#define LOGOUTPUT_H

#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>

namespace logging {

class LogOutput {
    public:
    LogOutput(int levels);
    virtual ~LogOutput() = default;

    bool         should_log(int level) const;
    virtual void log(int level, const std::string& message) const = 0;

    protected:
    int levels_;
};

class FileOutput : public LogOutput {
    public:
    FileOutput(std::shared_ptr<std::ofstream> file_stream, int levels);
    void log(int level, const std::string& message) const override;

    private:
    std::shared_ptr<std::ofstream> file_stream_;
    mutable std::mutex             mtx_;
};

class StreamOutput : public LogOutput {
    public:
    StreamOutput(std::ostream& stream, int levels);
    void log(int level, const std::string& message) const override;

    private:
    std::ostream&      stream_;
    mutable std::mutex mtx_;
};

}
#endif // LOGOUTPUT_H
