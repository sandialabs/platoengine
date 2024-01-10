import os
import sys
import fnmatch
import subprocess
import math
import pyCAPS
from shutil import copyfile
from pyCAPS import capsProblem
from contextlib import contextmanager

##############################################################################
## define functionality that redirects console output to a file
##############################################################################
@contextmanager
def redirected(to=os.devnull):
    '''
    import os

    with redirected(to=filename):
        print("from Python")
        os.system("echo non-Python applications are also supported")
    '''
    fdout = sys.stdout.fileno()
    fderr = sys.stderr.fileno()

    def _redirect_stdout(to):
        sys.stdout.close()                  # + implicit flush()
        os.dup2(to.fileno(), fdout)         # fd writes to 'to' file
        sys.stdout = os.fdopen(fdout, 'w')  # Python writes to fd

    def _redirect_stderr(to):
        sys.stderr.close()                  # + implicit flush()
        os.dup2(to.fileno(), fderr)         # fd writes to 'to' file
        sys.stderr = os.fdopen(fderr, 'w')  # Python writes to fd

    with os.fdopen(os.dup(fdout), 'w') as old_stdout, os.fdopen(os.dup(fderr), 'w') as old_stderr:
        with open(to, 'w') as file:
            _redirect_stdout(to=file)
            _redirect_stderr(to=file)
        try:
            yield # allow code to be run with the redirected stdout
        finally:
            _redirect_stdout(to=old_stdout) # restore stdout.
                                            # buffering and flags such as
                                            # CLOEXEC may be different
            _redirect_stderr(to=old_stderr) # restore stderr.
                                            # buffering and flags such as
                                            # CLOEXEC may be different


##############################################################################
## define function that compute tesselation data and body mesh
##############################################################################
def aflr(modelName, meshName, minScale=0.2, maxScale=1.0, meshLengthFactor=1.0, etoName=None):


  tokens = meshName.split('.')
  tokens.pop()
  meshBaseName = ".".join(tokens)

  # Initialize capsProblem object
  myProblem = capsProblem()

  # Load EGADS file
  filename = os.path.join(".", modelName)
  model = myProblem.loadCAPS(filename)

  # Load AFLR4 AIM
  aflr4 = myProblem.loadAIM(aim         = "aflr4AIM",
                            altName     = "aflr4",
                            analysisDir = ".")

  if etoName != None:
    # if tesselation object exists, remove it
    subprocess.call(['rm', etoName])

    tokens = etoName.split('.')
    tokens.pop()
    etoBaseName = ".".join(tokens)

    # Dump VTK files for visualization
    aflr4.setAnalysisVal("Proj_Name", etoBaseName)
    aflr4.setAnalysisVal("Mesh_Format", "ETO")

  # Scaling factor to compute AFLR4 'ref_len' parameter
  aflr4.setAnalysisVal("Mesh_Length_Factor", meshLengthFactor)
  aflr4.setAnalysisVal("min_scale", minScale)
  aflr4.setAnalysisVal("max_scale", maxScale)

  # Run AIM pre-/post-analysis
  aflr4.preAnalysis()
  aflr4.postAnalysis()

  # Load AFLR3 AIM to generate the volume mesh
  aflr3 = myProblem.loadAIM(aim         = "aflr3AIM",
                            analysisDir = ".",
                            parents     = aflr4.aimName)

  # Dump VTK files for visualization
  aflr3.setAnalysisVal("Proj_Name", meshBaseName)
  aflr3.setAnalysisVal("Mesh_Format", "SU2")

  # Run AIM pre-/post-analysis
  aflr3.preAnalysis()
  aflr3.postAnalysis()

  # Close CAPS
  myProblem.closeCAPS()


