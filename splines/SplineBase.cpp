#include "splines/SplineBase.h"


// *****************************************
SplineBase::SplineBase() {
// *****************************************


}


// *****************************************
SplineBase::~SplineBase() {
// *****************************************


}

// *****************************************
// Get the spline coefficients from the TSpline3 so that we can load ONLY these onto the GPU, not the whole TSpline3 object
// This loads up coefficients into two arrays: one x array and one yabcd array
// This should maximize our cache hits!
void SplineBase::getTF1Coeff(TF1_red* &spl, short &nPoints, float *& coeffs) {
// *****************************************

  // Initialise all arrays to 1.0
  for (ushort i = 0; i < nTF1Coeff; ++i) {
    coeffs[i] = 0.0;
  }

  // Get number of points in spline
  nPoints = short(spl->GetSize());

  if(nPoints > nTF1Coeff)
  {
    MACH3LOG_ERROR("Too big number of points for TF1");
    throw MaCh3Exception(__FILE__ , __LINE__ );
  }
  // TSpline3 can only take doubles, not floats
  // But our GPU is slow with doubles, so need to cast to float
  for (int i = 0; i < nPoints; i++) {
    coeffs[i] = float(spl->GetParameter(i));
  }
  // The structure is now coeffs  = {a,b,c,d,e}
}
