#include "gtest/gtest.h"
#include "server.h"
#include <string>
#include <map>
#include "config_parser.h"

class ServerTest : public ::testing::Test {
    protected:
        short port = 8080;
        boost::asio::io_service io_service;
        // We are using the real implementation of a session.
        // Using a mock is not desirable as the attempted
        // refactoring of the code expanded it too much.
        NginxConfigParser config_parser;
        NginxConfig config;
        std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap;
        ServerTest() {
            bool success = config_parser.Parse("new_format_config_for_testing", &config);
            config.populateHdlrMap(hdlrMap);
        }
};

// Running start_accept should successfully create a new session and start it
TEST_F(ServerTest, StartAcceptSuccess) {
    server s(io_service, port, hdlrMap);
    bool success = s.start_accept();
    EXPECT_TRUE(success);
}