##############################################################################
## define function that extracts initial parameter values from csm file
##############################################################################
def getInitialValues(modelName):

  strVal = subprocess.check_output(['awk', '/despmtr/{print $0}', modelName]).decode(sys.stdout.encoding)
  params = strVal.split('\n')
  params = list(filter(None, params)) ## filter out empty strings

  if len(params) == 0:
      raise Exception("Parsing error: no keyword 'despmtr' found in file '" + modelName + "'" )

  initialValues = []
  for param in params:
    tokens = param.split(' ')
    tokens = list(filter(None, tokens)) ## filter out empty strings

    if len(tokens) != 9 and len(tokens) != 3:
      print("Expected line either in the form:")
      print("despmtr <name> <value> lbound <value> ubound <value> initial <value>")
      print("or in the form:")
      print("despmtr <name> <value>")
      print("got: " + param)
      raise Exception("Parsing error: reading initial values failed.")

    if len(tokens) == 9:
      ## eighth token should be 'initial'
      if tokens[7] != 'initial':
        raise Exception("Parsing error: expected 'initial' token, got '" + tokens[7] + "'" )

      ## get current value
      initialValue = float(tokens[8])

    elif len(tokens) == 3:

      ## get current value
      initialValue = float(tokens[2])

    initialValues.append(initialValue)
  
  return initialValues

##############################################################################
## define function that extracts current despmtr values from CAPS problem
##############################################################################
def getCurrentValues(problem):
  paramMap = problem.geometry.despmtr.items()
  if len(paramMap) == 0:
      raise Exception("Error: no 'despmtr' objects in the CAPS problem constructed with the given csm file." )
  
  params = []
  for pair in paramMap:
    params.append(pair[1].value)

  return params

##############################################################################
## define function that sets despmtr values in CAPS problem
##############################################################################
def setDesignParameterValues(problem, values):
  paramMap = problem.geometry.despmtr.items()

  if len(paramMap) != len(values):
      raise Exception("Error: Number of values provided does not equal the number of 'despmtr' objects in CAPS problem." )
  
  count = 0
  for pair in paramMap:
    pair[1].value = values[count]
    count += 1

##############################################################################
## check if 2 parameter sets are equal
##############################################################################
def parametersAreEqual(values1, values2):
  if len(values1) != len(values2):
    return False
  
  return all(math.isclose(val1, val2) for val1, val2 in zip(values1, values2))

##############################################################################
## set up and check if mesh morphing is needed
##############################################################################
def prepareGeometryForMeshMorph(modelName, problem, currentValues):
  initialValues = getInitialValues(modelName)
  if parametersAreEqual(initialValues, currentValues):
    return False
  else:
    setDesignParameterValues(problem, initialValues)
    return True

##############################################################################
## perform mesh morph
##############################################################################
def performMeshMorph(plato, problem, currentValues):
  plato.input["Mesh"].unlink()
  setDesignParameterValues(problem, currentValues)
  plato.preAnalysis()
  plato.postAnalysis()

##############################################################################
## define function that converts su2 mesh to exo mesh
##############################################################################
def toExo(meshName, groupAttrs):
  tokens = meshName.split('.')
  tokens.pop()
  baseDir = "./mesh/Scratch/aflr3AIM_aflr3AIM/"
  meshBaseName = ".".join(tokens)

  callArgs = ['Su2ToExodus', baseDir+meshBaseName+'.su2', meshBaseName+'.exo']

  # aflr writes faces w/o a capsGroup attribute to MARK 1
  callArgs.append("mark")
  callArgs.append("1")
  callArgs.append("sideset")
  callArgs.append("remaining_surface")

  for entry in groupAttrs:
    if entry["name"] != "solid_group":
      callArgs.append("mark")
      callArgs.append(entry["index"])
      if entry["name"].find("_nodeset") != -1:
        callArgs.append("nodeset")
      else :
        callArgs.append("sideset")
      callArgs.append(entry["name"])

  return_code = subprocess.call(callArgs)
  if return_code != 0:
    print("*******************************")
    print("ESP operation failed.. aborting")
    print("*******************************")
    exit(return_code)

