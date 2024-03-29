import unittest
import pyCAPS
import ESPtools
import csmFileGenerator

csmFileName = "dummy_despmtrs.csm"
meshSettingsFileName = "dummy_mesh_settings.json"
writer = csmFileGenerator.csmFileGenerator(csmFileName)

def constructCAPSProblem(csmFilename):
    return pyCAPS.Problem(problemName = "ESP_Scratch_Dir", capsFile=csmFileName, outLevel=0)

class getInitialValues(unittest.TestCase):
    def test_errorNoDespmtrKeyword(self):
        writer.writeCsmFile([
            "designparameter Lx 12.4"
        ])

        with self.assertRaises(Exception) as errMsg:
            ESPtools.getInitialValues(csmFileName)

        self.assertEqual(str(errMsg.exception), "Parsing error: no keyword 'despmtr' found in file 'dummy_despmtrs.csm'")

    def test_errorMoreThanNineTokens(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 lbound 2.0 ubound 3.0 initial 2.5 final 45.6"
        ])

        with self.assertRaises(Exception) as errMsg:
            ESPtools.getInitialValues(csmFileName)

        self.assertEqual(str(errMsg.exception), "Parsing error: reading initial values failed.")

    def test_errorFewerThanThreeTokens(self):
        writer.writeCsmFile([
            "despmtr Lx"
        ])

        with self.assertRaises(Exception) as errMsg:
            ESPtools.getInitialValues(csmFileName)

        self.assertEqual(str(errMsg.exception), "Parsing error: reading initial values failed.")

    def test_errorBetweenThreeAndNineTokens(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 lbound 2.0 initial 2.5"
        ])

        with self.assertRaises(Exception) as errMsg:
            ESPtools.getInitialValues(csmFileName)

        self.assertEqual(str(errMsg.exception), "Parsing error: reading initial values failed.")

    def test_errorInitialNotFoundAsTokenWhereExpected(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 lbound 2.0 ubound 3.0 starting 2.5 \n"
        ])

        with self.assertRaises(Exception) as errMsg:
            ESPtools.getInitialValues(csmFileName)

        self.assertEqual(str(errMsg.exception), "Parsing error: expected 'initial' token, got 'starting'")

    def test_getCorrectValuesWithOnlyDespmtrKeyword(self):
        writer.writeCsmFile([
            "despmtr Lx 0.523 \n",
            "despmtr Ly 13.2"
        ])

        params = ESPtools.getInitialValues(csmFileName)

        self.assertEqual(len(params), 2)
        self.assertEqual(params[0], 0.523)
        self.assertEqual(params[1], 13.2)

    def test_getCorrectValuesWithAllKeywords(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "despmtr Ly 39.5 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "despmtr Lz 0.09834 lbound 5.0 ubound 8.0 initial 6.5"
        ])

        params = ESPtools.getInitialValues(csmFileName)

        self.assertEqual(len(params), 3)
        self.assertEqual(params[0], 2.5)
        self.assertEqual(params[1], 3.5)
        self.assertEqual(params[2], 6.5)

    def test_getCorrectValuesWithMixedKeywords(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 \n",
            "despmtr Ly 39.5 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "despmtr Lz 0.09834"
        ])

        params = ESPtools.getInitialValues(csmFileName)

        self.assertEqual(len(params), 3)
        self.assertEqual(params[0], 2.5)
        self.assertEqual(params[1], 3.5)
        self.assertEqual(params[2], 0.09834)

