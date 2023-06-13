#ifndef MOCKSESSION_H
#define MOCKSESSION_H

#include "gmock/gmock.h"
#include "session.h"

//mock class for testing purposes only
//not meant for production use
class MockSession : public session {
public:
    MockSession(boost::asio::io_service& io_service,
                std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap,
                std::map<std::string, RequestHandlerFactory*> routes)
        : session(io_service, hdlrMap, routes) {
    }

    MOCK_METHOD(boost::asio::ip::tcp::socket&, socket, (), (override));
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, handle_read, (const boost::system::error_code& error,
        size_t bytes_transferred), (override));
    MOCK_METHOD(void, handle_write, (const boost::system::error_code& error), (override));
};

#endif
