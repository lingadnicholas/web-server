#ifndef SERVER_H
#define SERVER_H

#include "ApiRequestHandler.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "config_parser.h"
#include <cstdlib>
#include "EchoRequestHandler.h"
#include "ErrorRequestHandler.h"
#include "HealthRequestHandler.h"
#include <iostream>
#include <map>
#include "session.h"
#include "StaticRequestHandler.h"
#include <string>
#include "RequestHandlerFactory.h"

class server {
    public:
        server(boost::asio::io_service& io_service, short port,
            std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap);
        ~server();
        bool start_accept();
        void run();

    private:
        boost::asio::io_service& io_service_;
        boost::asio::ip::tcp::acceptor acceptor_;

        // hdlrMap_ maps from location (string) to a pair containing:
        // (1) the string handlerType (e.g. "StaticHandler") and 
        // (2) the NginxConfig with the info (e.g. root directory path) for that handler
        std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap_;

        // routes_ maps from location (string) to a pointer to an appropriate RequestHandlerFactory
        std::map<std::string, RequestHandlerFactory*> routes_;

        void handle_accept(session* new_session,
            const boost::system::error_code& error);
        RequestHandlerFactory* createHandlerFactory(const std::string &name);
};

#endif
