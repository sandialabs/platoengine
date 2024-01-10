/*
 * XMLGeneratorServiceMetadata.cpp
 *
 *  Created on: Jul 22, 2020
 */

#include "XMLG_Macros.hpp"
#include "Plato_FreeFunctions.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorServiceMetadata.hpp"

namespace XMLGen
{

std::string Service::getValue(const std::string& aTag) const
{
    auto tItr = mMetaData.find(aTag);
    if(tItr == mMetaData.end())
    {
        return "";
    }
    return tItr->second;
}

bool Service::getBool(const std::string& aTag) const
{
    auto tItr = mMetaData.find(aTag);
    if(tItr == mMetaData.end())
    {
        THROWERR(std::string("XML Generator Service Metadata: '") + aTag + "' keyword is not defined.")
    }
    return (XMLGen::transform_boolean_key(tItr->second));
}

std::string Service::value(const std::string& aTag) const
{
    auto tTag = Plato::tolower(aTag);
    auto tItr = mMetaData.find(tTag);
    if(tItr == mMetaData.end())
    {
        THROWERR(std::string("XML Generator Service Metadata: Parameter with tag '") + aTag + "' is not defined in metadata.")
    }
    return (tItr->second);
}

std::vector<std::string> Service::tags() const
{
    std::vector<std::string> tTags;
    for(auto& tProperty : mMetaData)
    {
        tTags.push_back(tProperty.first);
    }
    return tTags;
}

void Service::append(const std::string& aTag, const std::string& aValue)
{
    if (aTag.empty())
    {
        THROWERR(std::string("XML Generator Service Metadata: Input tag '") + aTag + "' is empty.")
    }
    auto tTag = Plato::tolower(aTag);
    mMetaData[aTag] = aValue;
}

void Service::type(const std::string& aInput)
{
    mMetaData["type"] = aInput;
}

std::string Service::type() const
{
    return (this->getValue("type"));
}

void Service::id(const std::string& aInput)
{
    mMetaData["id"] = aInput;
}

std::string Service::id() const
{
    return (this->getValue("id"));
}

void Service::code(const std::string& aInput)
{
    mMetaData["code"] = aInput;
}

std::string Service::code() const
{
    return (this->getValue("code"));
}

void Service::path(const std::string& aInput)
{
    mMetaData["path"] = aInput;
}

std::string Service::path() const
{
    return (this->getValue("path"));
}

void Service::deviceIDs(const std::vector<std::string>& aInput)
{
    mDeviceIDs = aInput;
}

std::vector<std::string> Service::deviceIDs() const
{
    return mDeviceIDs;
}

void Service::numberRanks(const std::string& aInput)
{
    mMetaData["number_ranks"] = aInput;
}

std::string Service::numberRanks() const
{
    return (this->getValue("number_ranks"));
}

void Service::numberProcessors(const std::string& aInput)
{
    mMetaData["number_processors"] = aInput;
}

std::string Service::numberProcessors() const
{
    return (this->getValue("number_processors"));
}

void Service::dimensions(const std::string& aInput)
{
    mMetaData["dimensions"] = aInput;
}

std::string Service::dimensions() const
{
    return (this->getValue("dimensions"));
}

std::string Service::performer() const
{
    return (code() + "_" + id());
}

void Service::cacheState(const std::string& aInput)
{
    mMetaData["cache_state"] = aInput;
}

bool Service::cacheState() const
{
    return (this->getBool("cache_state"));
}

void Service::updateProblem(const std::string& aInput)
{
    mMetaData["update_problem"] = aInput;
}

bool Service::updateProblem() const
{
    return (this->getBool("update_problem"));
}

void Service::existingInputDeck(const std::string& aInput) {
    mMetaData["existing_input_deck"] = aInput;
}

std::string Service::existingInputDeck() const {
    return (this->getValue("existing_input_deck"));
}


}
// namespace XMLGen
