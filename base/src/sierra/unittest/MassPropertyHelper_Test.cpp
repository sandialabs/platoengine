#include <gtest/gtest.h>

#include <string>
#include <filesystem>
#include <boost/filesystem.hpp>

#include "MassPropertyHelper.hpp"

namespace Plato::Sierra::Test {
TEST(Sierra, MassPropertySaveAndLoad) {
  boost::filesystem::path temp = boost::filesystem::unique_path();
  const std::string tFilename    = temp.native();
  constexpr Plato::Sierra::MassPropertyHelper tSave{
  /*.mVolume =*/ 1,
  /*.mMass =*/ 2,
  /*.mCG =*/ {3, 4, 5},
  /*.mInertia =*/ {6, 7, 8, 9, 10, 11}};
  EXPECT_NO_THROW(Plato::Sierra::save_to_xml(tFilename, tSave));

  Plato::Sierra::MassPropertyHelper tLoad = Plato::Sierra::load_from_xml(tFilename);
  
  EXPECT_EQ(tSave.mVolume, tLoad.mVolume);
  EXPECT_EQ(tSave.mMass, tLoad.mMass);
  
  EXPECT_EQ(tSave.mCG, tLoad.mCG);
  EXPECT_EQ(tSave.mInertia, tLoad.mInertia);

  std::filesystem::remove(tFilename);
}

}  // namespace Plato::Sierra::Test
