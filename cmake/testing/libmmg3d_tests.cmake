## =============================================================================
##  This file is part of the mmg software package for the tetrahedral
##  mesh modification.
##  Copyright (c) Bx INP/Inria/UBordeaux/UPMC, 2004- .
##
##  mmg is free software: you can redistribute it and/or modify it
##  under the terms of the GNU Lesser General Public License as published
##  by the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  mmg is distributed in the hope that it will be useful, but WITHOUT
##  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
##  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
##  License for more details.
##
##  You should have received a copy of the GNU Lesser General Public
##  License and of the GNU General Public License along with mmg (in
##  files COPYING.LESSER and COPYING). If not, see
##  <http://www.gnu.org/licenses/>. Please read their terms carefully and
##  use this copy of the mmg distribution only if you accept them.
## =============================================================================

###############################################################################
#####
#####  Build executables for Mmg3d library Examples and add tests if needed
#####
###############################################################################

SET ( MMG3D_LIB_TESTS
  libmmg3d_example0_a
  libmmg3d_example0_b
  libmmg3d_example1
  libmmg3d_example2
  libmmg3d_example4
  libmmg3d_example6_io
  libmmg3d_lsOnly
  libmmg3d_lsAndMetric
  libmmg3d_generic_io
  )

# Additional tests that needs to download ci meshes
IF ( MMG3D_CI AND NOT ONLY_VERY_SHORT_TESTS )
  LIST ( APPEND MMG3D_LIB_TESTS
    test_api3d_0
    test_api3d_domain-selection
    test_api3d_vtk2mesh
    # Remark: not clean -> next tests don't need the library in fact (moving them
    # in app tests will ask to clean the installation of public and private
    # headers, it will ask to sort the needed source files too). Added here, we
    # can use the ADD_LIBRARY_TEST macro...
    test_compare-para-tria
    test_ridge-preservation-in-ls-mode
    )
ENDIF ( )

SET ( MMG3D_LIB_TESTS_MAIN_PATH
  ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/adaptation_example0/example0_a/main.c
  ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/adaptation_example0/example0_b/main.c
  ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/adaptation_example1/main.c
  ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/adaptation_example2/main.c
  ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/LagrangianMotion_example0/main.c
  ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_multisols_example6/main.c
  ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/main.c
  ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsAndMetric/main.c
  ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_generic_and_get_adja/genericIO.c
  )

# Additional library tests that needs to download ci meshes to be run
#
# Remark: as they are piece of code, it would be probably better to keep it in
# the mmg repository (so we have versionning). Other pieces of code
# (compare-para-tria and ridge-preservation-in-ls-mode) are hosted in the repo,
# I don't think that there is a reason for the difference in the choice of
# hosting...
IF ( MMG3D_CI AND NOT ONLY_VERY_SHORT_TESTS )
  LIST ( APPEND MMG3D_LIB_TESTS_MAIN_PATH
    ${MMG3D_CI_TESTS}/API_tests/3d.c
    ${MMG3D_CI_TESTS}/API_tests/domain-selection.c
    ${MMG3D_CI_TESTS}/API_tests/vtk2mesh.c
    # Following pieces of code are left in repo to take advantage of versionning
    ${PROJECT_SOURCE_DIR}/cmake/testing/code/compare-para-tria.c
    ${PROJECT_SOURCE_DIR}/cmake/testing/code/ridge-preservation-in-ls-mode.c
    )
ENDIF( )

IF ( LIBMMG3D_STATIC )
  SET ( lib_name lib${PROJECT_NAME}3d_a )
  SET ( lib_type "STATIC" )
ELSEIF ( LIBMMG3D_SHARED )
  SET ( lib_name lib${PROJECT_NAME}3d_so )
  SET ( lib_type "SHARED" )
ELSE ()
  MESSAGE(WARNING "You must activate the compilation of the static or"
    " shared ${PROJECT_NAME} library to compile this tests." )
ENDIF ( )

