import unittest
import ESPtools
import exodus
import csmFileGenerator

csmFileName = "dummy_despmtrs.csm"
writer = csmFileGenerator.csmFileGenerator(csmFileName)

def generateMesh(csmFunc, meshFunc, val, meshName, meshMorph):
    csmFunc(val)
    meshFunc(csmFileName, meshName, meshMorph, quiet=True)
    mesh = exodus.ExodusDB()
    mesh.read(meshName)
    return mesh

def generateInitialMesh(csmFunc, meshFunc, val):
    initialMeshName = "initial_mesh.exo"
    mesh = generateMesh(csmFunc, meshFunc, val, initialMeshName, meshMorph=False)
    return mesh

def generatePerturbedMesh(csmFunc, meshFunc, val, meshMorph):
    initialMeshName = "perturbed_mesh.exo"
    mesh = generateMesh(csmFunc, meshFunc, val, initialMeshName, meshMorph=meshMorph)
    return mesh

class MeshMorphTester(unittest.TestCase):
    def assertMeshesAreDifferent(self, mesh1, mesh2):
        self.assertNotEqual(mesh1.numNodes, mesh2.numNodes)
        self.assertNotEqual(mesh1.numElements, mesh2.numElements)
        self.assertNotEqual(mesh1.elementBlocks[0].connectivity, mesh2.elementBlocks[0].connectivity)

    def assertMeshesAreSame(self, mesh1, mesh2):
        self.assertEqual(mesh1.numNodes, mesh2.numNodes)
        self.assertEqual(mesh1.numElements, mesh2.numElements)
        self.assertEqual(mesh1.elementBlocks[0].connectivity, mesh2.elementBlocks[0].connectivity)

class MeshMorphInAflr4Aflr3Workflow(MeshMorphTester):
    def __init__(self, *args, **kwargs):
        super(MeshMorphInAflr4Aflr3Workflow, self).__init__(*args, **kwargs)
        self.meshFunction = ESPtools.aflr4_aflr3_meshing

    def test_BoxConnectivityDoesNotMatchInitialWhenRemeshed(self):
        Lx = 1.0
        initialMesh = generateInitialMesh(writer.writeBoxMeshCsm, self.meshFunction, Lx)

        Lx = 1.1
        perturbedMesh = generatePerturbedMesh(writer.writeBoxMeshCsm, self.meshFunction, Lx, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_BoxConnectivityMatchesInitialWhenMorphed(self):
        Lx = 1.0
        initialMesh = generateInitialMesh(writer.writeBoxMeshCsm, self.meshFunction, Lx)

        Lx = 1.1
        perturbedMesh = generatePerturbedMesh(writer.writeBoxMeshCsm, self.meshFunction, Lx, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)

    def test_SphereConnectivityDoesNotMatchInitialWhenRemeshed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeSphereMeshCsm, self.meshFunction, radius)

        radius = 2.1
        perturbedMesh = generatePerturbedMesh(writer.writeSphereMeshCsm, self.meshFunction, radius, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_SphereConnectivityMatchesInitialWhenMorphed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeSphereMeshCsm, self.meshFunction, radius)

        radius = 2.1
        perturbedMesh = generatePerturbedMesh(writer.writeSphereMeshCsm, self.meshFunction, radius, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)

    def test_CylinderConnectivityDoesNotMatchInitialWhenRemeshed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeCylinderMeshCsm, self.meshFunction, radius)

        radius = 2.1
        perturbedMesh = generatePerturbedMesh(writer.writeCylinderMeshCsm, self.meshFunction, radius, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_CylinderConnectivityMatchesInitialWhenMorphed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeCylinderMeshCsm, self.meshFunction, radius)

        radius = 2.1
        perturbedMesh = generatePerturbedMesh(writer.writeCylinderMeshCsm, self.meshFunction, radius, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)

    def test_ConeConnectivityDoesNotMatchInitialWhenRemeshed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeConeMeshCsm, self.meshFunction, radius)

        radius = 2.2
        perturbedMesh = generatePerturbedMesh(writer.writeConeMeshCsm, self.meshFunction, radius, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_ConeConnectivityMatchesInitialWhenMorphed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeConeMeshCsm, self.meshFunction, radius)

        radius = 2.2
        perturbedMesh = generatePerturbedMesh(writer.writeConeMeshCsm, self.meshFunction, radius, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)

    def test_ExtrudedCubeConnectivityDoesNotMatchInitialWhenRemeshed(self):
        length = 2.0
        initialMesh = generateInitialMesh(writer.writeExtrudedCubeMeshCsm, self.meshFunction, length)

        length = 2.5
        perturbedMesh = generatePerturbedMesh(writer.writeExtrudedCubeMeshCsm, self.meshFunction, length, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_ExtrudedCubeConnectivityMatchesInitialWhenMorphed(self):
        length = 2.0
        initialMesh = generateInitialMesh(writer.writeExtrudedCubeMeshCsm, self.meshFunction, length)

        length = 2.5
        perturbedMesh = generatePerturbedMesh(writer.writeExtrudedCubeMeshCsm, self.meshFunction, length, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)

    def test_CubeWithCylindricalHoleConnectivityDoesNotMatchInitialWhenRemeshed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeCubeWithCylindricalHoleMeshCsm, self.meshFunction, radius)

        radius = 2.1
        perturbedMesh = generatePerturbedMesh(writer.writeCubeWithCylindricalHoleMeshCsm, self.meshFunction, radius, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_CubeWithCylindricalHoleConnectivityMatchesInitialWhenMorphed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeCubeWithCylindricalHoleMeshCsm, self.meshFunction, radius)

        radius = 2.1
        perturbedMesh = generatePerturbedMesh(writer.writeCubeWithCylindricalHoleMeshCsm, self.meshFunction, radius, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)

    def test_PlusSignConnectivityDoesNotMatchInitialWhenRemeshed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writePlusSignMeshCsm, self.meshFunction, radius)

        radius = 2.1
        perturbedMesh = generatePerturbedMesh(writer.writePlusSignMeshCsm, self.meshFunction, radius, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_PlusSignConnectivityMatchesInitialWhenMorphed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writePlusSignMeshCsm, self.meshFunction, radius)

        radius = 2.1
        perturbedMesh = generatePerturbedMesh(writer.writePlusSignMeshCsm, self.meshFunction, radius, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)

