#pragma once

#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>

#include "Mesh.h"


namespace toolbox {


/* \brief write Mesh class into hdf5 file
 *
 * Actual data plus some metadata is written to the file.
 * This includes:
 *  original dimension Nx Ny Nz
 *  ??? Halo skipping length H
 *
 *
 */
template <class T, int H=0>
void write(Mesh<T, H>& mesh) {



}



template <class T, int H=0>
void read(Mesh<T, H>& mesh) {


}






}



