#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "MockSession.h"
#include "session.h"
#include "config_parser.h"

// using ::testing::_;
using ::testing::Exactly;  

// Running start should successfully start a new session
TEST(SessionTest, StartSuccess) {
    boost::asio::io_service io_service;
    NginxConfig out_config;
    std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap;
    std::map<std::string, RequestHandlerFactory*> routes;
    MockSession m(io_service, hdlrMap, routes);
    EXPECT_CALL(m, start).Times(Exactly(1));
}
