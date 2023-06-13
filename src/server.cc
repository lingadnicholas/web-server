// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#include "log.h"
#include "server.h"

using boost::asio::ip::tcp;

server :: server(boost::asio::io_service& io_service, short port,
		 std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap)
        : io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
        hdlrMap_(hdlrMap) {
    LOG_INFO << "In server constructor\n";
    std::map<std::string, std::pair<std::string, NginxConfig*>>::iterator it;
    for(it = hdlrMap_.begin(); it != hdlrMap_.end(); it++) {
        std::string handlerType = it->second.first;
        LOG_INFO << "server :: constructor : adding routes_ mapping for location = '" 
            << it->first << "' to '" << handlerType << "' Factory pointer\n";
        
        routes_[it->first] = createHandlerFactory(handlerType);
        
        if (routes_[it->first] == nullptr) {
            LOG_WARNING << "server :: constructor : createHandlerFactory unsuccessful, returned nullptr\n";
        }
        else {
            LOG_INFO << "server :: constructor : createHandlerFactory successful\n";
        }
    }
}

server :: ~server() {
    std::map<std::string, RequestHandlerFactory *>::iterator it;
    for(it = routes_.begin(); it != routes_.end(); it++) {
        if (it->second != nullptr) {
            delete it->second;
        }
    }
}

bool server :: start_accept() {
    LOG_INFO << "server :: start_accept()" << std::endl;
    session* new_session = new ConcreteSession(io_service_, hdlrMap_, routes_);
    acceptor_.async_accept(new_session->socket(),
    boost::bind(&server::handle_accept, this, new_session,
    boost::asio::placeholders::error));
    if (new_session == nullptr) {
        return false;
    }
    else {
        return true;
    }
}

// Multithreading 
void server::run() {
  const int NUM_THREADS = 4; 
  boost::asio::thread_pool tpool(NUM_THREADS);
  for (int i = 0; i < NUM_THREADS; ++i) {
      boost::asio::post(tpool, boost::bind(&boost::asio::io_service::run, &io_service_));
  }
  tpool.join();
}

void server :: handle_accept(session* new_session,
    const boost::system::error_code& error) {
    LOG_INFO << "server :: handle_accept()" << std::endl;
    if (!error) {
        LOG_INFO << "Starting new session" << std::endl; // debug msg
        new_session->start();
    }
    else {
        delete new_session;
    }
    start_accept();
}

RequestHandlerFactory* server :: createHandlerFactory(const std::string &name) {
    if (name == "EchoHandler") {
        return new EchoHandlerFactory();
    }
    if (name == "StaticHandler") {
        return new StaticHandlerFactory();
    }
    if (name == "ErrorHandler") {
        return new ErrorHandlerFactory();
    }
    if (name == "ApiHandler") {
        return new ApiHandlerFactory();
    }
    if (name == "BlockHandler") {
        return new BlockHandlerFactory(); 
    }
    if (name == "HealthHandler") {
        return new HealthHandlerFactory(); 
    }
    if (name == "BoardHandler") {
        return new BoardHandlerFactory();
    }
    return nullptr;
}
