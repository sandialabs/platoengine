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

#ifndef SRC_SIMP_HPP_
#define SRC_SIMP_HPP_

#include "Plato_PenaltyModel.hpp"
#include "Plato_SerializationHeaders.hpp"

namespace Plato {

class InputData;

  class SIMP : public PenaltyModel {
    public:
      SIMP() = default;
      SIMP(double a_penaltyExponent, double a_minimumValue) :
        m_penaltyExponent(a_penaltyExponent),
        m_minimumValue(a_minimumValue)
      {
      }
      SIMP(const Plato::InputData& input);

      double eval(double x) const override;
      double grad(double x) const override;

      template<class Archive>
      void serialize(Archive & aArchive, const unsigned int /*version*/)
      {
        aArchive & boost::serialization::make_nvp("PenaltyModel",boost::serialization::base_object<PenaltyModel>(*this));
        aArchive & boost::serialization::make_nvp("PenaltyExponent",m_penaltyExponent);
        aArchive & boost::serialization::make_nvp("MinimumValue",m_minimumValue);
      }

    private:
      double m_penaltyExponent = 1;
      double m_minimumValue = 0.0;
  };

} /* namespace Plato */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY2(Plato::SIMP, "SIMP")

#endif /* SRC_PENALTYMODEL_HPP_ */
