/*
 * XMLGeneratorPostOptimizationRunFileUtilities.cpp
 *
 *  Created on: Aug 26, 2021
 */

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorPostOptimizationRunFileUtilities.hpp"
#include "XMLGeneratorSierraSDInputDeckUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
void write_post_optimization_run_files
(const XMLGen::InputData& aMetaData)
{
    for(const XMLGen::Run &tCurRun : aMetaData.runs())
    {
        if(tCurRun.type() == "modal_analysis")
        {
            std::string tInputDeckName = build_post_optimization_run_input_deck_name(tCurRun);
            XMLGen::write_sierra_sd_modal_input_deck(tInputDeckName, tCurRun, aMetaData);
        }
    }
}

std::string build_post_optimization_run_input_deck_name(const XMLGen::Run &aRun)
{
    std::string tReturn = aRun.type();
    tReturn += "_run_";
    tReturn += aRun.id();
    tReturn += ".i";
    return tReturn;
}

}
// namespace XMLGen
