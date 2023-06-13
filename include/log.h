#ifndef LOG_H 
#define LOG_H  

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/trivial.hpp> 

// Global logger code obtained from https://gernotklingler.com/blog/simple-customized-logger-based-boost-log-v2/ 

// just log messages with severity >= SEVERITY_THRESHOLD are written (info)
#define SEVERITY_THRESHOLD logging::trivial::info   

// register a global logger
BOOST_LOG_GLOBAL_LOGGER(logger, boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>)

// just a helper macro used by the macros below - don't use it in your code
#define LOG(severity) BOOST_LOG_SEV(logger::get(),boost::log::trivial::severity)

// ===== log macros =====
#define LOG_TRACE   LOG(trace)
#define LOG_DEBUG   LOG(debug)
#define LOG_INFO    LOG(info)
#define LOG_WARNING LOG(warning)
#define LOG_ERROR   LOG(error)
#define LOG_FATAL   LOG(fatal)

// signalHandler to catch ctrl+c, termination of server
void signalHandler(int); 
#endif