#pragma once
#include <array>
#include <string>
#include <string_view>

#include "Plato_SerializationLoadSave.hpp"

namespace Plato::Sierra {
constexpr unsigned int X = 0;
constexpr unsigned int Y = 1;
constexpr unsigned int Z = 2;
constexpr unsigned int XX = 0;
constexpr unsigned int YY = 1;
constexpr unsigned int ZZ = 2;
constexpr unsigned int XY = 3;
constexpr unsigned int YZ = 4;
constexpr unsigned int ZX = 5;

constexpr std::string_view MASS_PROPERTY_SERIALIZATION_NAME = "MassPro";
struct MassPropertyHelper {
  double mVolume = 0;
  double mMass = 0;
  std::array<double, 3> mCG = {0, 0, 0};
  std::array<double, 6> mInertia = {0, 0, 0, 0, 0, 0};
  template <class Archive>
  void serialize(Archive& aArchive, const unsigned int /*version*/) {
    aArchive& boost::serialization::make_nvp("Volume", mVolume);
    aArchive& boost::serialization::make_nvp("Mass", mMass);
    aArchive& boost::serialization::make_nvp("CenterOfGravity_X", mCG[X]);
    aArchive& boost::serialization::make_nvp("CenterOfGravity_Y", mCG[Y]);
    aArchive& boost::serialization::make_nvp("CenterOfGravity_Z", mCG[Z]);
    aArchive& boost::serialization::make_nvp("Inertia_XX", mInertia[XX]);
    aArchive& boost::serialization::make_nvp("Inertia_YY", mInertia[YY]);
    aArchive& boost::serialization::make_nvp("Inertia_ZZ", mInertia[ZZ]);
    aArchive& boost::serialization::make_nvp("Inertia_XY", mInertia[XY]);
    aArchive& boost::serialization::make_nvp("Inertia_YZ", mInertia[YZ]);
    aArchive& boost::serialization::make_nvp("Inertia_ZX", mInertia[ZX]);
  }
};

void save_to_xml(const std::string& aFilename, const MassPropertyHelper& aMP);
MassPropertyHelper load_from_xml(const std::string& aFilename);

}  // namespace Plato::Sierra