##############################################################################
## define function that updates the "_opt.csm" file
##############################################################################
def updateModelAflr4Aflr3Exodus(modelName, paramVals):

  response = subprocess.check_output(['awk', '/set/{if ($2=="MeshLength") print $3}', modelName]).decode(sys.stdout.encoding)

  ## is 'MeshLength' in the csm file?
  if response == "":
    raise Exception("Error reading CSM file: required variable, 'MeshLength', not found..")

  ## is 'MeshLength' in the csm file only once?
  tokens = response.rstrip().split("\n")
  if len(tokens) > 1:
    raise Exception("Error reading CSM file: multiple 'MeshLength' keywords found. 'MeshLength' variable should appear once.")

  MeshLength = str(response)

  modedName = modelName + ".tmp"

  # Put global body attributes at the top of the file
  f_in = open(modelName)
  f_out = open(modedName, 'w')
  # put these two at the beginning
  f_out.write("attribute capsAIM $aflr4AIM;aflr3AIM;platoAIM\n")
  f_out.write("attribute capsMeshLength " + MeshLength + "\n")
  for line in f_in:
    if line.strip().lower() == 'end':
      # put this at the end to name bodies sequentially
      f_out.write("patbeg i @stack.size\n")
      f_out.write("  select body @stack[i]\n")
      f_out.write("  attribute _name $block_+val2str(i,0)\n")
      f_out.write("patend\n")
    f_out.write(line)
  f_out.close()

  # If paramVals were provided, set them in the model file
  for ip in range(len(paramVals)):
    p = paramVals[ip]
    print("param: " + str(p))
    tmp_string = modelName + '-tmp.file'
    f = open(tmp_string, "w")
    command = 'BEGIN{ip=0};{if($1~"despmtr"){if(ip=='+str(ip)+'){print $1, $2, val, $4, $5, $6, $7, $8, $9}else{print $0}ip++}else{print $0}}'
    print("command: ", command)
    subprocess.call(['awk', '-v', 'val='+str(paramVals[ip]), command, modedName], stdout=f)
    f.close()
    subprocess.call(['mv', tmp_string, modedName])

  subprocess.call(['mv', modedName, modelName])

##############################################################################
## define function that converts su2 mesh to exo mesh
##############################################################################
def updateModelXXXTetgenExodus(modelName, paramVals):

  modedName = modelName + ".tmp"

  subprocess.call(['cp', modelName, modedName])

  #
  # If paramVals were provided, set them in the model file
  #

  for ip in range(len(paramVals)):
    p = paramVals[ip]
    print("param: " + str(p))
    tmp_string = modelName + '-tmp.file'
    f = open(tmp_string, "w")
    command = 'BEGIN{ip=0};{if($1~"despmtr"){if(ip=='+str(ip)+'){print $1, $2, val, $4, $5, $6, $7, $8, $9}else{print $0}ip++}else{print $0}}'
    print("command: ", command)
    subprocess.call(['awk', '-v', 'val='+str(paramVals[ip]), command, modedName], stdout=f)
    f.close()
    subprocess.call(['mv', tmp_string, modedName])

  subprocess.call(['mv', modedName, modelName])

