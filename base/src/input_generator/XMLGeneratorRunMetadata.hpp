/*
 * XMLGeneratorRun.hpp
 *
 *  Created on: Aug 24, 2021
 */

#pragma once

#include <vector>
#include <string>
#include <map>

#include "Plato_FreeFunctions.hpp"
#include "XMLG_Macros.hpp"

namespace XMLGen
{


/******************************************************************************//**
 * \struct Material
 * \brief Material metadata for Plato problems.
**********************************************************************************/
struct Run
{

private:
    std::unordered_map<std::string, std::string> mMetaData;
    std::string mID; 

public:
    /******************************************************************************//**
     * \fn id
     * \brief Return run identification number.
     * \return identification number
    **********************************************************************************/
    std::string id() const
    {
        return mID;
    }

    /******************************************************************************//**
     * \fn id
     * \brief Set run identification number.
     * \param [in] aID identification number
    **********************************************************************************/
    void id(const std::string& aID)
    {
        mID = aID;
    }

    /******************************************************************************//**
     * \fn value
     * \brief Return value for property with input tag; else, throw an error if \n
     * property is not defined in the metadata.
     * \param [in] aTag property tag
     * \return property string value
     **********************************************************************************/
    std::string value(const std::string& aTag) const;

    /******************************************************************************//**
     * \fn append
     * \brief Append parameter to metadata.
     * \param [in] aTag   parameter tag
     * \param [in] aValue parameter value
     **********************************************************************************/
    void append(const std::string& aTag, const std::string& aValue);

    /******************************************************************************//**
     * \fn command
     * \brief Return the command string.
     **********************************************************************************/
    std::string command() const
    {
        return value("command");
    }

    /******************************************************************************//**
     * \fn type
     * \brief Return the type string.
     **********************************************************************************/
    std::string type() const
    {
        return value("type");
    }

    /******************************************************************************//**
     * \fn criterion
     * \brief Return the criterion string.
     **********************************************************************************/
    std::string criterion() const
    {
        return value("criterion");
    }

    /******************************************************************************//**
     * \fn service
     * \brief Return the service string.
     **********************************************************************************/
    std::string service() const
    {
        return value("service");
    }
};
// struct Run

}
// namespace XMLGen
