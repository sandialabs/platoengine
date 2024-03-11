class csmFileGenerator:
    def __init__(self, csmFileName):
        self.csmFileName = csmFileName
    
    def writeCsmFile(self, contents):
        csmFile = open(self.csmFileName, "w")
        csmFile.writelines(contents)
        csmFile.close()
    
    def appendAflr4Aflr3AIMData(self):
        return [
            "attribute capsAIM $aflr4AIM;aflr3AIM;platoAIM \n",
            "attribute capsMeshLength 5.0 \n",
            "select face \n",
            "attribute capsGroup   $solid_group \n",
            "patbeg i @stack.size \n",
            "  select body @stack[i] \n", 
            "  attribute _name $block_+val2str(i,0) \n",
            "patend \n",
            "end"]

    def appendAflr2AIMData(self):
        return [
            "select body \n",
            "attribute capsAIM $aflr2AIM;platoAIM \n",
            "attribute capsMeshLength 1.0 \n",
            "attribute capsGroup   $solid_group \n",
            "select edge \n",
            "attribute capsGroup $remaining_surface_sideset\n",
            "end"]

    def writeBoxMeshCsm(self, Lx):
        self.writeCsmFile([
            "despmtr Lx " + str(Lx) + " lbound 0.5 ubound 3.0 initial 1.0 \n",
            "conpmtr Ly 2.0 \n",
            "conpmtr Lz 1.5 \n",
            "box -Lx/2   -Ly/2   -Lz/2   Lx   Ly   Lz \n",
            "select body 1 \n"] +
            self.appendAflr4Aflr3AIMData()
        )

    def writeSphereMeshCsm(self, radius):
        self.writeCsmFile([
            "despmtr radius " + str(radius) + " lbound 1.0 ubound 3.0 initial 2.0 \n",
            "sphere 0   0   0   radius \n",
            "select body 1 \n"] +
            self.appendAflr4Aflr3AIMData()
        )

    def writeCylinderMeshCsm(self, radius):
        self.writeCsmFile([
            "despmtr radius " + str(radius) + " lbound 1.0 ubound 3.0 initial 2.0 \n",
            "cylinder 0   0   0   2   0   0   radius \n",
            "select body 1 \n"] +
            self.appendAflr4Aflr3AIMData()
        )

    def writeConeMeshCsm(self, radius):
        self.writeCsmFile([
            "despmtr radius " + str(radius) + " lbound 1.0 ubound 3.0 initial 2.0 \n",
            "cone 4   0   0   1   0   0   radius \n",
            "select body 1 \n"] +
            self.appendAflr4Aflr3AIMData()
        )

    def writeExtrudedCubeMeshCsm(self, length):
        self.writeCsmFile([
            "despmtr length " + str(length) + " lbound 1.0 ubound 3.0 initial 2.0 \n",
            "skbeg   -length/2   -length/2   0 \n",
	        "    linseg   -length/2   length/2   0 \n",
	        "    linseg   length/2   length/2   0 \n",
	        "    linseg   length/2   -length/2   0 \n",
	        "    linseg   -length/2   -length/2   0 \n",
            "skend     0 \n",
            "extrude 0 0 length \n",
            "select body 7 \n"] +
            self.appendAflr4Aflr3AIMData()
        )

    def writeCubeWithCylindricalHoleMeshCsm(self, radius):
        self.writeCsmFile([
            "conpmtr length 7.0 \n",
            "despmtr radius " + str(radius) + " lbound 1.0 ubound 3.0 initial 2.0 \n",
            "box -length/2   -length/2   -length/2   length   length   length \n",
            "cylinder 0   0   -length/2   0   0   length/2   radius \n",
            "subtract \n",
            "select body 3 \n"] + 
            self.appendAflr4Aflr3AIMData()
        )

    def writePlusSignMeshCsm(self, radius):
        self.writeCsmFile([
            "conpmtr length 5.0 \n",
            "despmtr radius " + str(radius) + " lbound 1.0 ubound 3.0 initial 2.0 \n",
            "cylinder 0   0   -length   0   0   length   radius \n",
            "cylinder -length   0   0   length   0   0   radius \n",
            "union \n",
            "select body 3 \n"] +
            self.appendAflr4Aflr3AIMData()
        )

    def writeSquareWithCircularCapSketchMeshCsm(self, radius):
        self.writeCsmFile([
            "conpmtr length 2.0 \n",
            "despmtr radius " + str(radius) + " lbound 1.0 ubound 3.0 initial 2.0 \n",
            "outpmtr MeshLength \n",
            "set MeshLength length/2.0 \n",
            "skbeg   -length/2   -length/2   0 \n",
	        "    linseg   -length/2   length/2   0 \n",
	        "    linseg   length/2   length/2   0 \n",
	        "    cirarc   length/2+radius   0   0   length/2   -length/2   0 \n",
	        "    linseg   -length/2   -length/2   0 \n",
            "skend     0 \n"] +
            self.appendAflr2AIMData()
        )

    def writeSquareWithCircularHoleMeshCsm(self, radius):
        self.writeCsmFile([
            "conpmtr length 7.0 \n",
            "despmtr radius " + str(radius) + " lbound 1.0 ubound 3.0 initial 2.0 \n",
            "outpmtr MeshLength \n",
            "set MeshLength length/2.0 \n",
            "box -length/2   -length/2   0   length length 0 \n",
            "set cyl_len 1 \n",
            "cylinder 0   0   -cyl_len/2   0   0   cyl_len/2   radius \n",
            "subtract \n",
            "select    body   3 \n",
            "select    edge   5 \n",
            "attribute capsMesh $circumference \n"] +
            self.appendAflr2AIMData()
        )

    def writeSquareWithSuperellipseHoleMeshCsm(self, radius):
        self.writeCsmFile([
            "conpmtr length 7.0 \n",
            "despmtr radius " + str(radius) + " lbound 1.0 ubound 3.0 initial 2.0 \n",
            "outpmtr MeshLength \n",
            "set MeshLength length/2.0 \n",
            "box -length/2   -length/2   0   length length 0 \n",
            "set cyl_len 1 \n",
            "udprim supell rx radius ry radius n 3.0 \n",
            "extrude 0 0 cyl_len \n",
            "subtract \n"] +
            self.appendAflr2AIMData()
        )