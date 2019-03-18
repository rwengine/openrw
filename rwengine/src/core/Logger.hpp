#ifndef _RWENGINE_LOGGER_HPP_
#define _RWENGINE_LOGGER_HPP_

#include <array>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

/**
 * Handles and stores messages from different components
 *
 * Dispatches received messages to logger outputs.
 */
class Logger {
public:
    enum MessageSeverity { Verbose = 0, Info, Warning, Error};
    static constexpr std::array<char, 4> messageSeverityName{{'V', 'I', 'W', 'E'}};

    struct LogMessage {
        /// The component that produced the message
        std::string component;
        /// Severity of the message.
        MessageSeverity severity;
        /// Logged message
        std::string message;

        template <class String1, class String2>
        LogMessage(String1&& cc, MessageSeverity ss,
                   String2&& mm)
            : component(std::forward<String1>(cc))
            , severity(ss)
            , message(std::forward<String2>(mm)) {
        }
    };

    /**
     * Interface for handling logged messages.
     *
     * The Logger class will not clean up allocated MessageReceivers.
     */
    struct MessageReceiver {
        virtual void messageReceived(const LogMessage&) = 0;
    };

    Logger(std::initializer_list<MessageReceiver*> initial = {})
        : receivers(initial) {
    }

    void addReceiver(MessageReceiver* out);
    void removeReceiver(MessageReceiver* out);

    void log(const std::string& component, Logger::MessageSeverity severity,
             const std::string& message);

    void verbose(const std::string& component, const std::string& message);
    void info(const std::string& component, const std::string& message);
    void warning(const std::string& component, const std::string& message);
    void error(const std::string& component, const std::string& message);

private:
    std::vector<MessageReceiver*> receivers;
};

class StdOutReceiver final : public Logger::MessageReceiver {
    void messageReceived(const Logger::LogMessage&) override;
};

#endif
