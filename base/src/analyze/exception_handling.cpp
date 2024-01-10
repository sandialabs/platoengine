#include "exception_handling.hpp"
#include "communicator.hpp"

ParsingException::ParsingException(std::string description)
{
  pXcout << "Input error " << std::endl;
  pXcout << description << std::endl;
}

IOException::IOException(std::string file, std::string description)
{
  pXcout << "!!! Problem reading exodus file " << file << std::endl;
  pXcout << description << std::endl;
}

RunTimeError::RunTimeError(string d)
{
  pXcout << "Run time error :" << std::endl;
  pXcout << d << std::endl;
}
