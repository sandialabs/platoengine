/*
 * XMLGeneratorAssemblyMetadata.hpp
 *
 *  Created on: June 1, 2021
 */

#pragma once

#include <map>
#include <vector>
#include <string>

#include "Plato_FreeFunctions.hpp"
#include "XMLG_Macros.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \struct Assembly
 * \brief Assembly metadata for Plato problems.
**********************************************************************************/
struct Assembly
{
private:
    std::map<std::string, std::pair<std::string, std::string>> mProperties; /*!< list of assembly properties, map< tag, pair<attribute,value> > */
    std::vector<std::string> mOffset;

public:
    /******************************************************************************//**
     * \fn id
     * \brief Return assembly identification number.
     * \return identification number
    **********************************************************************************/
    std::string id() const
    {
        return value("id");
    }

    /******************************************************************************//**
     * \fn id
     * \brief Set assembly identification number.
     * \param [in] aID identification number
    **********************************************************************************/
    void id(const std::string& aID)
    {
        property("id", aID);
    }

    /******************************************************************************//**
     * \fn attribute
     * \brief Return assembly attribute.
     * \return attribute
    **********************************************************************************/
    std::string attribute(const std::string& aTag) const
    {
        auto tTag = Plato::tolower(aTag);
        auto tItr = mProperties.find(tTag);
        if(tItr == mProperties.end())
        {
            THROWERR(std::string("XML Generator Assembly: Assembly property '") + aTag + "' is not supported.")
        }
        return (tItr->second.first);
    }

    /******************************************************************************//**
     * \fn value
     * \brief If assembly property is defined, return its value; else, return an empty string.
     * \param [in]  aTag    assembly property tag
     * \return assembly property value
    **********************************************************************************/
    std::string value(const std::string& aTag) const
    {
        auto tTag = Plato::tolower(aTag);
        auto tItr = mProperties.find(tTag);
        auto tOutput = tItr == mProperties.end() ? "" : tItr->second.second;
        return tOutput;
    }

    /******************************************************************************//**
     * \fn child_nodeset
     * \brief child nodest for assembly interface
     * \return assembly child_nodeset
    **********************************************************************************/
    std::string child_nodeset() const
    {
        auto tItr = mProperties.find("child_nodeset");
        auto tOutput = tItr == mProperties.end() ? "" : tItr->second.second;
        return tOutput;
    }

    /******************************************************************************//**
     * \fn parent_block
     * \brief parent block in assembly
     * \return assembly parent_block
    **********************************************************************************/
    std::string parent_block() const
    {
        auto tItr = mProperties.find("parent_block");
        auto tOutput = tItr == mProperties.end() ? "" : tItr->second.second;
        return tOutput;
    }

    /******************************************************************************//**
     * \fn offset
     * \brief offset distance between part faces in assembly
     * \return assembly offset
    **********************************************************************************/
    const std::vector<std::string>& offset() const
    {
        return mOffset;
    }

    /******************************************************************************//**
     * \fn offset
     * \brief set offset values
    **********************************************************************************/
    void offset(const std::vector<std::string>& aOffset) 
    {
        mOffset = aOffset;
    }

    /******************************************************************************//**
     * \fn rhs_value
     * \brief right-hand side value for MPC
     * \return assembly rhs_value
    **********************************************************************************/
    std::string rhs_value() const
    {
        auto tItr = mProperties.find("rhs_value");
        auto tOutput = tItr == mProperties.end() ? "" : tItr->second.second;
        return tOutput;
    }

    /******************************************************************************//**
     * \fn property
     * \brief If assembly property is defined, return its value; else, throw an error.
     * \param [in] aTag assembly property tag
     * \return assembly property value
    **********************************************************************************/
    std::string property(const std::string& aTag) const
    {
        auto tTag = Plato::tolower(aTag);
        auto tItr = mProperties.find(tTag);
        if(tItr == mProperties.end())
        {
            THROWERR(std::string("XML Generator Assembly: Assembly property '") + aTag + "' is not defined.")
        }
        return (tItr->second.second);
    }

    /******************************************************************************//**
     * \fn property
     * \brief Set assembly property value.
     * \param [in] aTag       assembly property tag
     * \param [in] aValue     assembly property value
     * \param [in] aAttribute assembly attribute
    **********************************************************************************/
    void property(const std::string& aTag, const std::string& aValue, std::string aAttribute = "homogeneous")
    {
        if(aTag.empty()) { THROWERR("XML Generator Assembly: Assembly property tag is empty.") }
        if(aValue.empty()) { THROWERR("XML Generator Assembly: Assembly property value is empty.") }
        if(aAttribute.empty()) { THROWERR("XML Generator Assembly: Assembly property attribute is empty.") }
        auto tTag = Plato::tolower(aTag);
        mProperties[aTag] = std::make_pair(aAttribute, aValue);
    }

    /******************************************************************************//**
     * \fn tags
     * \brief Return list of assembly property tags.
     * \return assembly property tags
    **********************************************************************************/
    std::vector<std::string> tags() const
    {
        std::vector<std::string> tTags;
        for(auto& tProperty : mProperties)
        {
            tTags.push_back(tProperty.first);
        }
        return tTags;
    }

    /******************************************************************************//**
     * \fn empty
     * \brief Throw error if assembly is empty, i.e. assembly properties are not defined.
    **********************************************************************************/
    void empty() const
    {
        if(mProperties.empty())
        {
            THROWERR(std::string("XML Generator Assembly: Assembly with identification (id) '")
                + id() + "' is empty, i.e. " + "assembly properties are not defined.")
        }
    }
};
// struct Assembly

}
// namespace XMLGen
