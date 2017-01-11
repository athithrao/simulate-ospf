//
// Created by Athith Amarnath on 12/4/16.
//

#ifndef OSPF_LOGGER_H
#define OSPF_LOGGER_H

#include <fstream>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
//https://github.com/SFML/SFML/wiki/Source:-Simple-File-Logger-(by-mateandmetal)

class FileLogger {

public:
    // If you can´t/dont-want-to use C++11, remove the "class" word after enum
    enum class e_logType { LOG_ERROR, LOG_WARNING, LOG_INFO };


    // ctor (remove parameters if you don´t need them)
    explicit FileLogger (const int log_type, const char *engine_version, const char *fname);


    // dtor
    ~FileLogger ();


    friend FileLogger &operator << (FileLogger &logger, const e_logType l_type) {

        struct timeval tv;
        struct tm* ptm;
        char time_string[40];
        long milliseconds;

        char logdata[100];
        //http://www.makelinux.net/alp/063
        gettimeofday (&tv, NULL);
        ptm = localtime (&tv.tv_sec);
        /* Format the date and time, down to a single second. */
        strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
        /* Compute milliseconds from microseconds. */
        milliseconds = tv.tv_usec / 1000;
        /* Print the formatted time, in seconds, followed by a decimal point
        and the milliseconds. */


        switch (l_type) {
            case FileLogger::e_logType::LOG_ERROR:
                sprintf (logdata,"[%s.%03ld][ERROR]: ", time_string, milliseconds);
                logger.myFile << logdata;
                ++logger.numErrors;
                break;

            case FileLogger::e_logType::LOG_WARNING:
                sprintf (logdata,"[%s.%03ld][WARNING]: ", time_string, milliseconds);
                logger.myFile << logdata;
                ++logger.numWarnings;
                break;

            case FileLogger::e_logType::LOG_INFO:
                sprintf (logdata,"[%s.%03ld][INFO]: ", time_string, milliseconds);
                logger.myFile << logdata;
                break;

            default:
                break;
        } // sw


        return logger;

    }


    // Overload << operator using C style strings
    // No need for std::string objects here
    friend FileLogger &operator << (FileLogger &logger, const char *text) {

        logger.myFile << text << std::endl;
        return logger;

    }

    // Make it Non Copyable (or you can inherit from sf::NonCopyable if you want)
    FileLogger (const FileLogger &) = delete;
    FileLogger &operator= (const FileLogger &) = delete;


private:

    std::ofstream           myFile;

    unsigned int            numWarnings;
    unsigned int            numErrors;

}; // class end


#endif //OSPF_LOGGER_H
