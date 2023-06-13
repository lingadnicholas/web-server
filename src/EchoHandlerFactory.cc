#include "RequestHandlerFactory.h"

EchoHandlerFactory :: EchoHandlerFactory() {
    LOG_INFO << "EchoHandlerFactory :: Constructor\n";
};

EchoRequestHandler* EchoHandlerFactory::create(std::string loc, NginxConfig* cfg) {
    return new EchoRequestHandler();
}
