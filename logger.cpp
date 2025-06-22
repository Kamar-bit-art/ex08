#include "logger.hpp"

// Define the static filename for FileLogger
const std::string FileLogger::filename = "logic_log.txt";

// Factory methods for Logger
std::shared_ptr<Logger> Logger::create_silent_logger() {
    return std::make_shared<SilentLogger>();
}

std::shared_ptr<Logger> Logger::create_stdout_logger() {
    return std::make_shared<StdoutLogger>();
}

std::shared_ptr<Logger> Logger::create_file_logger() {
    return std::make_shared<FileLogger>();
}

// FileLogger implementation
FileLogger::FileLogger() {
    file.open(filename, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open log file " << filename << std::endl;
    }
}

FileLogger::~FileLogger() {
    if (file.is_open()) {
        file.close();
    }
}

void FileLogger::log(const std::string& message) {
    if (file.is_open()) {
        file << message << std::endl;
        file.flush();
    }
}
