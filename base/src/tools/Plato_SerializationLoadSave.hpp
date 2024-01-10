#pragma once

#include "Plato_NamedType.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <fstream>

namespace Plato{
/// Strongly-typed string for holding an XML file name
using XMLFileName = Utils::NamedType<std::string, struct XMLFileNameTag>;
/// Strongly-typed string for holding an XML node name
using XMLNodeName = Utils::NamedType<std::string, struct XMLNodeNameTag>;

template<typename T>
void saveToXML(const T& aT, const XMLFileName& aFileName, const XMLNodeName& aNodeName)
{
    std::ofstream tOutFileStream(aFileName.mValue.c_str());
    boost::archive::xml_oarchive tOutputArchive(tOutFileStream, boost::archive::no_header | boost::archive::no_tracking);
    tOutputArchive << boost::serialization::make_nvp(aNodeName.mValue.c_str(), aT); 
}

template<typename T>
void loadFromXML(T& aT, const XMLFileName& aFileName, const XMLNodeName& aNodeName)
{
    std::ifstream tInFileStream(aFileName.mValue.c_str());
    constexpr int kNoHeader = 1;
    boost::archive::xml_iarchive tInputArchive(tInFileStream, kNoHeader);
    tInputArchive >> boost::serialization::make_nvp(aNodeName.mValue.c_str(), aT);
}
}
