import unittest
import MeshGenerationTester
import ESPtools
import exodus
import csmFileGenerator

csmFileName = "dummy_despmtrs.csm"
writer = csmFileGenerator.csmFileGenerator(csmFileName)

def generateMesh(csmFunc, val, meshName, meshMorph, csmFileName=csmFileName, meshSettingsFileName=None):
    csmFunc(val)
    ESPtools.aflr2_meshing(csmFileName, meshName, meshMorph, meshSettingsFileName, quiet=True)
    mesh = exodus.ExodusDB()
    mesh.read(meshName)
    return mesh

class MeshSettingsFile(MeshGenerationTester.MeshGenerationTester):
    def test_ImportingSettingsJsonFileChangesResult(self):
        radius = 2.0
        defaultMesh = generateMesh(writer.writeSquareWithCircularHoleMeshCsm, radius, meshName="initial_mesh.exo", meshMorph=False)
        settingsSpecifiedMesh = generateMesh(writer.writeSquareWithCircularHoleMeshCsm, radius, meshName="initial_mesh.exo", meshMorph=False, meshSettingsFileName="dummy_mesh_settings.json")
        self.assertMeshesAreDifferent(defaultMesh, settingsSpecifiedMesh)



if __name__ == "__main__":
    unittest.main()