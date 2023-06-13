//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "config_parser.h"
#include "log.h" 
#include "server.h"
#include <utility>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler); // setup termination log on ctrl+c 
    signal(SIGTERM, signalHandler); // setup termination log on termination
  try {
    if (argc != 2) {
      LOG_FATAL << "main : usage: async_tcp_echo_server <config_file>\n";
      return 1;
    }
    LOG_INFO << "main : Server received valid number of arguments (1 config file)" << std::endl;
    
    // read config file for port number
    
    NginxConfigParser config_parser;
    NginxConfig config;
    bool success = config_parser.Parse(argv[1], &config);
    if (!success) {
      LOG_FATAL << "main : Config file not parsable\n";
      return 1;
    }
    int port = config.getListeningPort();
    LOG_INFO << "main : Using port " << port << std::endl;

    std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap;
    config.populateHdlrMap(hdlrMap);
    std::map<std::string, std::pair<std::string, NginxConfig*>>::iterator itr;
    for(itr = hdlrMap.begin(); itr != hdlrMap.end(); itr++) {
        LOG_INFO << "main : hdlrMap_: URL path: '" << itr->first << 
             "' handled by '" << itr->second.first << 
             "' with configurations '" << itr->second.second << "\n";
    }

    boost::asio::io_service io_service;
    server s(io_service, port, hdlrMap);
    s.start_accept();
    LOG_INFO << "main : Created server object, now calling server.run()" << std::endl;
    s.run();
  } 
  catch (std::exception& e) {
    LOG_ERROR << "main : Exception: " << e.what() << "\n";
  }

  return 0;
}
