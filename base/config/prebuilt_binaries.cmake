# Attempts to install prebuilt binaries from the plato-prebuilt-binaries repo.
# 
# This uses the variable PREBUILT_BINARY_DIR, and if it does not exist, it will
# check a path relative to the project path, which should be correct if this
# was cloned from the super-plato super project.
macro( install_prebuilt_binaries )

  include(FetchContent)
  if( NOT EXISTS ${PREBUILT_BINARY_DIR} )
    set(PREBUILT_BINARY_DIR  ${PROJECT_SOURCE_DIR}/../plato-prebuilt-binaries )
  endif()
  if( NOT EXISTS ${PREBUILT_BINARY_DIR} )
    message( WARNING "Cound not find plato-prebuilt-binaries directory, some integration tests may not function." )
  else()
    FetchContent_Declare(
      PlatoPrebuiltBinaries
      SOURCE_DIR ${PREBUILT_BINARY_DIR} )
    FetchContent_MakeAvailable(PlatoPrebuiltBinaries)
  endif()

endmacro()
