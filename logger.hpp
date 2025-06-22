#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

// TODO Exercise 3
// Abstract Logger class
class Logger {
public:
    virtual ~Logger() = default;
    
    // Log a message
    virtual void log(const std::string& message) = 0;
    
    // Create specific logger types
    static std::shared_ptr<Logger> create_silent_logger();
    static std::shared_ptr<Logger> create_stdout_logger();
    static std::shared_ptr<Logger> create_file_logger();
};

// Silent logger - no output
class SilentLogger : public Logger {
public:
    void log(const std::string& /* message */) override {};
};

// Logger to stdout
class StdoutLogger : public Logger {
public:
    void log(const std::string& message) override {
        std::cout << message << std::endl;
    }
};

// Logger to file (hardcoded file name)
class FileLogger : public Logger {
public:
    FileLogger();
    ~FileLogger() override;
    void log(const std::string& message) override;
    
private:
    std::ofstream file;
    static const std::string filename;
};

#endif // LOGGER_HPP