##############################################################################
## define function that sets up csm file for aflr2 and plato AIMS
##############################################################################
def updateModelAflr2Exodus(modelName, paramVals):

  # find mesh size attribute 'MeshLength'
  #
  response = subprocess.check_output(['awk', '/set/{if ($2=="MeshLength") print $3}', modelName]).decode(sys.stdout.encoding)
  if response == "":
    raise Exception("Error reading CSM file: required variable, 'MeshLength', not found..")

  tokens = response.rstrip().split("\n")
  if len(tokens) > 1:
    raise Exception("Error reading CSM file: multiple 'MeshLength' keywords found. 'MeshLength' variable should appear once.")

  MeshLength = str(response)

  # append necessary lines to csm file
  #
  modedName = modelName + ".tmp"

  f_in = open(modelName)
  f_out = open(modedName, 'w')

  bodyName = "solid_group"

  for line in f_in:
    if line.strip().lower() == 'end':
      f_out.write("select body\n")
      f_out.write("attribute capsAIM $aflr2AIM;platoAIM\n")
      f_out.write("attribute capsMeshLength 1.0 \n")
      f_out.write("attribute capsGroup $" + bodyName + "\n")
      f_out.write("attribute capsMesh $" + bodyName + "\n")

      f_out.write("select edge\n")
      f_out.write("attribute capsGroup $remaining_surface_sideset\n")

    f_out.write(line)

  f_out.close()

  # If paramVals were provided, set them in the model file
  #
  for ip in range(len(paramVals)):
    p = paramVals[ip]
    print("param: " + str(p))
    tmp_string = modelName + '-tmp.file'
    f = open(tmp_string, "w")
    command = 'BEGIN{ip=0};{if($1~"despmtr"){if(ip=='+str(ip)+'){print $1, $2, val, $4, $5, $6, $7, $8, $9}else{print $0}ip++}else{print $0}}'
    print("command: ", command)
    subprocess.call(['awk', '-v', 'val='+str(paramVals[ip]), command, modedName], stdout=f)
    f.close()
    subprocess.call(['mv', tmp_string, modedName])

  subprocess.call(['mv', modedName, modelName])

  # find any boundary attribute assignments and copy them to the end of the file
  #
  boundaryTag = "edge"

  f_in = open(modelName)
  f_out = open(modedName, 'w')

  boundaryAttrs = []

  bodyLine = ""
  boundaryLine = ""
  for line in f_in:
    if line.strip().lower() == 'end':
      for boundaryAttr in boundaryAttrs:
        for attrLine in boundaryAttr:
          f_out.write(attrLine)
    else:
      tokens = line.split(' ')
      tokens = list(filter(None, tokens)) ## filter out empty strings
      if bodyLine != "" and boundaryLine != "":
        if tokens[0] == "attribute" and tokens[1] == "capsGroup":
          boundaryAttrs.append([bodyLine, boundaryLine, line])
          bodyLine = ""
          boundaryLine = ""
      elif bodyLine != "":
        if tokens[0] == "select" and tokens[1] == boundaryTag:
          boundaryLine = line
        else:
          bodyLine = ""
      else:
        if tokens[0] == "select" and tokens[1] == "body":
          bodyLine = line

    f_out.write(line)

  f_out.close()

  subprocess.call(['mv', modedName, modelName])


##############################################################################
## Moves ESP generated mesh files to files used by plato
##############################################################################
def move_and_rename_plato_caps_eto_files(etoBaseName, nameForError):
  dirName = './ESP_Mesh/Scratch/plato/'
  platoCapsBaseName = 'plato_CAPS'

  def fullEtoFileName(baseName, index):
    return baseName + '_' + str(index) + '.eto'

  num_tess_files=0
  for file in os.listdir(dirName):
    if fnmatch.fnmatch(file, fullEtoFileName(platoCapsBaseName, '*')):
      num_tess_files += 1

  if num_tess_files == 0:
    raise Exception(f"Error in {nameForError}. No tessellation file was found in plato analysis directory.")

  if num_tess_files == 1:
    subprocess.call(['mv', dirName + fullEtoFileName(platoCapsBaseName, 1), './' + etoBaseName + '.eto'])
  else:
    cntr=0
    for file in sorted(os.listdir(dirName)):
      if fnmatch.fnmatch(file, fullEtoFileName(platoCapsBaseName, '*')):
        subprocess.call(['mv', dirName + file, './' + fullEtoFileName(etoBaseName, cntr)])
        cntr += 1
 
