#include "RequestHandlerFactory.h"

BlockHandlerFactory :: BlockHandlerFactory() {
   LOG_INFO << "BlockHandlerFactory :: Constructor\n";
};

// this function constructs a ErrorRequestHandler
BlockRequestHandler* BlockHandlerFactory::create(std::string loc, NginxConfig* cfg) {
   return new BlockRequestHandler();
}