#include "pairing.h"



// explicit instantion of templated class method
template void qed::Pairing::solve<2>(pic::Tile<2>& );
template void qed::Pairing::solve<3>(pic::Tile<3>& );

template void qed::Pairing::comp_pmax<2>(string, float_p, std::map<std::string, pic::ParticleContainer<2>*>& cons);
template void qed::Pairing::comp_pmax<3>(string, float_p, std::map<std::string, pic::ParticleContainer<3>*>& cons);

template void qed::Pairing::solve_mc<2>(pic::Tile<2>& );
template void qed::Pairing::solve_mc<3>(pic::Tile<3>& );