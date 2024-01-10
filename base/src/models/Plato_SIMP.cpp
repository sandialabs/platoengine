#include "Plato_SIMP.hpp"
#include "Plato_Parser.hpp"

#include <math.h>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp> // For unit tests
#include <boost/archive/text_iarchive.hpp>
BOOST_CLASS_EXPORT_IMPLEMENT(Plato::SIMP)

namespace Plato {

SIMP::SIMP(const Plato::InputData& input) :
  m_penaltyExponent(Get::Double(input,"PenaltyExponent")),
  m_minimumValue(Get::Double(input,"MinimumValue"))
{
}

double SIMP::eval(double x) const
{
  return m_minimumValue + (1.0 - m_minimumValue)*pow(x, m_penaltyExponent);
}

double SIMP::grad(const double x) const
{
  return m_penaltyExponent == 0 ? 
    0 : 
   (1.0 - m_minimumValue)*m_penaltyExponent*pow(x,m_penaltyExponent - 1.0);
}
}
