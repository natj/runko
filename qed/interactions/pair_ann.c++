#include <iostream>
#include <cmath>
#include <cassert>
#include "pair_ann.h"
#include "../../tools/vector.h"


namespace qed {
  using std::string;
  using std::tuple;
  using std::sqrt;

  using toolbox::Vec3;
  using toolbox::Mat3;
  using toolbox::Vec4;
  using toolbox::Mat4;
  using toolbox::norm;
  using toolbox::dot;
  using toolbox::cross;
  using toolbox::unit_cross;
  using toolbox::sum;
  using toolbox::inv;


tuple<float_p, float_p> PairAnn::get_minmax_ene( string t1, string t2, double ene)
{
  return {0.0f, INF}; 
}


PairAnn::pair_float PairAnn::comp_cross_section(
    string t1, float_p ux1, float_p uy1, float_p uz1,
    string t2, float_p ux2, float_p uy2, float_p uz2)
{

  float_p zp = norm(ux1, uy1, uz1); // z_+
  float_p zm = norm(ux2, uy2, uz2); // z_-
  float_p gamp = sqrt(zp*zp + 1.0f); // \gamma_+
  float_p gamm = sqrt(zm*zm + 1.0f); // \gamma_-

  float_p zeta = ( ux1*ux2 + uy1*uy2 + uz1*uz2 )/( zm*zp ); //#angle between pair's momentum
  
  //qe = max(1+EPS, gamp*gamm - zp*zm*zeta) 
  float_p qe =  gamp*gamm - zp*zm*zeta; //# product of 4-moms; q_e = z_- . z_+; = 2gamma_cm^2 - 1 = gamma_r
  float_p q = qe + 1.0f;          //# q_e = q + 1
  float_p bcm = sqrt( (q - 2.0f)/q ); //# \beta_cm; matches beta' in Coppi & Blandford
  float_p gcm = 1.0f/sqrt(1.0f - bcm*bcm); //# gamma_cm

  //# expression via relativistic invariant x = sqrt( p_- . p_+ )
  float_p x = bcm;                  //# free variable
  //float_p s0 = (0.25f*(1.0f/x/x)*(1.0f-x*x))*( (3.0f-x*x*x*x)*log((1.0f+x)/(1.0f-x)) + 2.0f*x*(x*x - 2.0f)); //# ver2
  //s0 *= 3.0f/8.0f; //# normalization to rates; mathches with Coppi & Blandford eq 3.1

  //s0 *= 2.0; // FIXME ????

  //# ver2; Svensson 1992
  float_p s0 = (0.25f/(bcm*gcm*gcm))*( (1.0f/bcm)*(2.0f + 2.0f/gcm/gcm - 1.0f/pow(gcm,4))*log( (1.0f+bcm)/(1.0f-bcm) ) - 2.0f - 2.0f/gcm/gcm);
  s0 *= 3.0f/8.0f; //# normalization to rates; mathches with Coppi & Blandford eq 3.1
     
  //std::cout << " PAIR-ANN:" << s0 << " " << s1 << std::endl;

  //TODO why Riger notes show gamma-gamma peak at 3/8 sigT???

  //#--------------------------------------------------
  //# relative velocity calculation

  //# testing which one is correct.... and none of these are correct.
  //# 1 - ( beta_+ \beta_- cos(\theta) )
  //#vrel = 1 - (zp/gamp)*(zm/gamm)*zeta # seems most correct according to qed3 test results
  //#vrel = np.sqrt( (zp*zm*zeta)**2 - 1/(gamp*gamm)) # produces hi-ene peak; wrong
  //#vrel = (zp/gamp)*(zm/gamm)*zeta 
  //# NOTE: all of these are equal up to numerical precision; last one is roughly fastest

  //# fkin; eq 3.3 in Coppi & Blandford
  //#betam = zm/gamm
  //#betap = zp/gamp
  //#fkin1 = np.sqrt( betam**2 + betap**2 - (betam*betap)**2*(1-zeta**2) - 2*betam*betap*zeta )

  //# kinematic factor based on Svensson 82 Eq 9
  //#gamcm = 1/np.sqrt(1 - bcm**2) # gamma_cm
  //#fkin2 = 2*bcm*gamcm**2/(gamm*gamp)
  //#fkin2 = bcm*q/(gamm*gamp) # equal expression to above

  // kinematic factor based on Svensson 82 Eq 8; qe = q_+ . q_-
  float_p fkin = sqrt( qe*qe - 1.0 )/(gamp*gamm);
  //float_p fkin = 1.0;

  //if qe**2 -1 < 0: print('pair-ann cs:', s0, fkin, qe, gamp, gamm, zeta, x)
  return {s0, fkin};
}


//tuple<
//    string, float_p, float_p, float_p,
//    string, float_p, float_p, float_p>
//PairAnn::interact(
//  string t1, float_p ux1, float_p uy1, float_p uz1,
//  string t2, float_p ux2, float_p uy2, float_p uz2) 
//{
//
//  //      particle 1          particle 2
//  return {t3, ux3, uy3, uz3,  t4, ux4, uy4, uz4};
//}
  
void PairAnn::interact(
  string& t1, float_p& ux1, float_p& uy1, float_p& uz1,
  string& t2, float_p& ux2, float_p& uy2, float_p& uz2) 
{
  Vec3<float_p> zmvec(ux1, uy1, uz1);
  Vec3<float_p> zpvec(ux2, uy2, uz2);

  float_p zm = norm(zmvec); // electron momenta z_-
  float_p zp = norm(zpvec); // positron momenta z_+

  float_p gamm = sqrt(zm*zm + 1.0); // electron gamma_-
  float_p gamp = sqrt(zp*zp + 1.0); // positron gamma_+

  Vec3<float_p> omm = zmvec/zm; // direction of electron momenta Omega_-
  Vec3<float_p> omp = zpvec/zp; // direction of positron momenta Omega_+
        
  float_p zeta = dot(omm, omp); //#angle between electron and positron momenta

  float_p s0 = gamm + gamp;                        // s0 = x + x1 #sqrt(2q) in CoM
  float_p s  = sqrt(zm*zm + zp*zp + 2*zm*zp*zeta); // s  = sqrt(x**2  + x1**2 + 2*x*x1*mu)
  float_p q  = gamp*gamm - zp*zm*zeta + 1.0;       // q  = x*x1*(1-mu)
  // alternatively:  q = s0**2 - s**2

  //Vec3 svec = zp*omp + zm*omm;                 // svec = x*om + x1*om1
  Vec3<float_p> svec_a = zp*omp;                  // part 1
  Vec3<float_p> svec_b = zm*omm;                  // part 2
  Vec3<float_p> svec = svec_a + svec_b;

  //# CoM frame variables; x_c
  Vec3<float_p> bc = svec/(-1.0f*s0); //lorentz transform along CoM velocity vector
  float_p gc = s0/sqrt(2.0*q);       // lorentz factor of the boost; CoM vel
  Vec3<float_p> uv = gc*bc;          // com four vel
  float_p v2 = dot(bc,bc);           // v_c^2
  float_p vc = sqrt(v2);

  //--------------------------------------------------
  // boosted variables in CoM frame; x_cm 
  float_p gcm = sqrt(q/2.0); // # prtcl energies are equal in this frame; photons also have this energy
  float_p vcm = sqrt(1.0-1.0/(gcm*gcm)); //# v_cm

  //# angle between b_cm and b_c; electron and CoM 
  float_p y = (1.0/vc/vcm)*((gamp - gamm)/(gamp + gamm));

  //# build new coordinate system along svec
  Vec3<float_p> kvec = bc/norm(bc);            // z axis along b_c vector
  Vec3<float_p> jvec = unit_cross(kvec, omm);  // y axis to electron direction  
  Vec3<float_p> ivec = unit_cross(jvec, kvec); // x axis just orthogonal to others 
  Mat3<float_p> M(ivec, jvec, kvec);           // 3x3 rotation matrix


  //#--------------------------------------------------
  //# draw angles
  int niter = 0;

  float_p cosz, phi, xang, z1, z2, F; // temp variables
  while(true) {
    phi = 2.0*PI*rand(); // azimuthal symmetry angle
    cosz= -1.0 + 2.0*rand(); //# angle between k_cm and b_c; photon and CoM

    //# angle between k_cm and b_cm; photon and electron
    xang = y*cosz + sqrt(1.0 - y*y)*sqrt(1.0 - cosz*cosz)*cos(phi);

    //# four product scalar between electron/positron and primary/secondary photon
    z1 = (gcm*gcm)*(1.0 - vcm*xang);
    z2 = (gcm*gcm)*(1.0 + vcm*xang);

    //# differential cross section angle part; F function 
    F = 0.5*( (z1/z2) + (z2/z1) + 2.0*( (1.0/z1) + (1.0/z2) ) - pow( (1.0/z1) + (1.0/z2), 2)  );
    F *= 1.0/((1.0 + vcm)*gcm*gcm); //# normalize to [0,1]

    if( F > rand() ) break;   // accept angles
    if( niter > n_max ) break; // too many iterations
    niter += 1;

  }

  if(niter > n_max) std::cerr << "PAIR-ANN WARNING: too many iterations" << std::endl;


  //# new photon vectors in CoM frame
  float_p sinz = sqrt(1.0 - cosz*cosz); 
  Vec3<float_p> omrR( sinz*cos(phi), sinz*sin(phi), cosz );

  //# rotate back to lab frame angles
  Mat3<float_p> Minv = inv( M );
  Vec3<float_p> omr0 = dot( Minv, omrR );
  Vec3<float_p> omr1 = -1.0f*omr0; // other photon has same but opposite direction 


  //# boost matrix back to lab frame; constructed from b_c vector
  Vec4<float_p> B1( gc,       -uv(0),                    -uv(1),                    -uv(2)                 );
  Vec4<float_p> B2(-uv(0), 1.+(gc-1.)*bc(0)*bc(0)/v2,    (gc-1.)*bc(1)*bc(0)/v2,    (gc-1.)*bc(2)*bc(0)/v2 );
  Vec4<float_p> B3(-uv(1),    (gc-1.)*bc(0)*bc(1)/v2, 1.+(gc-1.)*bc(1)*bc(1)/v2,    (gc-1.)*bc(2)*bc(1)/v2 );
  Vec4<float_p> B4(-uv(2),    (gc-1.)*bc(0)*bc(2)/v2,    (gc-1.)*bc(1)*bc(2)/v2, 1.+(gc-1.)*bc(2)*bc(2)/v2 );
  Mat4<float_p> B(B1, B2, B3, B4);

  //# four momenta of photons
  Vec4<float_p> xp0(gcm, gcm*omr0(0), gcm*omr0(1), gcm*omr0(2));
  Vec4<float_p> xp1(gcm, gcm*omr1(0), gcm*omr1(1), gcm*omr1(2));

  //# boost 
  Vec4<float_p> xpp0 = dot(B, xp0);
  Vec4<float_p> xpp1 = dot(B, xp1);

  float_p x0 = xpp0(0); // energy of primary photon
  float_p x1 = xpp1(0); // energy of secondary photon

  Vec3<float_p> om0( xpp0(1)/x0, xpp0(2)/x0, xpp0(3)/x0 ); 
  Vec3<float_p> om1( xpp1(1)/x1, xpp1(2)/x1, xpp1(3)/x1 ); 

  //--------------------------------------------------
  // # test energy conservation # NOTE: we can remove these debug tests if needed
  if(true){
    float_p enec = gamm + gamp - (x0 + x1);
    Vec3<float_p> momc; // zmvec + zpvec; - (x*om + x1*om1);
    for(size_t i=0; i<3; i++) momc(i) = zmvec(i) + zpvec(i) - ( x0*om0(i) + x1*om1(i) );
    float_p moms = sum(momc);

    float_p nom1i = norm(omm);
    float_p nom2i = norm(omp);
    float_p nom1  = norm(om0);
    float_p nom2  = norm(om1);

    bool ts[8]; // t1,t2,t3,t4,t5,t6,t7,t8 = False,False,False,False,False,False,False,False
    for(size_t i=0; i<8; i++)    ts[i] = false; 

    if(abs(enec) >     tol) ts[0] = true; 
    if(x0 < 0.0)            ts[1] = true;
    if(x1 < 0.0)            ts[2] = true;
    if(abs(nom1i)-1. > tol) ts[3] = true;
    if(abs(nom2i)-1. > tol) ts[4] = true;
    if(abs(nom1)-1.  > tol) ts[5] = true;
    if(abs(nom2)-1.  > tol) ts[6] = true;
    if(abs(moms)     > tol) ts[7] = true;

    //if(true) {
    if(ts[0] ||
       ts[1] ||
       ts[2] ||
       ts[3] ||
       ts[4] ||
       ts[5] ||
       ts[6] ||
       ts[7] ) {
      std::cerr << "ERROR PAIR-ANN:" << std::endl;
      for(size_t i = 0; i < 8; i++) { std::cerr << i << " " << ts[i] << std::endl; }
    
      std::cerr << "zm, zp    " <<  zmvec << " " <<  zpvec << std::endl;
      std::cerr << "gm, gp    " <<  gamm  << " " <<  gamp  << std::endl;
      std::cerr << "z,s0,s,q  " <<  zeta  << " " <<  s0    << " " <<  s << " " << q  << std::endl;
      std::cerr << "x,x1,enec " <<  x0    << " " <<  x1    << " " <<  enec << std::endl;
      std::cerr << "|omi||omi|" <<  nom1i << " " <<  nom2i << std::endl;
      std::cerr << "|om|,|om1|" <<  nom1  << " " <<  nom2  << std::endl;
      std::cerr << "momc      " <<  moms  << " " <<  momc  << std::endl;
      std::cerr << "th,phi,om " <<  cosz  << " " <<  phi   << "om " << om0 << "om1 " <<  om1  << std::endl;
      std::cerr << "xpp0,xpp1 " <<  xpp0  << " " <<  xpp1  << std::endl;
      std::cerr << "B         " <<  B      <<                 std::endl;
      std::cerr << "omr0,omr1 " <<  omr0  << " " <<  omr1  << std::endl;
      std::cerr << "xp0, xp1  " <<  xp0   << " " <<  xp1   << std::endl;

      //assert(false);
    }
  }

  //# NOTE flip randomly; does not have an effect because photons are identical
  //# alternate the role of primary/secondary; otherwise algorithm is deterministic
  //#if np.random.rand() < 0.5:
  //#    x, x1  = x1,x
  //#    om,om1 = om1, om

  t1 = "ph";
  ux1 = xpp0(1);
  uy1 = xpp0(2);
  uz1 = xpp0(3);

  t2 = "ph";
  ux2 = xpp1(1);
  uy2 = xpp1(2);
  uz2 = xpp1(3);

  return;
}


} // end of namespace qed
