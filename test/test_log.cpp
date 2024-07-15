#include <iostream>
#include "../src/log.h"
#include "../src/util.h"
using namespace aware;
int main(int argc, char** argv) {
    Logger::ptr logger(new Logger);
    logger->addAppender( LogAppender::ptr(new StdoutLogAppender));


    /*FileLogAppender::ptr file_appender(new  FileLogAppender("./log.txt"));
    LogFormatter::ptr fmt(new  LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel( LogLevel::ERROR);

    logger->addAppender(file_appender); */
    aware_LOG_INFO(logger) << "test macro";
    aware_LOG_ERROR(logger) << "test macro error";

    //aware_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    //auto l =  LoggerMgr::getInstance()->getLogger("xx");
    //aware_LOG_INFO(l) << "xxx"; 
    return 0;
}
