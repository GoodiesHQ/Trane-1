#ifndef TRANE_LOGGING_HPP
#define TRANE_LOGGING_HPP

#include <sstream>
#include <iostream>

#ifdef ERROR
#undef ERROR
#endif
enum LogLevel
{
    ERROR,
    SUCCESS,
    WARNING,
    INFO,
    DEBUG,
    VERBOSE,
};

constexpr const char * log_level_name(LogLevel level)
{
    switch(level)
    {
        case ERROR:
            return "ERROR    ";
        case SUCCESS:
            return "SUCCESS  ";
        case WARNING:
            return "WARNING  ";
        case INFO:
            return "INFO     ";
        case DEBUG:
            return "DEBUG    ";
        case VERBOSE:
            return "VERBOSE  ";
    }
    return "";
}

class Logger
{
public:
    Logger(LogLevel level = INFO)
    {
        m_buf << log_level_name(level);
    }

    template<typename T>
    Logger& operator<<(const T& value)
    {
        m_buf << value;
        return *this;
    }

    ~Logger()
    {
        m_buf << '\n';
        std::cerr << m_buf.str();
    }

protected:
    std::ostringstream m_buf;
};

extern LogLevel LOGLEVEL;

#define LOG(level) \
    if (level > LOGLEVEL) {} \
    else Logger(level) << __FILE__ << ':' << std::dec << __LINE__ << " - " << __FUNCTION__ << ": "

#endif