##############################################################################
## define function for running aflr4_aflr3 meshing workflow
##############################################################################
def aflr4_aflr3_meshing(modelNameOut, meshName, meshMorph, quiet=False):
  outLevel = 0 if quiet else 1
  problem = pyCAPS.Problem(problemName = "ESP_Mesh",
                     capsFile=modelNameOut,
                     outLevel=outLevel)

  if meshMorph:
    currentValues = getCurrentValues(problem)
    meshMorph = prepareGeometryForMeshMorph(modelNameOut, problem, currentValues)

  aflr4 = problem.analysis.create(aim='aflr4AIM', name='aflr4')

  aflr4.input.Mesh_Quiet_Flag = quiet
  aflr4.input.Mesh_Format = "ETO"
  aflr4.input.Mesh_Length_Factor = .2 
  aflr4.input.min_scale =  1
  aflr4.input.max_scale = 1
  aflr4.input.Multiple_Mesh = 'MultiDomain'
  aflr4.runAnalysis()

  aflr3 = problem.analysis.create(aim='aflr3AIM', name='aflr3')
  aflr3.input.Mesh_Quiet_Flag = quiet
  aflr3.input["Surface_Mesh"].link(aflr4.output["Surface_Mesh"])
  aflr3.input.Multiple_Mesh = 'MultiDomain'
  aflr3.runAnalysis()

  plato = problem.analysis.create(aim='platoAIM', name='plato')
  plato.input["Mesh"].link(aflr3.output["Volume_Mesh"])
  plato.input.Mesh_Morph = meshMorph

  plato.preAnalysis()
  plato.postAnalysis()

  if meshMorph:
    performMeshMorph(plato, problem, currentValues)

  tokens = meshName.split('.')
  tokens.pop()
  etoBaseName = '.'.join(tokens)
  subprocess.call(['cp', './ESP_Mesh/Scratch/plato/plato_CAPS.exo', meshName])

  move_and_rename_plato_caps_eto_files(etoBaseName, 'aflr4_aflr3_meshing')

##############################################################################
## define function for running egads_tetgen meshing workflow
##############################################################################
def egads_tetgen_meshing(modelNameOut, meshName, minScale, maxScale, meshLengthFactor, etoName):

  problem = pyCAPS.Problem(problemName = "ESP_Mesh",
                     capsFile=modelNameOut,
                     outLevel=1)
  surface = problem.analysis.create(aim='egadsTessAIM', name='egads')

  surface.input.Mesh_Length_Factor = 1.0

  face_sizes = problem.geometry.cfgpmtr.egadsFaceMeshSizes
  max_curvature_dists = problem.geometry.cfgpmtr.egadsMeshMaxCurvatureDistances
  max_dihedral_angles = problem.geometry.cfgpmtr.egadsMeshMaxDihedralAngles
  surf_mesh_sizing={}
  if hasattr(face_sizes, "__len__"): 
    for i in range(len(face_sizes)):
      temp={"tessParams":[face_sizes[i],max_curvature_dists[i],max_dihedral_angles[i]]}
      cur_name = "FaceSize" + str(i+1)
      surf_mesh_sizing[cur_name] = temp
  else:
    temp={"tessParams":[face_sizes,max_curvature_dists,max_dihedral_angles]}
    surf_mesh_sizing["FaceSize1"] = temp
  
  surface.input.Mesh_Sizing = surf_mesh_sizing
  volume = problem.analysis.create(aim='tetgenAIM', name='tetgen')
  volume.input["Surface_Mesh"].link(surface.output["Surface_Mesh"])

  volume.input.Multiple_Mesh = 'MultiDomain'
  # Equation for volume of perfectly formed tet
  adjusted_mesh_edge_length = .8*problem.geometry.cfgpmtr.egadsMaxTetEdgeSize
  mesh_max_tet_volume = adjusted_mesh_edge_length*adjusted_mesh_edge_length*adjusted_mesh_edge_length/8.485
  volume.input.Mesh_Gen_Input_String="a" + str(mesh_max_tet_volume) + "pYq1.100/0.000T1.00e-16A"
  volume.runAnalysis()
  plato = problem.analysis.create(aim='platoAIM', name='plato')
  plato.input["Mesh"].link(volume.output["Volume_Mesh"])
  plato.preAnalysis()
  plato.postAnalysis()

  tokens = meshName.split('.')
  tokens.pop()
  etoBaseName = '.'.join(tokens)
  subprocess.call(['cp', './ESP_Mesh/Scratch/plato/plato_CAPS.exo', meshName])

  move_and_rename_plato_caps_eto_files(etoBaseName, 'egads_tetgen_meshing')

