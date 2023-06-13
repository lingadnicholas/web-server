#include "log.h"
#include "RequestHandlerFactory.h"

ErrorHandlerFactory :: ErrorHandlerFactory() {
    LOG_INFO << "ErrorHandlerFactory :: Constructor\n";
};

ErrorRequestHandler* ErrorHandlerFactory::create(std::string loc, NginxConfig* cfg) {
    return new ErrorRequestHandler();
}