#####         Fortran Tests
IF ( CMAKE_Fortran_COMPILER )
  ENABLE_LANGUAGE ( Fortran )

  SET ( MMG3D_LIB_TESTS ${MMG3D_LIB_TESTS}
    libmmg3d_fortran_a
    libmmg3d_fortran_b
    libmmg3d_fortran_io
    libmmg3d_fortran_lsOnly
    libmmg3d_fortran_lsAndMetric
    )
  # Additional tests that needs to download ci meshes
  IF ( MMG3D_CI AND NOT ONLY_VERY_SHORT_TESTS )
    LIST ( APPEND MMG3D_LIB_TESTS test_api3d_fortran_0 )
  ENDIF( )

  SET ( MMG3D_LIB_TESTS_MAIN_PATH ${MMG3D_LIB_TESTS_MAIN_PATH}
    ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/adaptation_example0_fortran/example0_a/main.F90
    ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/adaptation_example0_fortran/example0_b/main.F90
    ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_multisols_example6/main.F90
    ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/main.F90
    ${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsAndMetric/main.F90
    )
  # Additional tests that needs to download ci meshes
  IF ( MMG3D_CI AND NOT ONLY_VERY_SHORT_TESTS )
    LIST ( APPEND MMG3D_LIB_TESTS_MAIN_PATH
      ${MMG3D_CI_TESTS}/API_tests/3d.F90
      )
  ENDIF( )

ENDIF ( CMAKE_Fortran_COMPILER )

LIST(LENGTH MMG3D_LIB_TESTS nbTests_tmp)
MATH(EXPR nbTests "${nbTests_tmp} - 1")

FOREACH ( test_idx RANGE ${nbTests} )
  LIST ( GET MMG3D_LIB_TESTS           ${test_idx} test_name )
  LIST ( GET MMG3D_LIB_TESTS_MAIN_PATH ${test_idx} main_path )

  ADD_LIBRARY_TEST ( ${test_name} ${main_path} copy_3d_headers ${lib_name} ${lib_type} )

ENDFOREACH ( )

SET ( src_test_met3d
  ${PROJECT_SOURCE_DIR}/src/common/bezier.c
  ${PROJECT_SOURCE_DIR}/src/common/eigenv.c
  ${PROJECT_SOURCE_DIR}/src/common/mettools.c
  ${PROJECT_SOURCE_DIR}/src/common/anisosiz.c
  ${PROJECT_SOURCE_DIR}/src/common/isosiz.c
  ${PROJECT_SOURCE_DIR}/src/common/tools.c
  ${PROJECT_SOURCE_DIR}/src/common/mmgexterns.c
  ${PROJECT_SOURCE_DIR}/cmake/testing/code/test_met3d.c
  )
ADD_LIBRARY_TEST ( test_met3d "${src_test_met3d}" copy_3d_headers ${lib_name} ${lib_type})
TARGET_LINK_LIBRARIES ( test_met3d PRIVATE ${M_LIB} )

SET(LIBMMG3D_EXEC0_a ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_example0_a)
SET(LIBMMG3D_EXEC0_b ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_example0_b)
SET(LIBMMG3D_EXEC1   ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_example1)
SET(LIBMMG3D_EXEC2   ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_example2)
SET(LIBMMG3D_EXEC4   ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_example4)
SET(LIBMMG3D_EXEC5   ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_example5)
SET(LIBMMG3D_EXEC6   ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_example6_io)
SET(LIBMMG3D_GENERICIO ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_generic_io)
SET(LIBMMG3D_LSONLY ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_lsOnly )
SET(LIBMMG3D_LSANDMETRIC ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_lsAndMetric )
SET(TEST_MET3D ${EXECUTABLE_OUTPUT_PATH}/test_met3d)

IF ( MMG3D_CI AND NOT ONLY_VERY_SHORT_TESTS )
  SET(TEST_API3D_EXEC0 ${EXECUTABLE_OUTPUT_PATH}/test_api3d_0)
  SET(TEST_API3D_DOMSEL ${EXECUTABLE_OUTPUT_PATH}/test_api3d_domain-selection)
  SET(TEST_API3D_VTK2MESH ${EXECUTABLE_OUTPUT_PATH}/test_api3d_vtk2mesh)
  SET(TEST_COMPARE_PARA_TRIA ${EXECUTABLE_OUTPUT_PATH}/test_compare-para-tria)
  SET(TEST_RIDGE_PRESERVATION_IN_LS_MODE ${EXECUTABLE_OUTPUT_PATH}/test_ridge-preservation-in-ls-mode)

ENDIF()

ADD_TEST(NAME libmmg3d_example0_a COMMAND ${LIBMMG3D_EXEC0_a}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/adaptation_example0/example0_a/cube.mesh"
  "${CTEST_OUTPUT_DIR}/libmmg3d_Adaptation_0_a-cube.o"
  )
ADD_TEST(NAME libmmg3d_example0_b COMMAND ${LIBMMG3D_EXEC0_b}
  "${CTEST_OUTPUT_DIR}/libmmg3d_Adaptation_0_b.o.mesh"
  )
ADD_TEST(NAME libmmg3d_example1   COMMAND ${LIBMMG3D_EXEC1}
  "${CTEST_OUTPUT_DIR}/libmmg3d_Adaptation_1.o.mesh"
  )
ADD_TEST(NAME libmmg3d_example2   COMMAND ${LIBMMG3D_EXEC2}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/adaptation_example2/2spheres.mesh"
  "${CTEST_OUTPUT_DIR}/libmmg3d_Adaptation_1-2spheres_1.o"
  "${CTEST_OUTPUT_DIR}/libmmg3d_Adaptation_1-2spheres_2.o"
  )
IF ( ELAS_FOUND AND NOT USE_ELAS MATCHES OFF )
  ADD_TEST(NAME libmmg3d_example4   COMMAND ${LIBMMG3D_EXEC4}
    "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/LagrangianMotion_example0/tinyBoxt"
    "${CTEST_OUTPUT_DIR}/libmmg3d_LagrangianMotion_0-tinyBoxt.o"
    )
ENDIF ()
ADD_TEST(NAME libmmg3d_example6_io_0   COMMAND ${LIBMMG3D_EXEC6}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_multisols_example6/torus.mesh"
  "${CTEST_OUTPUT_DIR}/libmmg3d_io_6-naca.o" "0"
  )
ADD_TEST(NAME libmmg3d_example6_io_1   COMMAND ${LIBMMG3D_EXEC6}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_multisols_example6/torus.mesh"
  "${CTEST_OUTPUT_DIR}/libmmg3d_io_6-naca.o" "1"
  )
ADD_TEST(NAME libmmg3d_lsOnly   COMMAND ${LIBMMG3D_LSONLY}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/plane.mesh"
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/m.sol"
  "${CTEST_OUTPUT_DIR}/libmmg3d_lsOnly_multimat.o"
  )
ADD_TEST(NAME libmmg3d_lsAndMetric   COMMAND ${LIBMMG3D_LSANDMETRIC}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/plane.mesh"
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/m.sol"
  "${CTEST_OUTPUT_DIR}/libmmg3d_lsAndMetric_multimat.o"
  )
ADD_TEST(NAME test_met3d   COMMAND ${TEST_MET3D}
  )

ADD_TEST(NAME libmmg3d_generic_io_msh   COMMAND ${LIBMMG3D_GENERICIO}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_generic_and_get_adja/cube.msh"
  "${CTEST_OUTPUT_DIR}/cube.o.msh" "1"
  )
ADD_TEST(NAME libmmg3d_generic_io_mesh   COMMAND ${LIBMMG3D_GENERICIO}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_generic_and_get_adja/cube.mesh"
  "${CTEST_OUTPUT_DIR}/cube.o.mesh" "1"
  )
ADD_TEST(NAME libmmg3d_generic_io_vtk   COMMAND ${LIBMMG3D_GENERICIO}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_generic_and_get_adja/cube.vtk"
  "${CTEST_OUTPUT_DIR}/cube.o.vtk" "1"
  )
ADD_TEST(NAME libmmg3d_generic_io_vtu   COMMAND ${LIBMMG3D_GENERICIO}
  "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_generic_and_get_adja/cube.vtu"
  "${CTEST_OUTPUT_DIR}/cube.o.vtu" "1"
  )

IF ( (NOT VTK_FOUND) OR USE_VTK MATCHES OFF )
  SET(expr "VTK library not founded")

  IF ( MMG3D_CI AND NOT ONLY_VERY_SHORT_TESTS )
    SET_PROPERTY(TEST test_api3d_vtk2mesh
      PROPERTY PASS_REGULAR_EXPRESSION "${expr}")
  ENDIF()
  SET_PROPERTY(TEST libmmg3d_generic_io_vtk
    PROPERTY PASS_REGULAR_EXPRESSION "${expr}")
  SET_PROPERTY(TEST libmmg3d_generic_io_vtu
    PROPERTY PASS_REGULAR_EXPRESSION "${expr}")
ENDIF ( )

IF ( MMG3D_CI AND NOT ONLY_VERY_SHORT_TESTS )
  ADD_TEST(NAME test_api3d_0   COMMAND ${TEST_API3D_EXEC0}
    "${MMG3D_CI_TESTS}/API_tests/2dom.mesh"
    "${CTEST_OUTPUT_DIR}/test_API3d.o"
    )
  ADD_TEST(NAME test_api3d_domain-selection COMMAND ${TEST_API3D_DOMSEL}
    "${MMG3D_CI_TESTS}/OptLs_plane/plane.mesh"
    "${MMG3D_CI_TESTS}/OptLs_plane/p.sol"
    "${CTEST_OUTPUT_DIR}/test_API3d-domsel-whole.o"
    "${CTEST_OUTPUT_DIR}/test_API3d-domsel-dom2.o"
    )
  ADD_TEST(NAME test_api3d_vtk2mesh   COMMAND ${TEST_API3D_VTK2MESH}
    "${MMG3D_CI_TESTS}/API_tests/cellsAndNode-data.vtk"
    "${CTEST_OUTPUT_DIR}/test_API3d-vtk2mesh.o"
    )
  ADD_TEST(NAME test_compare_para_tria
    COMMAND ${TEST_COMPARE_PARA_TRIA}
    ${MMG3D_CI_TESTS}/test_para_tria/proc0.mesh
    ${CTEST_OUTPUT_DIR}/proc0.o.mesh
    )
  SET_TESTS_PROPERTIES ( test_compare_para_tria
    PROPERTIES FIXTURES_REQUIRED test_para_tria )

  ADD_TEST(NAME test_ridge_preservation_in_ls_mode
    COMMAND ${TEST_RIDGE_PRESERVATION_IN_LS_MODE}
    ${MMG3D_CI_TESTS}/OptLs_NM_ridge/cube-it2.mesh
    ${CTEST_OUTPUT_DIR}/mmg3d_OptLs_NM_cube-it2.o.mesh
    )
  SET_TESTS_PROPERTIES ( test_ridge_preservation_in_ls_mode
    PROPERTIES FIXTURES_REQUIRED mmg3d_OptLs_NM_ridge )

ENDIF()


IF ( CMAKE_Fortran_COMPILER)
  SET(LIBMMG3D_EXECFORTRAN_a  ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_fortran_a)
  SET(LIBMMG3D_EXECFORTRAN_b  ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_fortran_b)
  SET(LIBMMG3D_EXECFORTRAN_IO ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_fortran_io)
  SET(LIBMMG3D_EXECFORTRAN_LSONLY ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_fortran_lsOnly )
  SET(LIBMMG3D_EXECFORTRAN_LSANDMETRIC ${EXECUTABLE_OUTPUT_PATH}/libmmg3d_fortran_lsAndMetric )

  IF ( MMG3D_CI AND NOT ONLY_VERY_SHORT_TESTS )
    SET(TEST_API3D_FORTRAN_EXEC0 ${EXECUTABLE_OUTPUT_PATH}/test_api3d_fortran_0)
  ENDIF()

  ADD_TEST(NAME libmmg3d_fortran_a  COMMAND ${LIBMMG3D_EXECFORTRAN_a}
    "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/adaptation_example0_fortran/example0_a/cube.mesh"
    "${CTEST_OUTPUT_DIR}/libmmg3d-Adaptation_Fortran_0_a-cube.o"
    )
  ADD_TEST(NAME libmmg3d_fortran_b  COMMAND ${LIBMMG3D_EXECFORTRAN_b}
    "${CTEST_OUTPUT_DIR}/libmmg3d-Adaptation_Fortran_0_b-cube.o"
    )
  ADD_TEST(NAME libmmg3d_fortran_io_0   COMMAND ${LIBMMG3D_EXECFORTRAN_IO}
    "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_multisols_example6/torus.mesh"
    "${CTEST_OUTPUT_DIR}/libmmg3d_Fortran_io-torus.o" "0"
    )
  ADD_TEST(NAME libmmg3d_fortran_io_1   COMMAND ${LIBMMG3D_EXECFORTRAN_IO}
    "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/io_multisols_example6/torus.mesh"
    "${CTEST_OUTPUT_DIR}/libmmg3d_Fortran_io-torus.o" "1"
    )
  ADD_TEST(NAME libmmg3d_fortran_lsOnly3d   COMMAND ${LIBMMG3D_EXECFORTRAN_LSONLY}
    "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/plane.mesh"
    "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/m.sol"
    "${CTEST_OUTPUT_DIR}/libmmg3d_lsOnly_multimat.o" )

  ADD_TEST(NAME libmmg3d_fortran_lsAndMetric3d   COMMAND ${LIBMMG3D_EXECFORTRAN_LSANDMETRIC}
    "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/plane.mesh"
    "${PROJECT_SOURCE_DIR}/libexamples/mmg3d/IsosurfDiscretization_lsOnly/m.sol"
    "${CTEST_OUTPUT_DIR}/libmmg3d_lsAndMetric_multimat.o" )

  IF ( MMG3D_CI AND NOT ONLY_VERY_SHORT_TESTS )
    ADD_TEST(NAME test_api3d_fortran_0   COMMAND ${TEST_API3D_FORTRAN_EXEC0}
      "${MMG3D_CI_TESTS}/API_tests/2dom.mesh"
      "${CTEST_OUTPUT_DIR}/test_API3d.o"
      )
  ENDIF()

ENDIF()
