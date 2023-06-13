#include "RequestHandlerFactory.h"

ApiHandlerFactory :: ApiHandlerFactory() {
    LOG_INFO << "ApiHandlerFactory :: Constructor\n";
};

ApiRequestHandler* ApiHandlerFactory::create(std::string loc, NginxConfig* cfg) {
    return new ApiRequestHandler(loc, cfg, entity_ids);
}
