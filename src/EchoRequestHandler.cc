#include <boost/lexical_cast.hpp>
#include "EchoRequestHandler.h"

EchoRequestHandler :: EchoRequestHandler() 
    : RequestHandler() {
    LOG_INFO << "EchoRequestHandler :: Constructor\n";
}

int EchoRequestHandler :: handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res) {
    if (req.method_string()=="") {
        res.result(HTTP_STATUS_BAD_REQUEST);
        res.reason(HTTP_REASON_BAD_REQUEST);
        return HTTP_STATUS_BAD_REQUEST;
    }

    res.version(req.version());
    res.result(HTTP_STATUS_OK);
    res.reason(HTTP_REASON_OK);
    res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);

    std::string body;
    std::string const strHeaders = boost::lexical_cast<std::string>(req.base());
    body += strHeaders;
    body += req.body();
    res.body() = body;

    res.set(http::field::content_length, std::to_string(res.body().size()));

    return HTTP_STATUS_OK;
}
