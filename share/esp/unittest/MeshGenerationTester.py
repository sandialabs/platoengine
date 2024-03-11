import unittest

class MeshGenerationTester(unittest.TestCase):
    def assertMeshesAreDifferent(self, mesh1, mesh2):
        self.assertNotEqual(mesh1.numNodes, mesh2.numNodes)
        self.assertNotEqual(mesh1.numElements, mesh2.numElements)
        self.assertNotEqual(mesh1.elementBlocks[0].connectivity, mesh2.elementBlocks[0].connectivity)

    def assertMeshesAreSame(self, mesh1, mesh2):
        self.assertEqual(mesh1.numNodes, mesh2.numNodes)
        self.assertEqual(mesh1.numElements, mesh2.numElements)
        self.assertEqual(mesh1.elementBlocks[0].connectivity, mesh2.elementBlocks[0].connectivity)
