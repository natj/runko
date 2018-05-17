#pragma once

#include <string>

#include <omp.h>

#include "amr_momentum_solver.h"
#include "amr_spatial_solver.h"
#include "amr_analyzator.h"
#include "../io/io.h"



namespace vlasov{


void stepLocation( vlasov::Grid& grid )
{

#pragma omp parallel
  {
#pragma omp single
    {

      for(auto cid : grid.getCellIds() ){
#pragma omp task
        {
            
          vlasov::VlasovCell& cell 
            = dynamic_cast<vlasov::VlasovCell&>(grid.getCell( cid ));

          //vlasov::AmrSpatialLagrangianSolver<Realf> ssol;
          //ssol.solve(cell, grid);
            
          cell.stepLocation(grid);
        }// end of omp task
      }


    }// end of omp single
  }// end of omp parallel

}

template<int D>
void stepInitial( vlasov::Grid& grid )
{

#pragma omp parallel
  {
#pragma omp single
    {

      for(auto cid : grid.getCellIds() ){
#pragma omp task
        {
          vlasov::AmrMomentumLagrangianSolver<Realf,D> vsol;
          vlasov::VlasovCell& cell 
            = dynamic_cast<vlasov::VlasovCell&>(grid.getCell( cid ));
          vsol.solve(cell, -0.5);
        }// end of omp task
      }


    }// end of omp single
  }// end of omp parallel

}




template<int D>
void stepVelocity( vlasov::Grid& grid )
{

#pragma omp parallel
  {
#pragma omp single
    {

      for(auto cid : grid.getCellIds() ){
#pragma omp task
        {
          vlasov::AmrMomentumLagrangianSolver<Realf,D> vsol;
          vlasov::VlasovCell& cell 
            = dynamic_cast<vlasov::VlasovCell&>(grid.getCell( cid ));
          vsol.solve(cell);
        }// end of omp task
      }


    }// end of omp single
  }// end of omp parallel

}


/// Update Yee lattice boundaries
/*
void updateBoundaries()
{

  for(auto cid : getCellIds() ){
    vlasov::VlasovCell& cell = dynamic_cast<vlasov::VlasovCell& >(getCell( cid ));
    cell.updateBoundaries( *this );
  }

}
*/


void analyze( vlasov::Grid& grid )
{

#pragma omp parallel
  {
#pragma omp single
    {

      for(auto cid : grid.getCellIds() ){
#pragma omp task
        {
          vlasov::Analyzator<Realf> analyzator;
          vlasov::VlasovCell& cell 
            = dynamic_cast<vlasov::VlasovCell&>(grid.getCell( cid ));
          analyzator.analyze(cell);
        }// end of omp task
      }


    }// end of omp single
  }// end of omp parallel

}




void write( vlasov::Grid& grid )
{

  std::string prefix("fields_"); 
  prefix += std::to_string(grid.rank);
  h5io::Writer writer(prefix);


  for(auto cid : grid.getCellIds() ){
    fields::PlasmaCell& cell 
      = dynamic_cast<fields::PlasmaCell&>(grid.getCell( cid ));
    writer.write(cell);
  }

}




}// end of namespace
