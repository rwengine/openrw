#pragma once

#include <string>
#include <vector>

/**
 * Handles and stores messages from different components
 *
 * Dispatches recieved messages to logger outputs.
 */
class Logger {
public:
    enum MessageSeverity { Verbose, Info, Warning, Error };

    struct LogMessage {
        /// The component that produced the message
        std::string component;
        /// Severity of the message.
        MessageSeverity severity;
        /// Logged message
        std::string message;

        LogMessage(const std::string& cc, MessageSeverity ss,
                   const std::string& mm)
            : component(cc), severity(ss), message(mm) {
        }
    };

    /**
     * Interface for handling logged messages.
     *
     * The Logger class will not clean up allocated MessageRecievers.
     */
    struct MessageReciever {
        virtual void messageRecieved(const LogMessage&) = 0;
    };

    void addReciever(MessageReciever* out);
    void removeReciever(MessageReciever* out);

    void log(const std::string& component, Logger::MessageSeverity severity,
             const std::string& message);

    void verbose(const std::string& component, const std::string& message);
    void info(const std::string& component, const std::string& message);
    void warning(const std::string& component, const std::string& message);
    void error(const std::string& component, const std::string& message);

private:
    std::vector<MessageReciever*> recievers;
};

class StdOutReciever : public Logger::MessageReciever {
    virtual void messageRecieved(const Logger::LogMessage&);
};
