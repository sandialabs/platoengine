#ifndef EXCEPTION_HANDLING
#define EXCEPTION_HANDLING
#include <string>
#include <exception>
#include <vector>

class ParsingException : public std::exception
{
  public:
    ParsingException(std::string d);
};

class IOException : public std::exception
{
  public:
    IOException(std::string file, std::string description);
};

class RunTimeError : public std::exception
{
  public:
    RunTimeError(std::string d);
};
#endif
