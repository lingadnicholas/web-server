# bro-code
# How the source code is laid out: 
## How it works
The main function in server_main.cc creates a config parser which parses the config, populates the handler map (hdlrMap_), and starts the server. The server uses the handler map to populate its routes_ object, which maps from locations to the correct handler. For each new connection accepted, it creates a session object, which gets passed the handler map along with the routes. For each request, the session reads, uses an http_parser to convert the request into a boost::beast::http::request type, passes the request to the proper request handler (which session creates using the appropriate RequestHandlerFactory found in routes_), and returns the response to the client. 
## server_main.cc and header files
--------------------------------------------------------------


server_main.cc


--------------------------------------------------------------




The file that contains the main function. It creates a config_parser which parses the config, populates the handler map, and starts the server. 


--------------------------------------------------------------


log.h


--------------------------------------------------------------


Defines the logger macros and is implemented in ../src/log.cc. 
The log macros we use are LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_FATAL.
An example of logging: 
LOG_INFO << “This is a log info\n”
LOG_FATAL << “This is a fatal log\n”


--------------------------------------------------------------


config_parser.h 


--------------------------------------------------------------


Implements a config parser in ../src/config_parser.cc that parses a config file. By parsing the config file, it populates the handler map in populateHdlrMap(std::map<std::string, std::pair<std::string, NginxConfig *>> &hdlrMap). The map maps Request URL paths from the config to a pair containing the HandlerType (which is a string, e.g. “StaticHandler”) and the config block for that handler. 


--------------------------------------------------------------


HttpParser.h


--------------------------------------------------------------


Implements the HTTP parser used in session.cc which parses a request string in getFields(std::string request_string, http::request<http::string_body>& req), which sets the fields of the request object, and getResponse, which converts a boost response object into a string to send back to the user. 


--------------------------------------------------------------


MIME.h


--------------------------------------------------------------


Implemented in ../src/MIME.cc, has a map of content_types_ from extension to the corresponding MIME type. The public method get_content_type(std::string fileExtension) takes in a file extension and returns the appropriate content type as a string. 


--------------------------------------------------------------


MockSession.h




--------------------------------------------------------------


Implements a MockSession only for testing purposes. 


--------------------------------------------------------------


RequestHandler.h


--------------------------------------------------------------


Implements the pure virtual class RequestHandler, which all other RequestHandlers inherit from. It contains the virtual function handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res), which takes in a request and a response, and returns a response by reference. handleRequest() is implemented in each request handler. 


StaticRequestHandler.h: Implemented in src/StaticRequestHandler.cc, its handleRequest() function returns a 400 Bad Request if it is given a bad request, a 404 not found if the file cannot be found, and otherwise sets the response to the file found if it is found. 


EchoRequestHandler.h: Implemented in src/EchoRequestHandler.cc, its handleRequest() function gives back a response that echoes the body of the request back to the user. 


ErrorRequestHandler.h: Implemented in src/ErrorRequestHandler.cc, its handleRequest() function simply sets the response object to a 404 not found message. 


ApiRequestHandler.h: Implemented in src/ApiRequestHandler.cc, its handleRequest() function handles create, read, update, and delete (CRUD) operations.


BlockRequestHandler.h: Implemented in src/BlockRequestHandler.cc, its handleRequest() function makes the thread sleep when called. Request path of [url]/sleep/2 will make it sleep for 2 seconds and not specifying a number or putting an invalid number will make it sleep for 1 second.


HealthRequestHandler.h: Implemented in src/HealthRequestHandler.cc, its handleRequest() function returns 200 OK to all requests, to indicate the server is up and responding to requests.


--------------------------------------------------------------


RequestHandlerFactory.h


--------------------------------------------------------------


This file implements the pure virtual RequestHandlerFactory, along with each handler’s factory. Each factory in this file inherits from RequestHandlerFactory. 


EchoHandlerFactory: implemented in src/EchoHandlerFactory.cc


StaticHandlerFactory: implemented in src/StaticHandlerFactory.cc


ErrorHandlerFactory: implemented in src/StaticHandlerFactory.cc 


ApiHandlerFactory: implemented in src/ApiHandlerFactory.cc


BlockHandlerFactory: implemented in src/BlockHandlerFactory.cc


HealthHandlerFactory: implemented in src/HealthHandlerFactory.cc


