#ifndef PLATO_GEOMETRY_EXTENSION_CUBIT_BRACEDOUTPUT
#define PLATO_GEOMETRY_EXTENSION_CUBIT_BRACEDOUTPUT

namespace plato::geometry::extension::cubit
{

///@brief A wrapper struct that enables the output << operator and adds curly braces around the value.
///@note This is primarily to facilitate simplified aprepro variable output in a journal file.
template <typename T>
struct BracedOutput
{
    const T& mValue;
};

///@brief Output operator that takes a stream @a aStream and a braced output type @a aOutput and adds curly braces
/// around the value and inserts it in the stream
template <typename Stream, typename T>
auto operator<<(Stream& aStream, const BracedOutput<T>& aOutput) -> Stream&
{
    aStream << "{" << aOutput.mValue << "}";
    return aStream;
}

}  // namespace plato::geometry::extension::cubit

#endif