class getCurrentValues(unittest.TestCase):
    def test_errorNoDespmtrsInCAPSProblem(self):
        writer.writeCsmFile([
            "conpmtr Lx 2.5 \n",
            "conpmtr Ly 39.5 \n",
            "conpmtr Lz 0.09834"
        ])
        problem = constructCAPSProblem(csmFileName)

        with self.assertRaises(Exception) as errMsg:
            ESPtools.getCurrentValues(problem)

        self.assertEqual(str(errMsg.exception), "Error: no 'despmtr' objects in the CAPS problem constructed with the given csm file.")

    def test_getCorrectValuesWithAllKeywords(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "despmtr Ly 39.5 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "despmtr Lz 0.09834 lbound 5.0 ubound 8.0 initial 6.5"
        ])
        problem = constructCAPSProblem(csmFileName)

        params = ESPtools.getCurrentValues(problem)

        self.assertEqual(len(params), 3)
        self.assertEqual(params[0], 2.5)
        self.assertEqual(params[1], 39.5)
        self.assertEqual(params[2], 0.09834)

    def test_getCorrectValuesWithOnlyDespmtrKeyword(self):
        writer.writeCsmFile([
            "despmtr Lx 0.523 \n",
            "despmtr Ly 13.2"
        ])
        problem = constructCAPSProblem(csmFileName)

        params = ESPtools.getCurrentValues(problem)

        self.assertEqual(len(params), 2)
        self.assertEqual(params[0], 0.523)
        self.assertEqual(params[1], 13.2)

    def test_getCorrectValuesWithMixedKeywords(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 \n",
            "despmtr Ly 39.5 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "despmtr Lz 4.5 lbound 3.0 ubound 5.0 initial 3.5"
        ])
        problem = constructCAPSProblem(csmFileName)

        params = ESPtools.getCurrentValues(problem)

        self.assertEqual(len(params), 3)
        self.assertEqual(params[0], 2.5)
        self.assertEqual(params[1], 39.5)
        self.assertEqual(params[2], 4.5)

class setDesignParameterValues(unittest.TestCase):
    def test_errorFewerDespmtrsThanExpected(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 \n",
            "despmtr Lz 0.09834"
        ])
        problem = constructCAPSProblem(csmFileName)

        newVals = [10.0, 9.0, 8.0]
        with self.assertRaises(Exception) as errMsg:
            ESPtools.setDesignParameterValues(problem, newVals)

        self.assertEqual(str(errMsg.exception), "Error: Number of values provided does not equal the number of 'despmtr' objects in CAPS problem.")

    def test_dummyValuesSetCorrectly(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 \n",
            "despmtr Ly 39.5 \n",
            "despmtr Lz 0.09834"
        ])
        problem = constructCAPSProblem(csmFileName)

        newVals = [10.0, 9.0, 8.0]
        ESPtools.setDesignParameterValues(problem, newVals)

        params = ESPtools.getCurrentValues(problem)

        self.assertEqual(len(params), 3)
        self.assertEqual(params[0], 10.0)
        self.assertEqual(params[1], 9.0)
        self.assertEqual(params[2], 8.0)

class parametersAreEqual(unittest.TestCase):
    def test_parametersHaveDifferentLengths(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 \n",
            "despmtr Ly 39.5 \n",
            "despmtr Lz 0.09"
        ])
        problem = constructCAPSProblem(csmFileName)

        initialParams = ESPtools.getInitialValues(csmFileName)

        newVals = [2.5, 39.5, 0.09, 13.4]

        paramsAreSame = ESPtools.parametersAreEqual(initialParams, newVals)
        self.assertFalse(paramsAreSame)

    def test_parametersAreNotEqual(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 \n",
            "despmtr Ly 39.5 \n",
            "despmtr Lz 0.09"
        ])
        problem = constructCAPSProblem(csmFileName)

        initialParams = ESPtools.getInitialValues(csmFileName)

        newVals = [10.0, 9.0, 8.0]
        ESPtools.setDesignParameterValues(problem, newVals)
        newParams = ESPtools.getCurrentValues(problem)

        paramsAreSame = ESPtools.parametersAreEqual(initialParams, newParams)
        self.assertFalse(paramsAreSame)

    def test_parametersAreNotEqualWithinTolerance(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 \n",
            "despmtr Ly 39.5 \n",
            "despmtr Lz 0.09"
        ])
        problem = constructCAPSProblem(csmFileName)

        initialParams = ESPtools.getInitialValues(csmFileName)

        newVals = [2.5, 39.5000001, 0.09]
        ESPtools.setDesignParameterValues(problem, newVals)
        newParams = ESPtools.getCurrentValues(problem)

        paramsAreSame = ESPtools.parametersAreEqual(initialParams, newParams)
        self.assertFalse(paramsAreSame)

    def test_parametersAreEqual(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 \n",
            "despmtr Ly 39.5 \n",
            "despmtr Lz 0.09"
        ])
        problem = constructCAPSProblem(csmFileName)

        initialParams = ESPtools.getInitialValues(csmFileName)

        newVals = [2.5, 39.5, 0.09]
        ESPtools.setDesignParameterValues(problem, newVals)
        newParams = ESPtools.getCurrentValues(problem)

        paramsAreSame = ESPtools.parametersAreEqual(initialParams, newParams)
        self.assertTrue(paramsAreSame)

