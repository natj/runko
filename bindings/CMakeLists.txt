project (pyrunko LANGUAGES CXX C)


#find_package(PYBIND11 REQUIRED) #loaded externally from corgi
#INCLUDE_DIRECTORIES ( "$ENV{pybind11_INCLUDE_DIR}" )

set (TOOLS_FILES 
     pytools.c++
     ../tools/hilbert.c++
     )

set (FIELDS_FILES 
     pyfields.c++
     ../em-fields/tile.c++ 
     ../em-fields/propagator/fdtd2.c++ 
     ../em-fields/propagator/fdtd2_pml.c++ 
     ../em-fields/propagator/fdtd4.c++ 
     ../em-fields/propagator/fdtd_general.c++ 
     ../em-fields/filters/binomial2.c++
     ../em-fields/filters/compensator.c++
     ../em-fields/filters/general_binomial.c++
     ../em-fields/filters/strided_binomial.c++
     #../em-fields/filters/sweeping_binomial.c++
     ../em-fields/boundaries/damping_tile.c++
     ../em-fields/boundaries/conductor.c++
     )

set (VLV_FILES 
     # pyvlv.c++
     # ../vlasov/tile.c++
     # ../vlasov/boundaries/piston.c++
     # ../vlasov/boundaries/outflow.c++
     # ../vlasov/momentum-solvers/amr_momentum_solver.c++
     # ../vlasov/momentum-solvers/bwd_lagrangian.c++
     )


set (PIC_FILES 
     pypic.c++
     ../pic/tile.c++
     ../pic/particle.c++
     ../pic/boundaries/wall.c++
     ../pic/boundaries/piston.c++
     ../pic/boundaries/piston_z.c++
     ../pic/boundaries/star_surface_injector.c++
     ../pic/pushers/boris.c++
     ../pic/pushers/boris_drag.c++
     ../pic/pushers/boris_rad.c++
     ../pic/pushers/boris_grav.c++
     ../pic/pushers/vay.c++
     ../pic/pushers/higuera_cary.c++
     ../pic/pushers/rgca.c++
     ../pic/pushers/photon.c++
     ../pic/pushers/pulsar.c++
     ../pic/interpolators/linear_1st.c++
     ../pic/interpolators/quadratic_2nd.c++
     ../pic/interpolators/cubic_3rd.c++
     ../pic/interpolators/quartic_4th.c++
     ../pic/depositers/zigzag.c++
     ../pic/depositers/zigzag_2nd.c++
     ../pic/depositers/zigzag_3rd.c++
     ../pic/depositers/zigzag_4th.c++
     ../pic/depositers/esikerpov_2nd.c++
     ../pic/depositers/esikerpov_4th.c++
     #../pic/depositers/esikerpov_4th_vec.c++
     )

set (QED_FILES 
     pyqed.c++
     ../qed/interactions/pair_ann.c++
     ../qed/interactions/phot_ann.c++
     ../qed/interactions/compton.c++
     ../qed/interactions/synchrotron.c++
     ../qed/interactions/multi_phot_ann.c++
     ../qed/pairing.c++
     )

set (FFE_FILES 
     pyffe.c++
     ../ffe/tile.c++
     ../ffe/skinny_yee.c++
     ../ffe/currents/rffe2.c++
     ../ffe/currents/rffe4.c++
     ../ffe/currents/ffe2.c++
     ../ffe/currents/ffe4.c++
     )



set (IO_FILES 
     ../io/writers/writer.c++
     ../io/readers/reader.c++
     ../io/snapshots/fields.c++
     ../io/snapshots/test_prtcls.c++
     ../io/snapshots/pic_moments.c++
     ../io/snapshots/field_slices.c++
     ../io/snapshots/master_only_fields.c++
     ../io/snapshots/master_only_moments.c++
    )

set (ACC_FILES 
    )


set (MISC_FILES 
     pycpl.c++
     ../coupling/tile.c++
    )


# python module creation
#--------------------------------------------------
# https://stackoverflow.com/questions/51907755/building-a-pybind11-module-with-cpp-and-cuda-sources-using-cmake
pybind11_add_module(pyrunko 
                    runko.c++ 
                    ${TOOLS_FILES}
                    ${FIELDS_FILES}
                    ${VLV_FILES}
                    ${PIC_FILES}
                    ${QED_FILES}
                    ${FFE_FILES}
                    ${IO_FILES}
                    ${ACC_FILES}
                    ${MISC_FILES}
                    )

#target_link_libraries(pyrunko PRIVATE -lhdf5)
target_link_libraries(pyrunko PRIVATE ${HDF5_C_LIBRARIES})
target_include_directories(pyrunko PRIVATE ${HDF5_INCLUDE_DIRS})

#target_link_libraries(pyrunko PRIVATE -lfftw3)
#target_link_libraries(pyrunko PRIVATE -lfftw3f)
target_link_libraries(pyrunko PUBLIC coverage_config)

include(CheckLanguage)

option(ENABLE_CUDA "Enable CUDA" OFF)
if(ENABLE_CUDA)
     check_language(CUDA REQUIRED)
     enable_language(CUDA)

     find_package(CUDA 11.0 REQUIRED)
     target_link_libraries(pyrunko PRIVATE ${CUDA_LIBRARIES})
endif()



target_compile_options(pyrunko PRIVATE ${WARNING_FLAGS})

if(ENABLE_CUDA)
     
     set_target_properties(pyrunko PROPERTIES CUDA_SEPERABLE_COMPILATION ON)
     set_source_files_properties(../ffe/currents/rffe.c++ PROPERTIES LANGUAGE CUDA)
     set_source_files_properties(../em-fields/tile.c++ PROPERTIES LANGUAGE CUDA)
     set_source_files_properties(../em-fields/filters/binomial2.c++ PROPERTIES LANGUAGE CUDA)
     set_source_files_properties(../em-fields/propagator/fdtd2.c++ PROPERTIES LANGUAGE CUDA)
     set_source_files_properties(../pic/interpolators/linear.c++ PROPERTIES LANGUAGE CUDA)
     set_source_files_properties(../pic/pushers/boris.c++ PROPERTIES LANGUAGE CUDA)
     set_source_files_properties(../pic/depositers/zigzag.c++ PROPERTIES LANGUAGE CUDA)

     #TODO this will give wrong results if the file passed to nvcc 
      set_source_files_properties(../pic/particle.c++ PROPERTIES LANGUAGE CUDA)
     target_compile_definitions(pyrunko PUBLIC GPU)

     set_property(TARGET pyrunko PROPERTY CUDA_STANDARD 17)

     # need extended lambda support to be able to pass lambdas to interation functions
     set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --extended-lambda --expt-relaxed-constexpr -std=c++17")
     message( ${CMAKE_CUDA_FLAGS} )
endif()


# installation
install (TARGETS pyrunko DESTINATION lib)
