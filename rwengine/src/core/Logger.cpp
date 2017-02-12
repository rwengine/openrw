#include <core/Logger.hpp>

#include <algorithm>
#include <iostream>
#include <map>

void Logger::log(const std::string& component, Logger::MessageSeverity severity,
                 const std::string& message) {
    LogMessage m{component, severity, message};

    for (MessageReciever* r : recievers) {
        r->messageRecieved(m);
    }
}

void Logger::addReciever(Logger::MessageReciever* out) {
    recievers.push_back(out);
}

void Logger::removeReciever(Logger::MessageReciever* out) {
    recievers.erase(std::remove(recievers.begin(), recievers.end(), out),
                    recievers.end());
}

void Logger::error(const std::string& component, const std::string& message) {
    log(component, Logger::Error, message);
}

void Logger::info(const std::string& component, const std::string& message) {
    log(component, Logger::Info, message);
}

void Logger::warning(const std::string& component, const std::string& message) {
    log(component, Logger::Warning, message);
}

void Logger::verbose(const std::string& component, const std::string& message) {
    log(component, Logger::Verbose, message);
}

std::map<Logger::MessageSeverity, std::string> severityStr{
    {Logger::Error,     "\x1b[30;1m[Erro"},
    {Logger::Warning,   "\x1b[33;1m[Warn"},
    {Logger::Info,      "\x1b[36;1m[Info"},
    {Logger::Verbose,   "\x1b[37;1m[Verb"}};

void StdOutReciever::messageRecieved(const Logger::LogMessage& message) {
    std::cout << severityStr[message.severity] << "->" << message.component
              << "]\x1b[0m " << message.message << std::endl;
}
