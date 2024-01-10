#include "Plato_SuppressBoostNvccWarnings.hpp"

#include "Plato_FileList.hpp"

namespace Plato
{

std::string::const_iterator FileName::begin() const 
{
    return mName.begin();
}

std::string::const_iterator FileName::end() const
{
    return mName.end();
}

std::string::iterator FileName::begin()
{
    return mName.begin();
}

std::string::iterator FileName::end()
{
    return mName.end();
}

void FileName::insert(const std::string::iterator aIter, const char aVal)
{
    mName.insert(aIter, aVal);
}

std::ostream& operator<<(std::ostream& stream, const Plato::FileName& aFileName)
{
    stream << aFileName.mName << ", ";
    return stream;
}

FileList::const_iterator FileList::begin() const
{
    return mList.begin();
}

FileList::const_iterator FileList::end() const
{
    return mList.end();
}

FileList::iterator FileList::begin()
{
    return mList.begin();
}

FileList::iterator FileList::end()
{
    return mList.end();
}

void FileList::insert(const std::vector<std::string>::iterator aIter, std::string aVal)
{
    mList.insert(aIter, std::move(aVal));
}

std::ostream& operator<<(std::ostream& stream, const Plato::FileList& aFileList)
{
    for(const std::string& tFileName : aFileList.mList)
    {
        stream << tFileName << ", ";
    }
    return stream;
}

}

#include "Plato_RestoreBoostNvccWarnings.hpp"