##############################################################################
## define function for running aflr4_tetgen meshing workflow
##############################################################################
def aflr4_tetgen_meshing(modelNameOut, meshName, minScale, maxScale, meshLengthFactor, etoName):

  problem = capsProblem()
  model = problem.loadCAPS(modelNameOut)
  aflr4 = problem.loadAIM(aim         = "aflr4AIM",
                      altName     = "aflr4",
                      analysisDir = ".")

  #problem = pyCAPS.Problem(problemName = "ESP_Mesh",
  #                   capsFile=modelNameOut,
  #                   outLevel=1)
  #aflr4 = problem.analysis.create(aim='aflr4AIM', name='aflr4')
  #aflr4 = problem.loadAIM(aim         = "aflr4AIM",
  #                    altName     = "aflr4",
  #                    analysisDir = ".")
  aflr4.setAnalysisVal("Proj_Name", "ESP_Mesh")
  aflr4.setAnalysisVal("Mesh_Format", "ETO")
  aflr4.setAnalysisVal("Mesh_Length_Factor", .2)
  aflr4.setAnalysisVal("min_scale", 1)
  aflr4.setAnalysisVal("max_scale", 1)
  #aflr4.input.Mesh_Format = "ETO"
  #aflr4.input.Mesh_Length_Factor = 1 
  #aflr4.input.min_scale =  1
  #aflr4.input.max_scale = 1
  #aflr4.runAnalysis()
  aflr4.preAnalysis()
  aflr4.postAnalysis()
  volume = problem.analysis.create(aim='tetgenAIM', name='tetgen')
  volume.input["Surface_Mesh"].link(aflr4.output["Surface_Mesh"])
  volume.input.Multiple_Mesh = 'MultiDomain'
  #volume.input.Mesh_Gen_Input_String="a2.00e-4pYq1.500/0.000T1.00e-16A"
  # Equation for volume of perfectly formed tet
  #adjusted_mesh_edge_length = .8*mesh_edge_length
  adjusted_mesh_edge_length = .8*problem.geometry.cfgpmtr.egadsMaxTetEdgeSize
  mesh_max_tet_volume = adjusted_mesh_edge_length*adjusted_mesh_edge_length*adjusted_mesh_edge_length/8.485
  volume.input.Mesh_Gen_Input_String="a" + str(mesh_max_tet_volume) + "pYq1.100/0.000T1.00e-16A"
  volume.runAnalysis()
  plato = problem.analysis.create(aim='platoAIM', name='plato')
  plato.input["Mesh"].link(volume.output["Volume_Mesh"])
  plato.preAnalysis()
  plato.postAnalysis()

  tokens = meshName.split('.')
  tokens.pop()
  etoBaseName = '.'.join(tokens)
  subprocess.call(['cp', './ESP_Mesh/Scratch/tetgen/tetgen_0.exo', meshName])
  num_tess_files=0
  for file in os.listdir('./ESP_Mesh/Scratch/aflr4_aflr4AIM'):
    if fnmatch.fnmatch(file, 'aflr4_*.eto'):
      num_tess_files += 1

  if num_tess_files == 1:
    subprocess.call(['cp', './ESP_Mesh/Scratch/aflr4_aflr4AIM/' + file, './' + etoBaseName + '.eto'])
  else:
    cntr=0
    for file in sorted(os.listdir('./ESP_Mesh/Scratch/aflr4_aflr4AIM')):
      if fnmatch.fnmatch(file, 'aflr4_*.eto'):
        subprocess.call(['cp', './ESP_Mesh/Scratch/aflr4_aflr4AIM/' + file, './' + etoBaseName + "_" + str(cntr) + '.eto'])
        cntr += 1