--------------------------------------------------------------


server.h


--------------------------------------------------------------




Defines the server class implemented in ../src/server.cc . It creates a server object, and creates new sessions as needed, and contains the createHandlerFactory() function to keep track of a map from locations from the config file to the appropriate RequestHandlerFactory. 


--------------------------------------------------------------


session.h


--------------------------------------------------------------


Defines the class session, implemented in ../src/session.cc, which is instantiated by the server class upon receiving a request. It also defines the class ConcreteSession, which is the actual implementation of session that we use (since we also use a MockSession for testing). Each ConcreteSession, in handle_read, handles the reading of requests, matches to a location, and then gets a response from the appropriate request handler, and then writes back the response to the client. 
# Building the code 
Use cmake to perform out-of-source build:
```
$ mkdir -p build
$ cd build
$ cmake ..
$ make
```
# Testing the code 
Within the build directory:

```
$ make test
```

# Unit Tests
We currently have test cases for 9 classes:
config_parser_test.cc
echo_handler_test.cc
error_handler_test.cc
MIME_test.cc
request_test.cc
response_test.cc
server_test.cc
session_test.cc
static_handler_test.cc

The convention for naming test files are:
{Source_filename}_test.cc

The files named config_{N} are config files used for unit testing (primarily for config_parser_test.cc)

the http_requests folder contains http requests for the server, and is used in our integration test

the curl_expected_res folder contains expected responses from curl requests to our server, and is used in our integration test

The files named expected_response{N} are expected responses from requests to our server using netcat, and are used in our integration test

static_hdlr_test_badconfig1 & static_hdlr_test_badconfig1 are used for unit testing our static handler within static_handler_test.cc

# Integration Test
The integration test is named integration_test.sh, and currently uses netcat and curl to give http requests to our server and compares it to the expected responses

# Running the code 

To run the server locally, first set /build as your cwd (our local files rely on the server’s working directory to be /build/):
```
$ cd build
```

Our server takes 1 argument (the config file). To run the server executable within the /build directory:
```
$ ./bin/server path/to/config
```

The path to the config we have been using to run the server locally is ../new_format_config in the root directory, so:
```
$ ./bin/server ../new_format_config 
```


# Data paths supported

The data served by our server is found in the data folder. 

http://34.82.60.235/static1/images/hehe-kitty.png

http://34.82.60.235/static1/images/bernie-meme.jpg

http://34.82.60.235/static1/images/orange-sun-small.jpg

http://34.82.60.235/static1/text/file1.txt

http://34.82.60.235/static1/text/sample.html

http://34.82.60.235/static1/files.zip

http://34.82.60.235/static1/file

# How to Add a Request Handler
We will use the 404 Handler ErrorHandler as an example

1. Add an entry to new_format_config (for local testing) and conf/cloudbuild.conf (for the cloud build)
```
port 8080; # port my server listens on

location /echo EchoHandler { # no arguments
}

location /static1 StaticHandler {
  root ../data/data_other;
}

location /static1/text StaticHandler {
  root ../data/data_text; # supports relative paths
}

location /static1/images StaticHandler {
  root ../data/data_images;
}

location /static2 StaticHandler {
  root ../data;
}

location / ErrorHandler {
}

# add your handler!
```
2. Add a new [insert-name]HandlerFactory class derived from RequestHandlerFactory by adding its [insert-name]HandlerFactory.cc file

NOTE: You do not need to add a [insert-name]HandlerFactory.h file; the header file RequestHandlerFactory.h contains the class definitions for all request handler factories!
Instead, add your new [insert-name]HandlerFactory class definition within the RequestHandlerFactory.h header file
```
class RequestHandlerFactory {
   public:
       virtual RequestHandler* create(std::string loc, NginxConfig* cfg) = 0;
};


# add your class definition
class ErrorHandlerFactory : public RequestHandlerFactory {
   public:
       ErrorHandlerFactory();
       ErrorRequestHandler* create(std::string loc, NginxConfig* cfg);
};
```
```
// include RequestHandlerFactory and log
#include "RequestHandlerFactory.h"
#include "log.h"


ErrorHandlerFactory :: ErrorHandlerFactory() {
   // log lets us know when a handler is being used
   LOG_INFO << "In ErrorHandlerFactory constructor\n";
};


// this function constructs a ErrorRequestHandler
ErrorRequestHandler* ErrorHandlerFactory::create(std::string loc, NginxConfig* cfg) {
   return new ErrorRequestHandler();
}
```

