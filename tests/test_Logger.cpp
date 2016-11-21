#include <boost/test/unit_test.hpp>
#include <core/Logger.hpp>
#include <test_globals.hpp>

class CallbackReceiver : public Logger::MessageReciever {
public:
    std::function<void(const Logger::LogMessage&)> func;

    CallbackReceiver(std::function<void(const Logger::LogMessage&)> func)
        : func(func) {
    }

    virtual void messageRecieved(const Logger::LogMessage& message) {
        func(message);
    }
};

BOOST_AUTO_TEST_SUITE(LoggerTests)

BOOST_AUTO_TEST_CASE(test_reciever) {
    Logger log;

    Logger::LogMessage lastMessage("", Logger::Error, "");

    CallbackReceiver reciever(
        [&](const Logger::LogMessage& m) { lastMessage = m; });

    log.addReciever(&reciever);

    log.info("Tests", "Test");

    BOOST_CHECK_EQUAL(lastMessage.component, "Tests");
    BOOST_CHECK_EQUAL(lastMessage.severity, Logger::Info);
    BOOST_CHECK_EQUAL(lastMessage.message, "Test");
}

BOOST_AUTO_TEST_SUITE_END()