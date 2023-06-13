#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include "log.h"
#include "constants.h"

namespace beast = boost::beast;
namespace http = beast::http;

class RequestHandler {
    public:
        RequestHandler();
        virtual int handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res) = 0;
};

#endif
