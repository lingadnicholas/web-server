//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <bits/stdc++.h> // needed to convert char array to string
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "constants.h"
#include <cstdlib>
#include <cstring> // needed for strcpy
#include "EchoRequestHandler.h"
#include "ErrorRequestHandler.h"
#include "HealthRequestHandler.h"
#include <iostream>
#include <log.h>
#include "session.h"
#include "StaticRequestHandler.h"
#include <string.h> // needed for memset
#include <vector>

using boost::asio::ip::tcp;

session :: session(boost::asio::io_service& io_service,
                   std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap,
                   std::map<std::string, RequestHandlerFactory*> routes)
        : socket_(io_service) {
    hdlrMap_ = hdlrMap;
    routes_ = routes;
}

ConcreteSession :: ConcreteSession(boost::asio::io_service& io_service,
                   std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap,
                   std::map<std::string, RequestHandlerFactory*> routes)
   : session(io_service, hdlrMap, routes) {
}

tcp::socket& ConcreteSession :: socket() {
    return socket_;
}

void ConcreteSession :: start() {
    client_ip_ = socket_.remote_endpoint().address().to_string();
    LOG_INFO << "ConcreteSession :: start : start at ip " << client_ip_ << std::endl;
    memset(data_, 0, sizeof data_); // reset data buffer so it can be reused for future read
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&ConcreteSession::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

// upon reading, we write a response, which is why handle_read
// function appears to have code for handling writing
void ConcreteSession :: handle_read(const boost::system::error_code& error,
      size_t bytes_transferred) {
    LOG_INFO << "ConcreteSession :: handle_read :" << std::endl;
    LOG_INFO << "Bytes received: " << bytes_transferred << std::endl;

    if (!error) {
        std::string message = std::string(data_);  // the message to be echoed back
        LOG_INFO << "ConcreteSession :: handle_read : from IP " << client_ip_ << ": Msg received is:\n[" << message << "]\n";
        http::request_parser<http::string_body> parser;
        
        boost::asio::const_buffer requestBuffer(message.data(), message.size());
        boost::beast::error_code ec;
        parser.put(requestBuffer, ec);

        http::request<http::string_body> req;
        http_parser.getFields(message, req);
        std::string target(req.target().begin(), req.target().end());
        std::string loc = match(target);
        LOG_INFO << "ConcreteSession :: handle_read : match function returned loc=" << loc << "\n";
        http::response<http::string_body> res;

        if (ec && ec!=http::error::bad_version) {
            LOG_INFO << "ConcreteSession :: handle_read : invalid request received: " << ec.message() << std::endl;
            res.result(HTTP_STATUS_BAD_REQUEST);
            res.reason(HTTP_REASON_BAD_REQUEST);
        }
        else {
            int status;
            if (loc != "") { // if a match was found then use the corresponding handler, if not, pass to 404 handler
                LOG_INFO << "ConcreteSession :: handle_read : found longest match=" << loc << "\n";
                RequestHandlerFactory* factory = routes_[loc];
                NginxConfig* cfg = hdlrMap_[loc].second;
                RequestHandler* handler = factory->create(loc, cfg);
                status = handler->handleRequest(req, res);
                LOG_INFO << "[ResponseMetrics] :: Handler returned Response Code [" << status << "]\n";
            }
            else { // no match found, passing to 404 handler
                LOG_INFO << "ConcreteSession :: handle_read : longest match is empty string \n";
                
                // since an empty location string cannot be properly represented within the config file,
                // if the loc is empty, we replace it with / so that it can use the 404 handler
                loc = "/"; 
                RequestHandlerFactory* factory = routes_[loc];
                NginxConfig* cfg = hdlrMap_[loc].second;
                RequestHandler* handler = factory->create(loc, cfg);
                status = handler->handleRequest(req, res);
                LOG_INFO << "[ResponseMetrics] :: Handler returned Response Code [" << status << "]\n";
            }            
        }

        memset(data_, 0, sizeof data_); // reset data buffer so it can be reused for future read
        
        std::string res_str = http_parser.getResponse(res);
        LOG_INFO << "ConcreteSession :: handle_read : Session responding to IP " << client_ip_ << " with:\n[" << res_str << "]\n";

        // copy response string into char array as required by async_write
        std::vector<char> response_(res_str.begin(), res_str.end());
        int response_num_bytes = res_str.length();
        LOG_INFO << "ConcreteSession :: handle_read : num bytes in response = " << response_num_bytes << "\n";
        // write the response to the client
        boost::asio::async_write(socket_,
            boost::asio::buffer(response_, response_num_bytes),
            boost::bind(&ConcreteSession::handle_write, this,
            boost::asio::placeholders::error));
    }
    else {
        delete this;
    }
    memset(data_, 0, sizeof data_);
}

// upon writing a response to client, we prepare to read the next
// input from client, which is why handle_write appears to have
// code for reading
void ConcreteSession :: handle_write(const boost::system::error_code& error) {
    LOG_INFO << "ConcreteSession :: handle_write" << std::endl;
    if (!error) {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&ConcreteSession::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else {
        delete this;
    }
}

std::string ConcreteSession :: match(std::string requestURI) {
    LOG_INFO << "ConcreteSession :: match : requestURI=" << requestURI << "\n";
    std::string result = "";
    int result_len = 0;
    std::map<std::string, RequestHandlerFactory*>::iterator it;
    for (it = routes_.begin(); it != routes_.end(); it++) {
        std::string loc = it->first; // the config location/URI
        if (requestURI.substr(0, loc.length()) == loc &&
           (requestURI.length() == loc.length() || // requestURI matches loc completely
            requestURI.at(loc.length()) == '/')) { // or matching portion is followed by a /
            if (loc.length() > result_len) { // if this is the longest match so far, save it
                result = loc;
                result_len = loc.length();
            }
         }
    }
    LOG_INFO << "ConcreteSession :: match : result=" << result << "\n";
    return result;
}
