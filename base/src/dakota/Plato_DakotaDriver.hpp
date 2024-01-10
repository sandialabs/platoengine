/*
//@HEADER
// *************************************************************************
//   Plato Engine v.1.0: Copyright 2018, National Technology & Engineering
//                    Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Sandia Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact the Plato team (plato3D-help@sandia.gov)
//
// *************************************************************************
//@HEADER
*/

/*
 * Plato_DakotaDriver.hpp
 *
 *  Created on: Aug 17, 2021
 */

#pragma once
#include <cfenv>

// Dakota includes
#include "ProgramOptions.hpp"
#include "LibraryEnvironment.hpp"

// Plato includes
#include "Plato_Macros.hpp"
#include "Plato_DriverInterface.hpp"
#include "Plato_DakotaAppInterface.hpp"

namespace Plato
{

inline std::shared_ptr<Dakota::ProgramOptions>
read_dakota_program_options(std::string aInputFileName = "")
{
    std::string tInputFileName = aInputFileName.empty() ? "plato_dakota_input_deck.i" : aInputFileName;

    auto tProgramOptions = std::make_shared<Dakota::ProgramOptions>();
    std::ifstream tInputFile(tInputFileName, std::ifstream::in);
    if(tInputFile.is_open())
    {
        tProgramOptions->input_file(tInputFileName);
        tInputFile.close();
    }
    else
    {
        THROWERR(std::string("Did not find input file \"" + tInputFileName + "\""))
    }

    return tProgramOptions;
}

template<typename ScalarType, typename OrdinalType = size_t>
class DakotaDriver : public Plato::DriverInterface<ScalarType, OrdinalType>
{
public:
    DakotaDriver(Plato::Interface *aInterface, const MPI_Comm &aComm) : 
      mComm(aComm),
      mInterface(aInterface)
    {
    }

    /******************************************************************************//**
     * @brief Return true if the last driver
    **********************************************************************************/
    bool lastDriver() const override { return true; }

    /******************************************************************************//**
     * \brief Return the driver type
     * \return driver type
     **********************************************************************************/
    Plato::driver_t driver() const override
    {
        return (Plato::driver_t::PLATO_DAKOTA_DRIVER);
    }

    /******************************************************************************//**
     * \brief Initialize inputs for this driver.
     **********************************************************************************/
    void initialize() override
    {
    }

    /******************************************************************************//**
     * \brief Run driver and perform work.
     **********************************************************************************/
    void run() override
    {
        // clear FE trap to avoid crashes in Dakota
        // some optimizers used by Dakota allow floating point exceptions
        // so we don't want them to send SIGFPE
        fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);

        this->initialize();

        auto tProgramOptions = Plato::read_dakota_program_options();
        auto tDakotaLibrary = this->buildDakotaLibrary(*tProgramOptions);
        tDakotaLibrary->execute();

        this->finalize();

// reenable FE trap
#ifndef NDEBUG
    feclearexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
    }

    /******************************************************************************//**
     * \brief Notifies Plato Engine that work in this driver is done.
     **********************************************************************************/
    void finalize() override
    {
        if(mInterface != nullptr)
        {
            mInterface->finalize();
        }
    }

private:
    std::shared_ptr<Dakota::LibraryEnvironment>
    buildDakotaLibrary(Dakota::ProgramOptions &aProgramOptions)
    {
        std::shared_ptr<Dakota::LibraryEnvironment> tDakotaLibrary =
            std::make_shared<Dakota::LibraryEnvironment>(mComm, aProgramOptions);

        std::string tModelType(""); // demo: empty string will match any model type
        std::string tInterfaceType("direct");
        std::string tAnalysisDriver("plato_dakota_plugin");
        Dakota::ProblemDescDB &tProblemDataBase = tDakotaLibrary->problem_description_db();

        std::shared_ptr<Dakota::Interface> tPlatoDakotaInterface = std::make_shared<Plato::DakotaAppInterface>(tProblemDataBase, mInterface); 

        auto tPluggedIn = tDakotaLibrary->plugin_interface(tModelType, tInterfaceType, tAnalysisDriver, tPlatoDakotaInterface);
        if (!tPluggedIn)
        {
            THROWERR(std::string("Plato-Dakota plugin was not allocated. Check compatibility between requested plugin and expected plugin '")
                + "plato_dakota_plugin" + "'. Keyword 'analysis_drivers' inside the 'interface' Dakota input file block should be set to 'plato_dakota_plugin'.")
        }

        return tDakotaLibrary;
    }

private:
    MPI_Comm mComm; /*!< MPI communicator */
    Plato::Interface *mInterface; /*!< provides access to Plato Engine interface */
};
// class DakotaDriver

}
// namespace Plato