3. Add [insert-name]RequestHandler.h and [insert-name]RequestHandler.cc files, copying for example, StaticRequestHandler.h and .cc to see how you can use the constructor to parse the config as needed.
```
// include the header for your handler, RequestHandler, and log
#include "ErrorRequestHandler.h"
#include "RequestHandler.h"
#include "log.h"


ErrorRequestHandler :: ErrorRequestHandler()
   : RequestHandler() {
   // log lets us know when a handler is being used
   LOG_INFO << "In ErrorRequestHandler constructor\n";
}


// returns a http status code (eg. 200 for OK, 400 for bad)
int ErrorRequestHandler :: handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res) {
	// unique implementation of handleRequest here 
	// given a request, formulate a response
	// the response is passed by reference
}
```
4. In src/server.cc ‘s createHandlerFactory(const std::string &name) function, add a case for that handler based on the string token that will be used to identify it in the config (step 1, e.g. “ErrorHandler”). 

```
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
   // add your handler factory!
   return nullptr;
}
```

5. Add test cases for your handler to tests/
```
// include gtest, your header file, and any other libraries you may find useful
#include "gtest/gtest.h"
#include "ErrorRequestHandler.h"


class ErrorHandlerTest : public ::testing::Test {
protected:
	// add functions that you may find helpful 
};


TEST_F(ErrorHandlerTest, ValidErrorRequestWithBasePath) {
	// test body here
}
```

6. In CMakeLists.txt, make the following changes: (there are a lot of changes, the easiest way to check you did them all is to see where static_handler_factory / static_request, etc. appears and copy that type of process for your handler).

6.1) Add library for handler: add_library(myname_request_lib src/MynameRequestHandler.cc)
```
# Update name and srcs
add_library(error_request_lib src/ErrorRequestHandler.cc)
add_library(error_handler_factory_lib src/ErrorHandlerFactory.cc)
```
6.2) Add library for factory: add_library(myname_handler_factory_lib src/MynameHandlerFactory.cc)

6.3) Link the new request_libs to the handler_lib by adding them in this line: target_link_libraries(handler_lib echo_request_lib static_request_lib error_request_lib myname_request_lib)
```
# Update executable name, srcs, and deps
add_executable(server src/server_main.cc)
# link handler_lib to your new handler
target_link_libraries(handler_lib echo_request_lib static_request_lib error_request_lib)
# link your handler to handler_lib
target_link_libraries(error_request_lib handler_lib)
# link server to your handler
target_link_libraries(server server_lib session_lib Boost::system pthread config_parser_lib echo_request_lib static_request_lib error_request_lib echo_handler_factory_lib static_handler_factory_lib error_handler_factory_lib log_lib Boost::regex Boost::log_setup Boost::log)
```

6.4) Add this line: target_link_libraries(myname_request_lib handler_lib) to link >>
```
# Add config parser tests
# link your server_test to your handler and factory
add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_lib session_lib echo_request_lib static_request_lib error_request_lib echo_handler_factory_lib static_handler_factory_lib error_handler_factory_lib config_parser_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)
# link session to your handler
add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test session_lib echo_request_lib static_request_lib error_request_lib config_parser_lib gtest_main gmock_main log_lib Boost::regex Boost::log_setup Boost::log)
# add the executable for your test file and link it to your handler, gtest, and the following logging libraries
add_executable(error_handler_test tests/error_handler_test.cc)
target_link_libraries(error_handler_test error_request_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)
```
6.5) add gtest_discover_tests
```
# add your test cases to gtest_discover_tests
gtest_discover_tests(error_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
```
6.6) add to code coverage 
```
# generate_coverage_report(TARGETS example_server example_lib TESTS example_lib_test)
include(cmake/CodeCoverageReportConfig.cmake)
# add your handler library and handler test cases to generate_coverage_report
generate_coverage_report(TARGETS config_parser_lib server_lib session_lib server handler_lib echo_request_lib static_request_lib error_request_lib mime_lib TESTS config_parser_test server_test session_test echo_handler_test static_handler_test error_handler_test mime_test)
```
