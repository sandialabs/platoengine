/*
 * Plato_EpetraSerialDenseVector.hpp
 *
 *  Created on: Nov 27, 2017
 */

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

#ifndef PLATO_EPETRASERIALDENSEVECTOR_HPP_
#define PLATO_EPETRASERIALDENSEVECTOR_HPP_

#include <memory>

#include "Plato_Vector.hpp"
#include "Epetra_SerialDenseVector.h"

namespace Plato
{

template<typename ScalarType, typename OrdinalType = size_t>
class EpetraSerialDenseVector : public Plato::Vector<ScalarType, OrdinalType>
{
public:
    explicit EpetraSerialDenseVector(const std::vector<ScalarType> & aInput) :
            mData(Epetra_SerialDenseVector(aInput.size()))
    {
        const OrdinalType tLength = aInput.size();
        for(OrdinalType tIndex = 0; tIndex < tLength; tIndex++)
        {
            mData[tIndex] = aInput[tIndex];
        }
    }
    explicit EpetraSerialDenseVector(const OrdinalType & aLength, ScalarType aValue = 0) :
            mData(Epetra_SerialDenseVector(aLength))
    {
        this->fill(aValue);
    }

    //! Scales a Vector by a ScalarType constant.
    void scale(const ScalarType & aInput) override
    {
        const OrdinalType tLength = mData.Length();
        mData.SCAL(static_cast<int>(tLength), aInput, mData.A());
    }
    //! Entry-Wise product of two vectors.
    void entryWiseProduct(const Plato::Vector<ScalarType, OrdinalType> & aInput) override
    {
        const OrdinalType tMyLength = mData.Length();
        assert(aInput.size() == tMyLength);

        for(OrdinalType tIndex = 0; tIndex < tMyLength; tIndex++)
        {
            mData[tIndex] = aInput[tIndex] * mData[tIndex];
        }
    }
    //! Update vector values with scaled values of A, this = beta*this + alpha*A.
    void update(const ScalarType & aAlpha,
                        const Plato::Vector<ScalarType, OrdinalType> & aInputVector,
                        const ScalarType & aBeta) override
    {
        const OrdinalType tMyLength = mData.Length();
        assert(aInputVector.size() == tMyLength);
        for(OrdinalType tIndex = 0; tIndex < tMyLength; tIndex++)
        {
            mData[tIndex] = aBeta * mData[tIndex] + aAlpha * aInputVector[tIndex];
        }
    }
    //! Computes the absolute value of each element in the container.
    void modulus() override
    {
        const OrdinalType tMyLength = this->size();
        for(OrdinalType tIndex = 0; tIndex < tMyLength; tIndex++)
        {
            mData[tIndex] = std::abs(mData[tIndex]);
        }
    }
    //! Returns the inner product of two vectors.
    ScalarType dot(const Plato::Vector<ScalarType, OrdinalType> & aInput) const override
    {
        const OrdinalType tMyLength = mData.Length();
        assert(aInput.size() == tMyLength);

        const Plato::EpetraSerialDenseVector<ScalarType, OrdinalType> & tInputVector =
                dynamic_cast<const Plato::EpetraSerialDenseVector<ScalarType, OrdinalType>&>(aInput);

        ScalarType tOutput = mData.DOT(tMyLength, mData.A(), tInputVector.mData.A());
        return (tOutput);
    }
    //! Assigns new contents to the Vector, replacing its current contents, and not modifying its size.
    void fill(const ScalarType & aValue) override
    {
        const OrdinalType tMyLength = this->size();
        for(OrdinalType tIndex = 0; tIndex < tMyLength; tIndex++)
        {
            mData[tIndex] = aValue;
        }
    }
    //! Returns the number of local elements in the Vector.
    OrdinalType size() const override
    {
        const OrdinalType tMyLength = mData.Length();
        return (tMyLength);
    }
    //! Creates an object of type Plato::Vector
    std::shared_ptr<Plato::Vector<ScalarType, OrdinalType>> create() const override
    {
        const ScalarType tBaseValue = 0;
        const OrdinalType tMyLength = this->size();
        std::shared_ptr<Plato::Vector<ScalarType, OrdinalType>> tOutput =
                std::make_shared<Plato::EpetraSerialDenseVector<ScalarType, OrdinalType>>(tMyLength, tBaseValue);
        return (tOutput);
    }
    //! Operator overloads the square bracket operator
    ScalarType & operator [](const OrdinalType & aIndex) override
    {
        assert(aIndex < this->size());
        return (mData[aIndex]);
    }
    //! Operator overloads the square bracket operator
    const ScalarType & operator [](const OrdinalType & aIndex) const override
    {
        assert(aIndex < this->size());
        return (mData[aIndex]);
    }
    //! Returns a direct pointer to the memory array used internally by the vector to store its owned elements.
    ScalarType* data() override
    {
        return (mData.A());
    }
    //! Returns a direct const pointer to the memory array used internally by the vector to store its owned elements.
    const ScalarType* data() const override
    {
        return (mData.A());
    }
    //! Returns reference to Epetra_SerialDenseVector
    Epetra_SerialDenseVector & vector()
    {
        return (mData);
    }
    //! Returns const reference to Epetra_SerialDenseVector
    const Epetra_SerialDenseVector & vector() const
    {
        return (mData);
    }

private:
    Epetra_SerialDenseVector mData;

private:
    EpetraSerialDenseVector(const Plato::EpetraSerialDenseVector<ScalarType, OrdinalType>&);
    Plato::EpetraSerialDenseVector<ScalarType, OrdinalType> & operator=(const Plato::EpetraSerialDenseVector<ScalarType, OrdinalType>&);
};

} // namespace Plato

#endif /* PLATO_EPETRASERIALDENSEVECTOR_HPP_ */
