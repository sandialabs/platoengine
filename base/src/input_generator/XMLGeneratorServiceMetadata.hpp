/*
 * XMLGeneratorServiceMetadata.hpp
 *
 *  Created on: Jul 22, 2020
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace XMLGen
{

/******************************************************************************//**
 * \struct Service
 * \brief Service metadata for Plato problems.
**********************************************************************************/
struct Service
{
public:

// private member data
private:
    std::unordered_map<std::string, std::string> mMetaData; /*!< Service metadata, map< tag, value > */
    std::vector<std::string> mDeviceIDs;

// private member functions
private:
    /******************************************************************************//**
     * \fn getValue
     * \brief Return string value for property with input tag; else, throw an error if \n
     * property is not defined in the metadata.
     * \param [in] aTag property tag
     * \return property string value
    **********************************************************************************/
    std::string getValue(const std::string& aTag) const;

    /******************************************************************************//**
     * \fn getBool
     * \brief Return bool value for property with input tag; else, throw an error if \n
     * property is not defined in the metadata.
     * \param [in] aTag property tag
     * \return property bool value
    **********************************************************************************/
    bool getBool(const std::string& aTag) const;

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return value for property with input tag; else, throw an error if \n
     * property is not defined in the metadata.
     * \param [in] aTag property tag
     * \return property string value
    **********************************************************************************/
    std::string value(const std::string& aTag) const;

    /******************************************************************************//**
     * \fn tags
     * \brief Return list of parameter tags.
     * \return parameter tags
    **********************************************************************************/
    std::vector<std::string> tags() const;

    /******************************************************************************//**
     * \fn append
     * \brief Append parameter to metadata.
     * \param [in] aTag   parameter tag
     * \param [in] aValue parameter value
    **********************************************************************************/
    void append(const std::string& aTag, const std::string& aValue);

    /******************************************************************************//**
     * \fn type
     * \brief Set 'type' keyword. Supported options: 'web_app', 'plato_app', and 'system_call'.
     * \param [in] aInput string value
    **********************************************************************************/
    void type(const std::string& aInput);

    /******************************************************************************//**
     * \fn type
     * \brief Return value for 'type' keyword.
     * \return string
    **********************************************************************************/
    std::string type() const;

    /******************************************************************************//**
     * \fn id
     * \brief Set string value for keyword 'id'.
     * \param [in] aInput string value
    **********************************************************************************/
    void id(const std::string& aInput);

    /******************************************************************************//**
     * \fn id
     * \brief Return string value for keyword 'id'.
     * \return value
    **********************************************************************************/
    std::string id() const;

    /******************************************************************************//**
     * \fn code
     * \brief Set string value for keyword 'code'.
     * \param [in] aInput string value
    **********************************************************************************/
    void code(const std::string& aInput);

    /******************************************************************************//**
     * \fn code
     * \brief Return string value for keyword 'code'.
     * \return value
    **********************************************************************************/
    std::string code() const;

    /******************************************************************************//**
     * \fn path
     * \brief Set string value for keyword 'path'.
     * \param [in] aInput string value
    **********************************************************************************/
    void path(const std::string& aInput);

    /******************************************************************************//**
     * \fn path
     * \brief Return string value for keyword 'path'.
     * \return value
    **********************************************************************************/
    std::string path() const;

    /******************************************************************************//**
     * \fn numberRanks
     * \brief Set string value for keyword 'number_ranks'.
     * \param [in] aInput string value
    **********************************************************************************/
    void numberRanks(const std::string& aInput);

    /******************************************************************************//**
     * \fn numberRanks
     * \brief Return string value for keyword 'number_ranks'.
     * \return value
    **********************************************************************************/
    std::string numberRanks() const;

    /******************************************************************************//**
     * \fn deviceIDs
     * \brief Set the vector of device ids
     * \param [in] aInput string value
    **********************************************************************************/
    void deviceIDs(const std::vector<std::string>& aInput);

    /******************************************************************************//**
     * \fn deviceIDs
     * \brief Return list of device ids
     * \return value
    **********************************************************************************/
    std::vector<std::string> deviceIDs() const;

    /******************************************************************************//**
     * \fn numberProcessors
     * \brief Set string value for keyword 'number_processors'.
     * \param [in] aInput string value
    **********************************************************************************/
    void numberProcessors(const std::string& aInput);

    /******************************************************************************//**
     * \fn numberProcessors
     * \brief Return string value for keyword 'number_processors'.
     * \return value
    **********************************************************************************/
    std::string numberProcessors() const;

    /******************************************************************************//**
     * \fn dimensions
     * \brief Set string value for keyword 'dimensions'.
     * \param [in] aInput string value
    **********************************************************************************/
    void dimensions(const std::string& aInput);

    /******************************************************************************//**
     * \fn dimensions
     * \brief Return string value for keyword 'dimensions'.
     * \return value
    **********************************************************************************/
    std::string dimensions() const;

    /******************************************************************************//**
     * \fn performer
     * \brief Return derived string value for performer.
     * \return value
    **********************************************************************************/
    std::string performer() const;

    /******************************************************************************//**
     * \fn cacheState
     * \brief Set string value for keyword 'cache_state'.
     * \param [in] aInput string value
    **********************************************************************************/
    void cacheState(const std::string& aInput);

    /******************************************************************************//**
     * \fn cacheState
     * \brief Return bool value for keyword 'cache_state'.
     * \return output bool
    **********************************************************************************/
    bool cacheState() const;

    /******************************************************************************//**
     * \fn updateProblem
     * \brief Set string value for keyword 'update_problem'.
     * \param [in] aInput string value
    **********************************************************************************/
    void updateProblem(const std::string& aInput);

    /******************************************************************************//**
     * \fn updateProblem
     * \brief Return bool value for keyword 'update_problem'.
     * \return output bool
    **********************************************************************************/
    bool updateProblem() const;

    /******************************************************************************//**
     * \fn existingInputDeck
     * \brief Set string value for keyword 'existing_input_deck'.
     * \param [in] aInput string value
    **********************************************************************************/
    void existingInputDeck(const std::string& aInput);

    /******************************************************************************//**
     * \fn existingInputDeck
     * \brief Return string value for keyword 'existing_input_deck'.
     * \return output string: non-empty if using an existing input deck
    **********************************************************************************/
    std::string existingInputDeck() const;

};
// struct Service

}