class MeshMorphInAflr2Workflow(MeshMorphTester):
    def __init__(self, *args, **kwargs):
        super(MeshMorphInAflr2Workflow, self).__init__(*args, **kwargs)
        self.meshFunction = ESPtools.aflr2_meshing

    def test_SquareWithCircularCapSketchConnectivityDoesNotMatchInitialWhenRemeshed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeSquareWithCircularCapSketchMeshCsm, self.meshFunction, radius)

        radius = 2.5
        perturbedMesh = generatePerturbedMesh(writer.writeSquareWithCircularCapSketchMeshCsm, self.meshFunction, radius, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_SquareWithCircularCapSketchConnectivityMatchesInitialWhenMorphed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeSquareWithCircularCapSketchMeshCsm, self.meshFunction, radius)

        radius = 2.5
        perturbedMesh = generatePerturbedMesh(writer.writeSquareWithCircularCapSketchMeshCsm, self.meshFunction, radius, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)

    def test_SquareWithCircularHoleConnectivityDoesNotMatchInitialWhenRemeshed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeSquareWithCircularHoleMeshCsm, self.meshFunction, radius)

        radius = 2.5
        perturbedMesh = generatePerturbedMesh(writer.writeSquareWithCircularHoleMeshCsm, self.meshFunction, radius, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_SquareWithCircularHoleConnectivityMatchesInitialWhenMorphed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeSquareWithCircularHoleMeshCsm, self.meshFunction, radius)

        radius = 2.5
        perturbedMesh = generatePerturbedMesh(writer.writeSquareWithCircularHoleMeshCsm, self.meshFunction, radius, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)

    def test_SquareWithSuperellipseHoleConnectivityDoesNotMatchInitialWhenRemeshed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeSquareWithSuperellipseHoleMeshCsm, self.meshFunction, radius)

        radius = 2.5
        perturbedMesh = generatePerturbedMesh(writer.writeSquareWithSuperellipseHoleMeshCsm, self.meshFunction, radius, meshMorph=False)

        self.assertMeshesAreDifferent(initialMesh, perturbedMesh)

    def test_SquareWithSuperellipseHoleConnectivityMatchesInitialWhenMorphed(self):
        radius = 2.0
        initialMesh = generateInitialMesh(writer.writeSquareWithSuperellipseHoleMeshCsm, self.meshFunction, radius)

        radius = 2.5
        perturbedMesh = generatePerturbedMesh(writer.writeSquareWithSuperellipseHoleMeshCsm, self.meshFunction, radius, meshMorph=True)

        self.assertMeshesAreSame(initialMesh, perturbedMesh)



if __name__ == '__main__':
    unittest.main()