class prepareGeometryForMeshMorph(unittest.TestCase):
    def test_currentAndInitialValuesAreEquivalent(self):
        writer.writeCsmFile([
            "despmtr Lx 2.5 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "despmtr Ly 3.5 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "despmtr Lz 6.5 lbound 5.0 ubound 8.0 initial 6.5"
        ])
        problem = constructCAPSProblem(csmFileName)
        currentVals = ESPtools.getCurrentValues(problem)

        meshMorph = ESPtools.prepareGeometryForMeshMorph(csmFileName, problem, currentVals)

        self.assertEqual(meshMorph, False)

    def test_currentAndInitialValuesAreDifferent(self):
        writer.writeCsmFile([
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "despmtr Ly 3.99 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "despmtr Lz 7.57 lbound 5.0 ubound 8.0 initial 6.5"
        ])
        problem = constructCAPSProblem(csmFileName)
        currentVals = ESPtools.getCurrentValues(problem)

        meshMorph = ESPtools.prepareGeometryForMeshMorph(csmFileName, problem, currentVals)
        preparedVals = ESPtools.getCurrentValues(problem)

        self.assertEqual(meshMorph, True)
        self.assertEqual(preparedVals, [2.5, 3.5, 6.5])

class parseMeshLength(unittest.TestCase):
    def test_errorNoMeshLengthKeyword(self):
        writer.writeCsmFile([
            "despmtr Lz 7.57 lbound 5.0 ubound 8.0 initial 6.5"
        ])

        with self.assertRaises(Exception) as errMsg:
            ESPtools.parseMeshLength(csmFileName)

        self.assertEqual(str(errMsg.exception), "Error reading CSM file: required variable, 'MeshLength', not found.")

    def test_errorMultipleMeshLengthKeywords(self):
        writer.writeCsmFile([
            "set MeshLength 0.25 \n",
            "despmtr Ly 3.99 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "set MeshLength 1.5"
        ])

        with self.assertRaises(Exception) as errMsg:
            ESPtools.parseMeshLength(csmFileName)

        self.assertEqual(str(errMsg.exception), "Error reading CSM file: multiple 'MeshLength' keywords found. 'MeshLength' variable should appear once.")

    def test_returnCorrectMeshLengthString(self):
        writer.writeCsmFile([
            "set MeshLength 0.25"
        ])

        meshLength = ESPtools.parseMeshLength(csmFileName)
        self.assertIsInstance(meshLength, str)
        self.assertEqual(meshLength, "0.25\n")

class parseMeshSettingsFile(unittest.TestCase):
    def test_errorNoJsonFile(self):
        with self.assertRaises(Exception) as errMsg:
            ESPtools.parseMeshSettingsFile("not_a_real_file.json")

        self.assertEqual(str(errMsg.exception), "Error mesh settings json file not found.")

    def test_meshSettingsFileNone(self):
        settings = ESPtools.parseMeshSettingsFile(None)
        self.assertEqual(settings, None)

    def test_jsonParsedIntoDict(self):
        settings = ESPtools.parseMeshSettingsFile(meshSettingsFileName)
        self.assertIs(type(settings), dict)
        self.assertEqual(settings, {"circumference":{"edgeDistribution":"Even","numEdgePoints":10}})

class insertCurrentParameterVals(unittest.TestCase):
    def test_emptyParameterValsChangesNothing(self):
        writer.writeCsmFile([
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "despmtr Ly 3.99 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "despmtr Lz 7.57 lbound 5.0 ubound 8.0 initial 6.5"
        ])
        
        parameterVals = []

        ESPtools.insertCurrentParameterVals(csmFileName, parameterVals)

        problem = constructCAPSProblem(csmFileName)
        params = ESPtools.getCurrentValues(problem)

        self.assertEqual(len(params), 3)
        self.assertEqual(params[0], 2.65)
        self.assertEqual(params[1], 3.99)
        self.assertEqual(params[2], 7.57)

    def test_firstParameterValInsertedCorrectly(self):
        writer.writeCsmFile([
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "despmtr Ly 3.99 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "despmtr Lz 7.57 lbound 5.0 ubound 8.0 initial 6.5"
        ])
        
        parameterVals = [91.77]

        ESPtools.insertCurrentParameterVals(csmFileName, parameterVals)

        problem = constructCAPSProblem(csmFileName)
        params = ESPtools.getCurrentValues(problem)

        self.assertEqual(len(params), 3)
        self.assertEqual(params[0], 91.77)
        self.assertEqual(params[1], 3.99)
        self.assertEqual(params[2], 7.57)

    def test_allParameterValsInsertedCorrectly(self):
        writer.writeCsmFile([
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "despmtr Ly 3.99 lbound 3.0 ubound 5.0 initial 3.5 \n",
            "despmtr Lz 7.57 lbound 5.0 ubound 8.0 initial 6.5"
        ])
        
        parameterVals = [91, 77, 86]

        ESPtools.insertCurrentParameterVals(csmFileName, parameterVals)

        problem = constructCAPSProblem(csmFileName)
        params = ESPtools.getCurrentValues(problem)

        self.assertEqual(len(params), 3)
        self.assertEqual(params[0], 91.0)
        self.assertEqual(params[1], 77.0)
        self.assertEqual(params[2], 86.0)