##############################################################################
## define function for running aflr2 meshing workflow
##############################################################################
def aflr2_meshing(modelNameOut, meshName, meshMorph, quiet=False):
  outLevel = 0 if quiet else 1

  problem = pyCAPS.Problem(problemName = "ESP_Mesh",
                     capsFile=modelNameOut,
                     outLevel=outLevel)

  if meshMorph:
    currentValues = getCurrentValues(problem)
    meshMorph = prepareGeometryForMeshMorph(modelNameOut, problem, currentValues)

  aflr2 = problem.analysis.create(aim='aflr2AIM', name='aflr2')
  aflr2.input.Mesh_Quiet_Flag = quiet
  aflr2.input.Tess_Params = [problem.geometry.outpmtr.MeshLength, 1.0, 20.0]

  plato = problem.analysis.create(aim='platoAIM', name='plato')
  plato.input["Mesh"].link(aflr2.output["Area_Mesh"])
  plato.input.Mesh_Morph = meshMorph

  plato.preAnalysis()
  plato.postAnalysis()

  if meshMorph:
    performMeshMorph(plato, problem, currentValues)

  tokens = meshName.split('.')
  tokens.pop()
  etoBaseName = '.'.join(tokens)
  subprocess.call(['cp', './ESP_Mesh/Scratch/plato/plato_CAPS.exo', meshName])

  move_and_rename_plato_caps_eto_files(etoBaseName, 'aflr2_meshing')

##############################################################################
## define function that generates exodus mesh from csm file
##############################################################################
def mesh(modelNameIn, modelNameOut=None, meshName=None, minScale=0.2, maxScale=1.0, meshLengthFactor=1.0, etoName=None, mesh=True, geom=None, url=None, precision=8, workflow="aflr4_aflr3", meshMorph=False, parameters=None ):

  deleteOnExit = False
  if modelNameOut == None:
    modelNameOut = "work" + modelNameIn
    deleteOnExit = True

  if meshName == None:
    dot = '.'
    tokens = modelNameIn.split(dot)
    tokens.pop()
    meshName = dot.join(tokens) + ".exo"

  if type(minScale) == str:
    minScale = float(minScale)

  if type(maxScale) == str:
    maxScale = float(maxScale)

  if type(meshLengthFactor) == str:
    meshLengthFactor = float(meshLengthFactor)

  if type(mesh) == str:
    mesh = bool(mesh)

  if type(meshMorph) == str:
    meshMorph = bool(meshMorph)

  if type(precision) == str:
    precision = int(precision)

  paramVals = []
  precision_str = '{0:.' + str(precision - 1) + 'E}'
  if type(parameters) == str:
    paramVals = [float(precision_str.format(float(entry))) for entry in parameters.split(',')]
  else:
    paramVals = getInitialValues(modelNameIn)

  subprocess.call(['cp', modelNameIn, modelNameOut])

  if workflow == "aflr4_aflr3":
    updateModelAflr4Aflr3Exodus(modelNameOut, paramVals)
  elif workflow == "egads_tetgen":
    updateModelXXXTetgenExodus(modelNameOut, paramVals)
  elif workflow == "aflr4_tetgen":
    updateModelXXXTetgenExodus(modelNameOut, paramVals)
  elif workflow == "aflr2":
    updateModelAflr2Exodus(modelNameOut, paramVals)

  if geom != None:
    # with redirected('dump.console'):
    #   subprocess.call(['serveCSM', '-batch', modelNameOut])
    return_code = subprocess.call(['serveCSM', '-batch', modelNameOut])
    if return_code != 0:
      print("*******************************")
      print("ESP operation failed.. aborting")
      print("*******************************")
      exit(return_code)

  if mesh == True:
    if workflow == "aflr4_aflr3":
      with redirected('aflr4_aflr3.console'):
        aflr4_aflr3_meshing(modelNameOut, meshName, meshMorph)

    elif workflow == "egads_tetgen":
      with redirected('egads_tetgen.console'):
        egads_tetgen_meshing(modelNameOut, meshName, minScale, maxScale, meshLengthFactor, etoName)

    elif workflow == "aflr4_tetgen":
      with redirected('aflr4_tetgen.console'):
        aflr4_tetgen_meshing(modelNameOut, meshName, minScale, maxScale, meshLengthFactor, etoName)

    elif workflow == "aflr2":
      with redirected('aflr2.console'):
        aflr2_meshing(modelNameOut, meshName, meshMorph)

  if deleteOnExit:
    subprocess.call(['rm', modelNameOut])
  
