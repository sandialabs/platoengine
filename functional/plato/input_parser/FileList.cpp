#include "plato/input_parser/FileList.hpp"

namespace plato::input_parser
{
FileList::const_iterator FileList::begin() const { return mList.begin(); }

FileList::const_iterator FileList::end() const { return mList.end(); }

FileList::iterator FileList::begin() { return mList.begin(); }

FileList::iterator FileList::end() { return mList.end(); }

void FileList::insert(const std::vector<std::string>::iterator aIter, std::string aVal)
{
    mList.insert(aIter, std::move(aVal));
}

std::ostream& operator<<(std::ostream& stream, const FileList& aFileList)
{
    for (const std::string& tFileName : aFileList.mList)
    {
        stream << tFileName << ", ";
    }
    return stream;
}

}  // namespace plato::input_parser
