//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include "config_parser.h"
#include <cstdlib>
#include "EchoRequestHandler.h"
#include "ErrorRequestHandler.h"
#include "HealthRequestHandler.h"
#include "HttpParser.h"
#include <iostream>
#include "RequestHandlerFactory.h"
#include "StaticRequestHandler.h"
namespace beast = boost::beast;
namespace http = beast::http;

class session {
    public:
        session(boost::asio::io_service& io_service, 
                std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap,
                std::map<std::string, RequestHandlerFactory*> routes);

        virtual boost::asio::ip::tcp::socket& socket() = 0;

        virtual void start() = 0;

    protected:
        boost::asio::ip::tcp::socket socket_;
        std::string client_ip_;
        enum { max_length = 8192 };
        char data_[max_length];

        // hdlrMap_ maps from location (string) to a pair containing:
        // (1) the string handlerType (e.g. "StaticHandler") and
        // (2) the NginxConfig with the info (e.g. root directory path) for that handler
        std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap_;

        // routes_ maps from location (string) to a pointer to an appropriate RequestHandlerFactory
        std::map<std::string, RequestHandlerFactory*> routes_;

        HttpParser http_parser;

        virtual void handle_read(const boost::system::error_code& error,
            size_t bytes_transferred) = 0;

        virtual void handle_write(const boost::system::error_code& error) = 0;
};

class ConcreteSession : public session {
    public:
        ConcreteSession(boost::asio::io_service& io_service, 
                std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap,
                std::map<std::string, RequestHandlerFactory*> routes);

        boost::asio::ip::tcp::socket& socket();

        void start();

    private:
        void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred);

        void handle_write(const boost::system::error_code& error);

        /* Function: match
         * Parameter: std::string requestURI
         * Returns: std::string longest matching location (per routes, per config)
         * 
         * Explanation:
         * Finds the longest matching location in routes for a given requestURI
         * For example, if routes has keys (locations): "/static", "/static/data/text"
         * Then for requestURI="/static/data", the longest matching location
         * would be /static
         */
        std::string match(std::string requestURI);

};

#endif
