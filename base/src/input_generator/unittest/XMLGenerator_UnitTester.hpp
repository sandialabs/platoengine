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
 * XMLGenerator_UnitTester.hpp
 *
 *  Created on: Sep 28, 2017
 *
 */

#ifndef SRC_XMLGENERATOR_UNITTESTER_HPP_
#define SRC_XMLGENERATOR_UNITTESTER_HPP_

#include "XMLGeneratorDataStruct.hpp"
#include "XMLGenerator.hpp"
#include "XMLGeneratorOptimizationParametersMetadata.hpp"


class XMLGenerator_UnitTester : public XMLGen::XMLGenerator
{

public:
    XMLGenerator_UnitTester();
    ~XMLGenerator_UnitTester();

    bool publicParseSingleValue(const std::vector<std::string> &aTokens,
                                const std::vector<std::string> &aInputStrings,
                                std::string &aReturnStringValue);
    bool publicParse_Tokens(char *buffer, std::vector<std::string> &tokens);
    bool publicParseSingleUnLoweredValue(const std::vector<std::string> &aTokens,
                                         const std::vector<std::string> &aUnLoweredTokens,
                                         const std::vector<std::string> &aInputStrings,
                                         std::string &aReturnStringValue);
    bool publicParseLoads(std::istream &sin);
    void publicParseConstraints(std::istream &sin);
    void publicParseObjective(std::istream &sin);
    void publicParseMaterials(std::istream &sin);
    void publicParseServices(std::istream &sin);
    void publicParseScenarios(std::istream &sin);
    void publicParseCriteria(std::istream &sin);
    void publicParseUncertainties(std::istream &sin);
    void publicParseBCs(std::istream &sin);
    bool publicParseOptimizationParameters(std::istream &sin);
    bool publicParseMesh(std::istream &sin);
    bool publicParseBlocks(std::istream &sin);
    bool publicRunSROMForUncertainVariables(XMLGen::InputData& aInputData);

    std::string getBlockID(const int &aIndex) {return m_InputData.blocks[aIndex].block_id;}
    std::string getBlockName(const int &aIndex) {return m_InputData.blocks[aIndex].name;}
    std::string getBlockMaterialID(const int &aIndex) {return m_InputData.blocks[aIndex].material_id;}
    std::string getBlockElementType(const int &aIndex) {return m_InputData.blocks[aIndex].element_type;}
    std::vector<double> getBoundingBox(const int &aIndex) {return m_InputData.blocks[aIndex].bounding_box;}
    std::string getCriterionID(const std::string& aIndex) {return m_InputData.criterion(aIndex).id();}
    std::string getCriterionType(const std::string& aIndex) {return m_InputData.criterion(aIndex).type();}
    std::string getCriterionBlock(const std::string& aIndex) {return m_InputData.criterion(aIndex).block();}
    std::string getBCApplicationType(const std::string &aBCID);
    std::string getBCApplicationID(const std::string &aBCID);
    std::string getBCApplicationDOF(const std::string &aBCID);
    std::string getInitDensityValue() {return m_InputData.optimization_parameters().initial_density_value();}
    std::string getMaxIterations() {return m_InputData.optimization_parameters().max_iterations();}
    std::string getRestartIteration() {return m_InputData.optimization_parameters().restart_iteration();}
    std::string getRestartFieldName() {return m_InputData.optimization_parameters().initial_guess_field_name();}
    std::string getRestartMeshFilename() {return m_InputData.optimization_parameters().initial_guess_file_name();}

    std::string getFixedBlock(const int &aIndex) {return m_InputData.optimization_parameters().fixed_block_ids()[aIndex];}
    std::string getFixedSideset(const int &aIndex) {return m_InputData.optimization_parameters().fixed_sideset_ids()[aIndex];}
    std::string getFixedNodeset(const int &aIndex) {return m_InputData.optimization_parameters().fixed_nodeset_ids()[aIndex];}
    std::string getFilterPower() {return m_InputData.optimization_parameters().filter_power();}
    std::string getNumberProcessors();
    std::string getFilterScale() {return m_InputData.optimization_parameters().filter_radius_scale();}
    std::string getFilterAbsolute() {return m_InputData.optimization_parameters().filter_radius_absolute();}
    std::string getAlgorithm() {return m_InputData.optimization_parameters().optimization_algorithm();}
    std::string getDiscretization() {return m_InputData.optimization_parameters().discretization();}
    std::string getCheckGradient() {return m_InputData.optimization_parameters().check_gradient();}
    std::string getCheckHessian() {return m_InputData.optimization_parameters().check_hessian();}
    std::string getMeshMorph() {return m_InputData.optimization_parameters().mesh_morph();}
    XMLGen::OptimizationType getOptimizationType() {return m_InputData.optimization_parameters().optimizationType();}
    std::vector<std::string> getDescriptors() {return m_InputData.optimization_parameters().descriptors();}
    std::vector<std::string> getLowerBounds() {return m_InputData.optimization_parameters().lower_bounds();}
    std::vector<std::string> getUpperBounds() {return m_InputData.optimization_parameters().upper_bounds();}
    std::vector<std::string> getMDPSPartitions() {return m_InputData.optimization_parameters().mdps_partitions();}
    std::string getConcurrentEvaluations() {return m_InputData.optimization_parameters().concurrent_evaluations();}
    bool getNeedsMeshMap() {return m_InputData.optimization_parameters().needsMeshMap();}
    std::string getFilterBeforeSymmetry() {return m_InputData.optimization_parameters().filter_before_symmetry_enforcement();}
    std::string getMeshMapFilterRadius() {return m_InputData.optimization_parameters().mesh_map_filter_radius();}
    std::string getMeshMapSearchTolerance() {return m_InputData.optimization_parameters().mesh_map_search_tolerance();}
    std::string getMeshName() {return m_InputData.mesh.name;}
    std::string getAuxiliaryMeshName() {return m_InputData.mesh.auxiliary_mesh_name;}
    void clearInputData();
    XMLGen::InputData* exposeInputData() {return &m_InputData;}
    
};


#endif /* SRC_XMLGENERATOR_UNITTESTER_HPP_ */
