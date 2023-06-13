#include "log.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <string.h>

namespace logging = boost::log;


BOOST_LOG_GLOBAL_LOGGER_INIT(logger, logging::sources::severity_logger_mt<logging::trivial::severity_level>) {
    logging::sources::severity_logger_mt<logging::trivial::severity_level> logger;
	
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");
    logging::add_common_attributes();

    // New log file should be obtained when a day's log file reaches 10MB and when it reaches midnight
    logging::add_file_log(
        logging::keywords::file_name = "../log/SERVER_LOG_%N.log",
        logging::keywords::rotation_size = 10 * 1024 * 1024, // Create new file when reaches 10 MB
        logging::keywords::time_based_rotation = logging::sinks::file::rotation_at_time_point(0, 0, 0), // Create new file at midnight
        logging::keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%",
        logging::keywords::auto_flush = true
    );

    logging::add_console_log(
        std::cerr,
        logging::keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%"
    );

    // just log messages with severity >= SEVERITY_THRESHOLD are written
    logging::core::get()->set_filter(
        logging::trivial::severity >= SEVERITY_THRESHOLD
    );

    return logger;
}

// https://www.tutorialspoint.com/cplusplus/cpp_signal_handling.htm
void signalHandler(int signum) {
    LOG_INFO << "Server termination by signal " << strsignal(signum) << std::endl;
    exit(signum);
}