class UpdateFileTester(unittest.TestCase):
    def assertCsmFileContentsEqual(self, goldContents):
        file = open(csmFileName, "r")
        contents = file.readlines()
        file.close()

        self.assertEqual(len(contents), len(goldContents))

        for i in range(len(contents)):
            self.assertEqual(contents[i], goldContents[i])

class updateModelAflr4Aflr3Exodus(UpdateFileTester):
    def test_appendedLinesCorrectWithoutNewParameters(self):
        writer.writeCsmFile([
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "set MeshLength 0.25 \n",
            "end"
        ])

        ESPtools.updateModelAflr4Aflr3Exodus(csmFileName, [])

        goldContents = [
            "attribute capsAIM $aflr4AIM;aflr3AIM;platoAIM\n",
            "attribute capsMeshLength 0.25\n",
            "\n",
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "set MeshLength 0.25 \n",
            "patbeg i @stack.size\n",
            "  select body @stack[i]\n",
            "  attribute _name $block_+val2str(i,0)\n",
            "patend\n",
            "end"
        ]

        self.assertCsmFileContentsEqual(goldContents)

    def test_appendedLinesCorrectWithNewParameters(self):
        writer.writeCsmFile([
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "set MeshLength 0.25 \n",
            "end"
        ])

        ESPtools.updateModelAflr4Aflr3Exodus(csmFileName, [91.0])

        goldContents = [
            "attribute capsAIM $aflr4AIM;aflr3AIM;platoAIM\n",
            "attribute capsMeshLength 0.25\n",
            "\n",
            "despmtr Lx 91.0 lbound 2.0 ubound 3.0 initial 2.5\n",
            "set MeshLength 0.25 \n",
            "patbeg i @stack.size\n",
            "  select body @stack[i]\n",
            "  attribute _name $block_+val2str(i,0)\n",
            "patend\n",
            "end\n"
        ]

        self.assertCsmFileContentsEqual(goldContents)

class updateModelAflr2Exodus(UpdateFileTester):
    def test_appendedLinesCorrectWithoutNewParameters(self):
        writer.writeCsmFile([
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "outpmtr MeshLength \n",
            "set MeshLength 0.25 \n",
            "end"
        ])

        ESPtools.updateModelAflr2Exodus(csmFileName, [])

        goldContents = [
            "attribute capsAIM $aflr2AIM;platoAIM\n",
            "attribute capsMeshLength 1.0 \n",
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "outpmtr MeshLength \n",
            "set MeshLength 0.25 \n",
            "end"
        ]

        self.assertCsmFileContentsEqual(goldContents)

    def test_appendedLinesCorrectWithNewParameters(self):
        writer.writeCsmFile([
            "despmtr Lx 2.65 lbound 2.0 ubound 3.0 initial 2.5 \n",
            "outpmtr MeshLength \n",
            "set MeshLength 0.25 \n",
            "end"
        ])

        ESPtools.updateModelAflr2Exodus(csmFileName, [77.0])

        goldContents = [
            "attribute capsAIM $aflr2AIM;platoAIM\n",
            "attribute capsMeshLength 1.0 \n",
            "despmtr Lx 77.0 lbound 2.0 ubound 3.0 initial 2.5\n",
            "outpmtr MeshLength \n",
            "set MeshLength 0.25 \n",
            "end\n"
        ]

        self.assertCsmFileContentsEqual(goldContents)


if __name__ == '__main__':
    unittest.main()
