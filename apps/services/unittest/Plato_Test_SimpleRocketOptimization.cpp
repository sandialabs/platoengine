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
 * Plato_Test_SimpleRocketOptimization.cpp
 *
 *  Created on: Aug 29, 2018
 */

#include "gtest/gtest.h"

#include "Plato_StandardVector.hpp"
#include "Plato_StandardMultiVector.hpp"
#include "Plato_GradBasedRocketObjective.hpp"

#include "PSL_Random.hpp"
#include "PSL_AbstractAuthority.hpp"

namespace PlatoTest
{

/******************************************************************************//**
 * @brief Return target thrust profile for gradient-based unit test
 * @return standard vector with target thrust profile
**********************************************************************************/
std::vector<double> get_target_thrust_profile()
{
    std::vector<double> tTargetThrustProfile =
        { 0, 6796.7769344285716215, 6867.4666589320704588, 6938.7140813601199625, 7010.5222074219882415,
                7082.8940515426593265, 7155.8326368664666006, 7229.3409952607416926, 7303.4221673194269897,
                7378.0792023667154353, 7453.3151584606503093, 7529.1331023967359215, 7605.5361097115401208,
                7682.5272646862767942, 7760.1096603504056475, 7838.2863984852019712, 7917.0605896273045801,
                7996.4353530723155927, 8076.4138168783356377, 8156.9991178695054259, 8238.1944016395609651,
                8320.0028225553669472, 8402.427543760421031, 8485.4717371784081479, 8569.138583516696599,
                8653.4312722698141442, 8738.3530017229986697, 8823.9069789556815522, 8910.0964198449109972,
                8996.9245490689172584, 9084.3946001105268806, 9172.5098152606296935, 9261.2734456216821854,
                9350.6887511110835476, 9440.7590004646899615, 9531.4874712402197474, 9622.8774498206785211,
                9714.9322314177934459, 9807.6551200754420279, 9901.0494286730263411, 9995.1184789289254695,
                10089.86560140385518, 10185.29413550427671, 10281.40742948577099, 10378.208840456431062,
                10475.701734380227208, 10573.889486080362985, 10672.775479242660367, 10772.363106418877578,
                10872.655769030066949, 10973.656877369950962, 11075.369850608207344, 11177.798116793806912,
                11280.945112858358698, 11384.814284619407772, 11489.409086783754901, 11594.732982950727092,
                11700.789445615524528, 11807.581956172476566, 11915.114004918334103, 12023.389091055551035,
                12132.410722695569348, 12242.182416862038735, 12352.707699494156259, 12463.990105449835028,
                12576.03317850901476, 12688.840471376890491, 12802.415545687126723, 12916.761972005117059,
                13031.883329831192896, 13147.783207603848496, 13264.465202702955139, 13381.932921452975279,
                13500.189979126145772, 13619.239999945690215, 13739.086617089016727, 13859.733472690882991,
                13981.184217846597676, 14103.442512615172745, 14226.512026022497594, 14350.396436064524096,
                14475.099429710398908, 14600.624702905623053, 14726.975960575182398, 14854.156916626747261,
                14982.171293953710119, 15111.022824438417956, 15240.715248955209972, 15371.252317373619007,
                15502.637788561391062, 15634.875430387684901, 15767.969019726122497, 15901.922342457881314,
                16036.739193474852073, 16172.423376682612798, 16308.97870500362842, 16446.409000380237558,
                16584.718093777799368, 16723.909825187685783, 16863.988043630397442 };

    return (tTargetThrustProfile);
}
// function get_target_thrust_profile

TEST(PlatoTest, Cylinder)
{
    // allocate problem inputs - use default parameters
    const double tRadius = 1;
    const double tLength = 2;
    Plato::Cylinder<double> tCylinder(tRadius, tLength);

    // test area calculation
    double tTolerance = 1e-6;
    double tArea = tCylinder.area();
    EXPECT_NEAR(tArea, 12.566370614359172, tTolerance);

    // test gradient
    const size_t tNumControls = 2;
    std::vector<double> tGrad(tNumControls);
    tCylinder.gradient(tGrad);
    EXPECT_NEAR(tGrad[0], 12.56637061435917, tTolerance);EXPECT_NEAR(tGrad[1], 6.283185307179586, tTolerance);

    // test update initial configuration
    std::map<std::string, double> tParam;
    tParam.insert(std::pair<std::string, double>("Radius", 2));
    tParam.insert(std::pair<std::string, double>("Configuration", Plato::Configuration::INITIAL));
    tCylinder.update(tParam);
    tArea = tCylinder.area();
    EXPECT_NEAR(tArea, 25.132741228718345, tTolerance);

    // test update dynamics configuration
    tParam.insert(std::pair<std::string, double>("BurnRate", 10));
    tParam.insert(std::pair<std::string, double>("DeltaTime", 0.1));
    tParam.find("Configuration")->second = Plato::Configuration::DYNAMIC;
    tCylinder.update(tParam);
    tArea = tCylinder.area();
    EXPECT_NEAR(tArea, 37.699111843077520, tTolerance);
}

} // namespace PlatoTest
