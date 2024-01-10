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
 * Plato_UnitTestUtils.hpp
 *
 *  Created on: Oct 21, 2017
 */

#ifndef PLATO_UNITTESTUTILS_HPP_
#define PLATO_UNITTESTUTILS_HPP_

#include <string>
#include <fstream>
#include <cassert>
#include <iostream>
#include <iomanip>

#include "Plato_Macros.hpp"
#include "Plato_Vector.hpp"
#include "Plato_MultiVector.hpp"
#include <Plato_FreeFunctions.hpp>

namespace PlatoTest
{

template<typename ScalarType, typename OrdinalType>
inline void printVector(const Plato::Vector<ScalarType, OrdinalType> & aInput)
{
    for (OrdinalType tIndex = 0; tIndex < aInput.size(); tIndex++)
    {
        std::cout << std::setprecision(10) << "X(" << tIndex << ") = " << aInput[tIndex] << "\n" << std::flush;
    }
}

template<typename ScalarType, typename OrdinalType>
inline void printMultiVector(const Plato::MultiVector<ScalarType, OrdinalType> & aInput)
{
    auto tNumVectors = aInput.getNumVectors();
    for(decltype(tNumVectors) tVectorIndex = 0; tVectorIndex < tNumVectors; tVectorIndex++)
    {
        for(decltype(tNumVectors) tElementIndex = 0; tElementIndex < aInput[tVectorIndex].size(); tElementIndex++)
        {
            std::cout << std::setprecision(10) << "X(" << tVectorIndex << ", " << tElementIndex << ") = "
            << aInput(tVectorIndex, tElementIndex) << "\n" << std::flush;
        }
    }
}

template<typename ScalarType, typename OrdinalType>
inline void checkVectorData(const Plato::Vector<ScalarType, OrdinalType> & aInput,
                            const Plato::Vector<ScalarType, OrdinalType> & aGold,
                            ScalarType aTolerance = 1e-6)
{
    if(aInput.size() != aGold.size())
    {
        std::string tMsg = std::string("DIMENSION MISMATCH. INPUT AND GOLD VECTORS DIMENSIONS DO NOT MATCH")
                + "INPUT SIZE IS " + std::to_string(aInput.size()) + " AND GOLD SIZE IS "
                + std::to_string(aInput.size()) + ".\n";
        THROWERR (tMsg)
    }

    OrdinalType tNumElements = aInput.size();
    for(OrdinalType tElemIndex = 0; tElemIndex < tNumElements; tElemIndex++)
    {
        ASSERT_NEAR(aInput[tElemIndex], aGold[tElemIndex], aTolerance);
    }
}

template<typename ScalarType, typename OrdinalType>
inline void checkMultiVectorData(const Plato::MultiVector<ScalarType, OrdinalType> & aInput,
                                const Plato::MultiVector<ScalarType, OrdinalType> & aGold,
                                ScalarType aTolerance = 1e-6)
{
    assert(aInput.getNumVectors() == aGold.getNumVectors());
    OrdinalType tNumVectors = aInput.getNumVectors();
    for(OrdinalType tVectorIndex = 0; tVectorIndex < tNumVectors; tVectorIndex++)
    {
        OrdinalType tNumElements = aInput[tVectorIndex].size();
        for(OrdinalType tElemIndex = 0; tElemIndex < tNumElements; tElemIndex++)
        {
            EXPECT_NEAR(aInput(tVectorIndex,tElemIndex), aGold(tVectorIndex,tElemIndex), aTolerance);
        }
    }
}

/******************************************************************************/
inline std::stringstream read_data_from_file(const std::string& aFilename)
{
    std::ifstream tReadFile;
    tReadFile.open(aFilename);
    std::string tInputString;
    std::stringstream tReadData;
    while (tReadFile >> tInputString)
    {
        tReadData << tInputString.c_str();
    }
    tReadFile.close();
    return (tReadData);
}
// function read_data_from_file
/******************************************************************************/

} // namespace PlatoTest

#endif /* PLATO_UNITTESTUTILS_HPP_ */
