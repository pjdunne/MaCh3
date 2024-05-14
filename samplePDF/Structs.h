#pragma once

/// Run low or high memory versions of structs
/// N.B. for 64 bit systems sizeof(float) == sizeof(double) so not a huge effect
/// KS: Need more testing on FD
#ifdef _LOW_MEMORY_STRUCTS_
#define _float_ float
#define _int_ short int
#define _unsigned_int_ unsigned short int
#else
#define _float_ double
#define _int_ int
#define _unsigned_int_ unsigned int
#endif

/// KS: noexcept can help with performance but is terrible for debugging, this is meant to help easy way of of turning it on or off. In near future move this to struct or other central class. Keep it in ND for the time being
//#define SafeException
#ifndef SafeException
#define _noexcept_ noexcept
#else
#define _noexcept_
#endif

/// KS: Using restrict limits the effects of pointer aliasing, aiding optimizations. While reading I found that there might be some compilers which don't have __restrict__. As always we use _restrict_ to more easily turn off restrict in the code
#ifndef DEBUG
#define _restrict_ __restrict__
#else
#define _restrict_
#endif

/// Number of overflow bins in TH2Poly,
#define _TH2PolyOverflowBins_ 9

/// Include some healthy defines for constructors
#define _BAD_DOUBLE_ -999.99
#define _BAD_INT_ -999

#define _DEFAULT_RETURN_VAL_ -999999.123456

// C++ includes
#include <sstream>
#include <fstream> 
#include <iostream>
#include <vector>
#include <iomanip>

// ROOT include
#include "set"
#include "TSpline.h"
#include "TF1.h"
#include "TLorentzVector.h"
#include "TObjString.h"
#include "unordered_map"
#include "TH2Poly.h"
#include "list"
#include "TFile.h"

#ifdef MULTITHREAD
#include "omp.h"
#endif

// *******************
/// @brief Template to make vector out of an array of any length
template< typename T, size_t N >
std::vector<T> MakeVector( const T (&data)[N] ) {
// *******************
  return std::vector<T>(data, data+N);
}

// *******************
/// KS: This is mad way of converting string to int. Why? To be able to use string with switch
constexpr unsigned int str2int(const char* str, int h = 0) {
// *******************
  return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

// *******************
/// ETA - Normalisations for cross-section parameters
/// Carrier for whether you want to apply a systematic to an event or not
class XsecNorms4 {
  // *******************
  public:
    /// Bins for normalisation parameter
    TAxis *ebins;
    /// Name of parameters
    std::string name;
    /// Mode which parameter applies to
    std::vector<int> modes;
    /// Horn currents which parameter applies to
    std::vector<int> horncurrents;
    /// PDG which parameter applies to
    std::vector<int> pdgs;
    /// Preosc PDG which parameter applies to
    std::vector<int> preoscpdgs;
    /// Targets which parameter applies to
    std::vector<int> targets;
    /// Does this parameter have kinematic bounds
    bool hasKinBounds;
	/// Generic vector contain enum relating to a kinematic variable
    /// and lower and upper bounds. This can then be passed to IsEventSelected
	std::vector< std::vector<double> > Selection;
	
	/// Generic vector containing the string of kinematic type
	/// This then needs to be converted to a kinematic type enum
	/// within a samplePDF daughter class
	/// The bounds for each kinematic variable are given in Selection
	std::vector< std::string > KinematicVarStr;
	
    /// Parameter number of this normalisation in current systematic model
    int index;
};


// ************************
/// CW: A reduced TF1 class only
/// Only saves parameters for each TF1 and how many parameters each parameter set has
class TF1_red {
// ************************
  public:
    /// Empty constructor
    TF1_red() {
      length = 0;
      Par = NULL;
    }

    /// Empty destructor
    ~TF1_red() {
      if (Par != NULL) {
        delete[] Par;
        Par = NULL;
      }
    }

    /// The useful constructor with deep copy
    TF1_red(_int_ nSize, _float_* Array, _int_ Parameter) {
      length = nSize;
      for (int i = 0; i < length; ++i) {
        Par[i] = Array[i];
      }
      ParamNo = Parameter;
    }

    /// The TF1 constructor with deep copy
    TF1_red(TF1* &Function, int Param = -1) {
      Par = NULL;
      SetFunc(Function, Param);
    }

    // Get the number
    inline std::string GetName() {
      std::stringstream ss;
      ss << ParamNo;
      return ss.str();
    }

    /// Set the function
    inline void SetFunc(TF1* &Func, int Param = -1) {
      length = Func->GetNpar();
      if (Par != NULL) delete[] Par;
      Par = new _float_[length];
      for (int i = 0; i < length; ++i) {
        Par[i] = Func->GetParameter(i);
      }
      ParamNo = Param;
      delete Func;
      Func = NULL;
    }

    /// Evaluate a variation
    inline double Eval(_float_ var) {
      /// If we have 5 parameters we're using a fifth order polynomial
      if (length == 5) {
        return 1+Par[0]*var+Par[1]*var*var+Par[2]*var*var*var+Par[3]*var*var*var*var+Par[4]*var*var*var*var*var;
      /// If we have 2 parameters we're using two linear equations
      } else if (length == 2) {
        return (var<=0)*(1+Par[0]*var)+(var>0)*(1+Par[1]*var);
      } else {
        std::cerr << "*** Error in reduced TF1 class!" << std::endl;
        std::cerr << "    Class only knows about 5th order polynomial and two superposed linear function" << std::endl;
        std::cerr << "    You have tried something else than this, which remains unimplemented" << std::endl;
        std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        throw;
      }
    }

    /// Set a parameter to a value
    inline void SetParameter(_int_ Parameter, _float_ Value) {
      Par[Parameter] = Value;
    }

    /// Get a parameter value
    double GetParameter(_int_ Parameter) {
      if (Parameter > length) {
        std::cerr << "Error: you requested parameter number " << Parameter << " but length is " << length << " parameters" << std::endl;
        throw;
        return -999.999;
      }
      return Par[Parameter];
    }

    /// Set the size
    inline void SetSize(_int_ nSpline) {
      length = nSpline;
      Par = new _float_[length];
    }
    /// Get the size
    inline int GetSize() { return length; }
    inline void Print() {
      std::cout << "Printing TF1_red: " << std::endl;
      std::cout << "  ParamNo = " << ParamNo << std::endl;
      std::cout << "  Length  = " << length << std::endl;
      std::cout << "  a       = " << Par[0] << std::endl;
      std::cout << "  b       = " << Par[1] << std::endl;
      if (length == 5) {
        std::cout << "  c       = " << Par[2] << std::endl;
        std::cout << "  d       = " << Par[3] << std::endl;
        std::cout << "  e       = " << Par[4] << std::endl;
      }
    }

  private:
    /// The parameters
    _float_* Par;
    _int_ length;
    /// Save the parameter number this spline applies to
    _int_ ParamNo;
};

/// Make an enum of the spline interpolation type
enum SplineInterpolation {
  kTSpline3,
  kLinear,
  kMonotonic,
  kAkima,
  kSplineInterpolations  //This only enumerates
};

// **************************************************
/// Convert a LLH type to a string
inline std::string SplineInterpolation_ToString(SplineInterpolation i) {
// **************************************************
  std::string name = "";
  switch(i) {
    //  TSpline3 (third order spline in ROOT)
    case kTSpline3:
    name = "TSpline3";
    break;
    case kLinear:
    name = "Linear";
    break;
    case kMonotonic:
    name = "Monotonic";
    break;
    //  (Experimental) Akima_Spline (crd order spline which is allowed to be discontinuous in 2nd deriv)
    case kAkima:
    name = "Akima";
    break;
    default:
      std::cerr << "UNKNOWN SPLINE INTERPOLATION SPECIFIED!" << std::endl;
      std::cerr << "You gave  " << i << std::endl;
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      throw;
  }
  return name;
}


// ************************
/// CW: Reduced TSpline3 class
class TSpline3_red {
// ************************
  public:
    /// @brief Empty constructor
    TSpline3_red() {
      nPoints = 0;
      Par = NULL;
      XPos = NULL;
      YResp = NULL;
    }

    /// @brief The constructor that takes a TSpline3 pointer and copies in to memory
    TSpline3_red(TSpline3* &spline, int Param = -1, SplineInterpolation InterPolation = kTSpline3) {
      Par = NULL;
      XPos = NULL;
      YResp = NULL;
      SetFunc(spline, Param, InterPolation);
    }

    /// @brief constructor taking parameters
    TSpline3_red(_float_ *X, _float_ *Y, _int_ N, _float_ **P, _int_ parNo){
      nPoints = N;
      ParamNo = parNo;
      // std::cout<<"nPoints: "<<nPoints<<std::endl;
      // Save the parameters for each knot
      Par = new _float_*[nPoints];
      // Save the positions of the knots
      XPos = new _float_[nPoints];
      // Save the y response at each knot
      YResp = new _float_[nPoints];
      for(int j=0; j<N; ++j){
        Par[j] = new _float_[3];
        Par[j][0] = P[j][0];
        Par[j][1] = P[j][1];
        Par[j][2] = P[j][2];
        XPos[j] = X[j];
        YResp[j] = Y[j];

        if((Par[j][0] == -999) | (Par[j][1] ==-999) | (Par[j][2] ==-999) | (XPos[j] ==-999) | (YResp[j] ==-999)){
          std::cerr<<"******************* Bad parameter values when construction TSpline3_red *********************" <<std::endl;
          std::cerr<<"passed val (i, x, y, b, c, d): "<<j<<", "<<X[j]<<", "<<Y[j]<<", "<<P[j][0]<<", "<<P[j][1]<<", "<<P[j][2]<<std::endl;
          std::cerr<<"set values (i, x, y, b, c, d): "<<j<<", "<<XPos[j]<<", "<<YResp[j]<<", "<<Par[j][0]<<", "<<Par[j][1]<<", "<<Par[j][2]<<std::endl;
          std::cerr<<"*********************************************************************************************" <<std::endl;
        }
      }
    }

    inline void SetFunc(TSpline3* &spline, int Param = -1, SplineInterpolation InterPolation = kTSpline3) {
      nPoints = spline->GetNp();
      ParamNo = Param;
      if (Par != NULL) {
        for (int i = 0; i < nPoints; ++i) {
          delete[] Par[i];
          Par[i] = NULL;
        }
        delete[] Par;
        Par = NULL;
      }
      if (XPos != NULL) delete[] XPos;
      if (YResp != NULL) delete[] YResp;
      // Save the parameters for each knot
      Par = new _float_*[nPoints];
      // Save the positions of the knots
      XPos = new _float_[nPoints];
      // Save the y response at each knot
      YResp = new _float_[nPoints];

      //KS: Default TSpline3 ROOT implementation
      if(InterPolation == kTSpline3)
      {
        for (int i = 0; i < nPoints; ++i) {
          // 3 is the size of the TSpline3 coefficients
          Par[i] = new _float_[3];
          double x = -999.99, y = -999.99, b = -999.99, c = -999.99, d = -999.99;
          spline->GetCoeff(i, x, y, b, c, d);
          XPos[i]   = x;
          YResp[i]  = y;
          Par[i][0] = b;
          Par[i][1] = c;
          Par[i][2] = d;
        }
      }
      //CW: Reduce to use linear spline interpolation for certain parameters
      // Not the most elegant way: use TSpline3 object but set coefficients to zero and recalculate spline points; the smart way (but more human intensive) would be to save memory here and simply not store the zeros at all
      // Get which parameters should be linear from the fit manager
      // Convert the spline number to global xsec parameter
      // Loop over the splines points
      else if(InterPolation == kLinear)
      {
        for (int k = 0; k < nPoints; ++k) {
          // 3 is the size of the TSpline3 coefficients
          Par[k] = new _float_[3];
          Double_t x1, y1, b1, c1, d1, x2, y2, b2, c2, d2 = 0;
          spline->GetCoeff(k, x1, y1, b1, c1, d1);
          spline->GetCoeff(k+1, x2, y2, b2, c2, d2);
          double tempb = (y2-y1)/(x2-x1);

          XPos[k]   = x1;
          YResp[k]  = y1;
          Par[k][0] = tempb;
          Par[k][1] = 0;
          Par[k][2] = 0;
        }
      }
      //EM: Akima spline is similar to regular cubic spline but is allowed to be discontinuous in 2nd derivative and coefficients in any segment
      // only depend on th 2 nearest points on either side
      else if(InterPolation == kAkima)
      {
        // get the knot values for the spline
        for (int i = 0; i < nPoints; ++i) {
          // 3 is the size of the TSpline3 coefficients
          Par[i] = new _float_[3];

          double x = -999.99, y = -999.99;
          spline->GetKnot(i, x, y);

          XPos[i]   = x;
          YResp[i]  = y;
        }

        _float_* mvals = new _float_[nPoints + 3];
        _float_* svals = new _float_[nPoints + 1];

        for (int i = -2; i <= nPoints; ++i) {
          // if segment is first or last or 2nd to first or last, needs to be dealt with slightly differently;
          // need to estimate the values for additinal points which would lie outside of the spline
          if(i ==-2){
            mvals[i+2] = 3.0 * (YResp[1] - YResp[0]) / (XPos[1] - XPos[0]) - 2.0*(YResp[2] - YResp[1]) / (XPos[2] - XPos[1]);
          }
          else if(i==-1){
            mvals[i+2] = 2.0 * (YResp[1] - YResp[0]) / (XPos[1] - XPos[0]) - (YResp[2] - YResp[1]) / (XPos[2] - XPos[1]);
          }
          else if(i==nPoints){
            mvals[i+2] = 3.0 * (YResp[nPoints-1] - YResp[nPoints-2]) / (XPos[nPoints-1] - XPos[nPoints-2]) - 2.0*(YResp[nPoints-2] - YResp[nPoints-3]) / (XPos[nPoints-2] - XPos[nPoints-3]);
          }
          else if(i == nPoints - 1){
            mvals[i+2] = 2.0 * (YResp[nPoints-1] - YResp[nPoints-2]) / (XPos[nPoints-1] - XPos[nPoints-2]) - (YResp[nPoints-2] - YResp[nPoints-3]) / (XPos[nPoints-2] - XPos[nPoints-3]);
          }
          //standard internal segment
          else{
            mvals[i+2] = (YResp[i+1] - YResp[i])/ (XPos[i+1] - XPos[i]);
          }
        }

        for(int i =2; i<=nPoints+2; i++){
          if (abs(mvals[i+1] - mvals[i]) + abs(mvals[i-1] - mvals[i-2]) != 0.0){
            svals[i-2] = (abs(mvals[i+1] - mvals[i]) * mvals[i-1] + abs(mvals[i-1] - mvals[i-2]) *mvals[i]) / (abs(mvals[i+1] - mvals[i]) + abs(mvals[i-1] - mvals[i-2]));
            }
          else{svals[i-2] = mvals[i];}
        }

        // calculate the coefficients for the spline
        for(int i = 0; i <nPoints; i++){
          _float_ b, c, d = -999.999;

          b = svals[i];
          c = (3.0* (YResp[i+1] - YResp[i]) / (XPos[i+1] - XPos[i]) -2.0 *svals[i] - svals[i +1]) /(XPos[i+1] - XPos[i]);
          d = ((svals[i + 1] +svals[i]) - 2.0*(YResp[i+1] - YResp[i]) / (XPos[i+1] - XPos[i])) / ((XPos[i+1] - XPos[i]) * (XPos[i+1] - XPos[i]));

          Par[i][0] = b;
          Par[i][1] = c;
          Par[i][2] = d;
        }

        // check the input spline for linear segments, if there are any then overwrite the calculated coefficients
        // this will pretty much only ever be the case if they are set to be linear in samplePDFND i.e. the user wants it to be linear
        for(int i = 0; i <nPoints-1; i++){
          double x = -999.99, y = -999.99, b = -999.99, c = -999.99, d = -999.99;
          spline->GetCoeff(i, x, y, b, c, d);

          if((c == 0.0 && d == 0.0)){
            Par[i][0] = b;
            Par[i][1] = 0.0;
            Par[i][2] = 0.0;
          }
        }
        delete[] mvals;
        delete[] svals;
      }
      //EM: Monotone spline is similar to regular cubic spline but enforce the condition that the interpolated value at any point
      // must be between its two nearest knots, DOES NOT make the entire spline monotonic, only the segments
      else if(InterPolation == kMonotonic)
      {
        // values of the secants at each point (for calculating monotone spline)
        _float_ * Secants = new _float_[nPoints -1];
        // values of the tangens at each point (for calculating monotone spline)
       _float_ *  Tangents = new _float_[nPoints];

        // get the knot values for the spline
        for (int i = 0; i < nPoints; ++i) {
          // 3 is the size of the TSpline3 coefficients
          Par[i] = new _float_[3];

          double x = -999.99, y = -999.99;
          spline->GetKnot(i, x, y);

          XPos[i]   = x;
          YResp[i]  = y;

          Tangents[i] = 0.0;
        }

        // deal with the case of two points (just do linear interpolation between them)
        if (nPoints ==2){
            Par[0][0] = (YResp[1] - YResp[0]) / (XPos[1] - XPos[0]);
            Par[0][1] = 0.0;
            Par[0][2] = 0.0;
            // extra "virtual" segment at end to make Par array shape fit with knot arrays shapes
            Par[1][1] = 0.0;
            Par[1][2] = 0.0;

            return;
        } // if nPoints !=2 do full monotonic spline treatment:

        // first pass over knots to calculate the secants
        for (int i = 0; i < nPoints-1; ++i) {
          Secants[i] = (YResp[i+1] - YResp[i]) / (XPos[i+1] - XPos[i]);
          //std::cout<<"secant "<<i<<": "<<Secants[i]<<std::endl;
        }

        Tangents[0] = Secants[0];
        Tangents[nPoints-1] = Secants[nPoints -2];

        _float_ alpha;
        _float_ beta;

        // second pass over knots to calculate tangents
        for (int i = 1; i < nPoints-1; ++i) {
          if ((Secants[i-1] >= 0.0 && Secants[i] >= 0.0) | (Secants[i-1] < 0.0 && Secants[i] < 0.0)){ //check for same sign
            Tangents[i] = (Secants[i-1] + Secants[i]) /2.0;
          }
        }

        // third pass over knots to rescale tangents
        for (int i = 0; i < nPoints-1; ++i) {
          if (Secants[i] == 0.0){
            Tangents[i] = 0.0;
            Tangents[i+1] = 0.0;
          }

          else{
            alpha = Tangents[i]  / Secants[i];
            beta = Tangents[i+1] / Secants[i];

            if (alpha <0.0){
              Tangents[i] = 0.0;
            }
            if (beta < 0.0){
              Tangents[i+1] = 0.0;
            }

            if (alpha * alpha + beta * beta >9.0){
              _float_ tau = 3.0 / sqrt(alpha * alpha + beta * beta);
              Tangents[i]   = tau * alpha * Secants[i];
              Tangents[i+1] = tau * beta  * Secants[i];
            }
          }
        } // finished rescaling tangents
        // fourth pass over knots to calculate the coefficients for the spline
        _float_ dx;
        for(int i = 0; i <nPoints-1; i++){
          _float_ b, c, d = -999.999;
          dx = XPos[i+1] - XPos[i];

          b = Tangents[i] * dx;
          c = 3.0* (YResp[i+1] - YResp[i]) -2.0 *dx * Tangents[i] - dx * Tangents[i +1];
          d = 2.0* (YResp[i] - YResp[i+1]) + dx * (Tangents[i] + Tangents[i+1]);

          Par[i][0] = b /  dx;
          Par[i][1] = c / (dx * dx);
          Par[i][2] = d / (dx * dx * dx);

          if((Par[i][0] == -999) | (Par[i][1] == -999) | (Par[i][2] ==-999) | (Par[i][0] == -999.999) | (Par[i][1] == -999.999) | (Par[i][2] ==-999.999)){
              std::cout<<"bad spline parameters for segment "<<i<<", will cause problems with GPU: (b, c, d) = "<<Par[i][0]<<", "<<Par[i][1]<<", "<<Par[i][2]<<std::endl;
          }
          //std::cout<<"b : "<<b<<std::endl;
          //std::cout<<"dx: "<<dx<<", x_0: "<<XPos[i]<<", x_1: "<<XPos[i+1]<<std::endl;
          //std::cout<<"    "<<" , y_0: "<<YResp[i]<<", y_1: "<<YResp[i+1]<<std::endl;
        }

        // include params for final "segment" outside of the spline so that par array fits with x and y arrays,
        // should never actually get used but if not set then the GPU code gets very angry
        Par[nPoints-1][0] = 0.0;
        Par[nPoints-1][1] = 0.0;
        Par[nPoints-1][2] = 0.0;

        // check the input spline for linear segments, if there are any then overwrite the calculated coefficients
        // this will pretty much only ever be the case if they are set to be linear in samplePDFND i.e. the user wants it to be linear
        for(int i = 0; i <nPoints-1; i++){
          double x = -999.99, y = -999.99, b = -999.99, c = -999.99, d = -999.99;
          spline->GetCoeff(i, x, y, b, c, d);

          if((c == 0.0 && d == 0.0)){
            Par[i][0] = b;
            Par[i][1] = 0.0;
            Par[i][2] = 0.0;
          }
        }
        delete[] Secants;
        delete[] Tangents;
      }

      delete spline;
      spline = NULL;
    }

    /// @brief Empty destructor
    ~TSpline3_red() {
      for (int i = 0; i < nPoints; ++i) {
        if (Par[i] != NULL) {
          delete[] Par[i];
        }
      }
      delete[] Par;
      delete[] XPos;
      delete[] YResp;
      Par = NULL;
      XPos = YResp = NULL;
    }

    /// Find the segment relevant to this variation in x
    /// See root/hist/hist/src/TSpline3::FindX(double) or samplePDFND....::FindSplineSegment
    inline int FindX(double x) {
      // The segment we're interested in (klow in ROOT code)
      int segment = 0;
      int kHigh = nPoints-1;
      // If the variation is below the lowest saved spline point
      if (x <= XPos[0]){
        segment = 0;
        // If the variation is above the highest saved spline point
      } else if (x >= XPos[nPoints-1]) {
        // Yes, the -2 is indeed correct, see TSpline.cxx:814 and //see: https://savannah.cern.ch/bugs/?71651
        segment = kHigh;
        // If the variation is between the maximum and minimum, perform a binary search
      } else {
        // The top point we've got
        int kHalf = 0;
        // While there is still a difference in the points (we haven't yet found the segment)
        // This is a binary search, incrementing segment and decrementing kHalf until we've found the segment
        while (kHigh - segment > 1) {
          // Increment the half-step
          kHalf = (segment + kHigh)/2;
          // If our variation is above the kHalf, set the segment to kHalf
          if (x > XPos[kHalf]) {
            segment = kHalf;
            // Else move kHigh down
          } else {
            kHigh = kHalf;
          }
        } // End the while: we've now done our binary search
      } // End the else: we've now found our point
      if (segment >= nPoints-1 && nPoints > 1) segment = nPoints-2;
      return segment;
    }

    /// @brief CW: Evaluate the weight from a variation
    inline double Eval(double var) {
      // Get the segment for this variation
      int segment = FindX(var);
      // The get the coefficients for this variation
      _float_ x = -999.99, y = -999.99, b = -999.99, c = -999.99, d = -999.99;
      GetCoeff(segment, x, y, b, c, d);
      double dx = var - x;
      // Evaluate the third order polynomial
      double weight = y+dx*(b+dx*(c+d*dx));
      return weight;
    }

    /// @brief CW: Get the number of points
    inline int GetNp() { return nPoints; }
    // Get the ith knot's x and y position
    inline void GetKnot(int i, _float_ &xtmp, _float_ &ytmp) {
      xtmp = XPos[i];
      ytmp = YResp[i];
    }

    /// @brief CW: Get the coefficient of a given segment
    inline void GetCoeff(int segment, _float_ &x, _float_ &y, _float_ &b, _float_ &c, _float_ &d) {
      b = Par[segment][0];
      c = Par[segment][1];
      d = Par[segment][2];
      x = XPos[segment];
      y = YResp[segment];
    }

    /// CW: Get the number
    inline std::string GetName() {
      std::stringstream ss;
      ss << ParamNo;
      return ss.str();
    }

  protected: //changed to protected from private so can be accessed by derived classes
    /// Number of points/knot in TSpline3
    _int_ nPoints;
    /// Always uses a third order polynomial, so hard-code the number of coefficients in implementation
    _float_ **Par;
    /// Positions of each x for each knot
    _float_ *XPos;
    /// y-value for each knot
    _float_ *YResp;
    /// Parameter number (which parameter is this spline for)
    _int_ ParamNo;
};

// ************************
/// @brief CW: Truncated spline class
class Truncated_Spline: public TSpline3_red {
// ************************
// cubic spline which is flat (returns y_first or y_last) if x outside of knot range
  public:
    /// Empty constructor
    Truncated_Spline()
    :TSpline3_red()
    {
    }

    /// The constructor that takes a TSpline3 pointer and copies in to memory
    Truncated_Spline(TSpline3* &spline, int Param = -1)
    :TSpline3_red(spline, Param)
    {
    }

    /// Empty destructor
    ~Truncated_Spline()
    {
    }

    /// Find the segment relevant to this variation in x
    /// See root/hist/hist/src/TSpline3::FindX(double) or samplePDFND....::FindSplineSegment
    inline int FindX(double x) {
      // The segment we're interested in (klow in ROOT code)
      int segment = 0;
      int kHigh = nPoints-1;
      // If the variation is below the lowest saved spline point
      if (x <= XPos[0]){
        segment = -1;
        // If the variation is above the highest saved spline point
      } else if (x >= XPos[nPoints-1]) {
        segment = -2;
        // If the variation is between the maximum and minimum, perform a binary search
      } else {
        // The top point we've got
        int kHalf = 0;
        // While there is still a difference in the points (we haven't yet found the segment)
        // This is a binary search, incrementing segment and decrementing kHalf until we've found the segment
        while (kHigh - segment > 1) {
          // Increment the half-step
          kHalf = (segment + kHigh)/2;
          // If our variation is above the kHalf, set the segment to kHalf
          if (x > XPos[kHalf]) {
            segment = kHalf;
            // Else move kHigh down
          } else {
            kHigh = kHalf;
          }
        } // End the while: we've now done our binary search
      } // End the else: we've now found our point
      if (segment >= nPoints-1 && nPoints > 1) segment = nPoints-2;
      return segment;
    }

    /// Evaluate the weight from a variation
    inline double Eval(double var) {
      // Get the segment for this variation
      int segment = FindX(var);
      // The get the coefficients for this variation
      _float_ x = -999.99, y = -999.99, b = -999.99, c = -999.99, d = -999.99;

      if(segment >=0){
        GetCoeff(segment, x, y, b, c, d);
      }

      // if var is outside of the defined range, set the coefficients to 0 so that Eval just returns the value at the end point of the spline
      else if(segment == -1){
        GetKnot(0, x, y);
        b = 0.0;
        c = 0.0;
        d = 0.0;
      }
      else if(segment == -2){
        GetKnot(nPoints-1, x, y);
        b = 0.0;
        c = 0.0;
        d = 0.0;
      }

      double dx = var - x;
      // Evaluate the third order polynomial
      double weight = y+dx*(b+dx*(c+d*dx));
      return weight;
    }
};


// ***************************
// A handy namespace for variables extraction
namespace MaCh3Utils {
  // ***************************

  // ***************************
  // Return mass for given PDG
  double GetMassFromPDG(int PDG);
  // ***************************

  extern std::unordered_map<int,int> KnownDetIDsMap;
  extern int nKnownDetIDs;

} // end MaCh3Utils namespace

// *****************
/// Enum to track the target material
enum TargetMat {
// *****************
  kTarget_H  = 1,
  kTarget_C  = 12,
  kTarget_N  = 14,
  kTarget_O  = 16,
  kTarget_Al = 27,
  kTarget_Ar = 40,
  kTarget_Ti = 48,
  kTarget_Fe = 56,
  kTarget_Pb = 207
};

// *****************
/// @brief Converted the Target Mat to a string
inline std::string TargetMat_ToString(TargetMat i) {
// *****************
  std::string name;

  switch(i) {
    case kTarget_H:
      name = "Hydrogen";
      break;
    case kTarget_C:
      name = "Carbon";
      break;
    case kTarget_N:
      name = "Nitrogen";
      break;
    case kTarget_O:
      name = "Oxygen";
      break;
    case kTarget_Al:
      name = "Aluminium";
      break;
    case kTarget_Ti:
      name = "Titanium";
      break;
    case kTarget_Fe:
      name = "Iron";
      break;
    case kTarget_Pb:
      name = "Lead";
      break;
    default:
      name = "TargetMat_Undefined";
      break;
  }

  return name;
}

// *****************
/// Enum to track the incoming neutrino species
enum NuPDG {
// *****************
  kNue = 12,
  kNumu = 14,
  kNutau = 16,
  kNueBar = -12,
  kNumuBar = -14,
  kNutauBar = -16
};

// *****************
/// Enum to track neutrino species for Prob3
enum ProbNu {
// *****************
  kProbNue = 1,
  kProbNumu = 2,
  kProbNutau = 3,
  kProbNueBar = -1,
  kProbNumuBar = -2,
  kProbNutauBar = -3
};

/// @brief ETA - Probs3++ doesn't use the PDG codes for the neutrino type so add in a small converter
inline int PDGToProbs(NuPDG pdg){

  int ReturnProbNu = -999;

  switch (pdg){
    case kNue:
	  ReturnProbNu = kProbNue;
	  break;
    case kNumu:
	  ReturnProbNu = kProbNumu;
	  break;
    case kNutau:
	  ReturnProbNu = kProbNutau;
	  break;
    case kNueBar:
	  ReturnProbNu = kProbNueBar;
	  break;
	case kNumuBar:
	  ReturnProbNu = kProbNumuBar;
	  break;
	case kNutauBar:
	  ReturnProbNu = kProbNutauBar;
	  break;
	default:
	  std::cout << "Unrecognised pdg for the neutrino so can't map this to an int for Prob3++" << std::endl;
	  break;
  }

  return ReturnProbNu;
}

inline int ProbsToPDG(ProbNu NuType){

  int ReturnNuPDG = -999;

  switch (NuType){
	case kProbNue:
	  ReturnNuPDG = static_cast<int>(kNue);
	  break;
	case kProbNumu:
	  ReturnNuPDG = static_cast<int>(kNumu);
	  break;
	case kProbNutau:
	  ReturnNuPDG = static_cast<int>(kNutau);
	  break;
	case kProbNueBar:
	  ReturnNuPDG = static_cast<int>(kNueBar);
	  break;
	case kProbNumuBar:
	  ReturnNuPDG = static_cast<int>(kNumuBar);
	  break;
	case kProbNutauBar:
	  ReturnNuPDG = static_cast<int>(kNutauBar);
	  break;
	default:
	  std::cout << "Unrecognised NuType for the neutrino so can't map this to a PDG code" << std::endl;
	  break;
  }

  return ReturnNuPDG;
}

/// Make an enum of the test statistic that we're using
enum TestStatistic {
  kPoisson,
  kBarlowBeeston,
  kIceCube,
  kPearson,
  kDembinskiAbdelmottele,
  kNTestStatistics //This only enumerates statistic
};

// **************************************************
/// @brief Convert a LLH type to a string
inline std::string TestStatistic_ToString(TestStatistic i) {
// **************************************************
  std::string name = "";

  switch(i) {
    case kPoisson:
    name = "Poisson";
    break;
    case kBarlowBeeston:
    name = "BarlowBeeston";
    break;
    case kIceCube:
    name = "IceCube";
    break;
    case kPearson:
    name = "Pearson";
    break;
    case kDembinskiAbdelmottele:
    name = "DembinskiAbdelmottele";
    break;
    default:
      std::cerr << "UNKNOWN LIKELHOOD SPECIFIED TO ND280!" << std::endl;
      std::cerr << "You gave test-statistic " << i << std::endl;
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      throw;
  }
  return name;
}

// *******************
/// Class containing information about interaction modes
class MaCh3_Modes {
  // *******************
  public:
    //KS: Getter for Modes, each experiment has to define it
    virtual int GetNModes();
    virtual std::string Mode_ToString(int i);
    virtual std::string Mode_ToFancyString(int i);
    virtual int Mode_ToSplineMode(int i);
};

/// @brief WP: Helper function for calculating unbinned Integral of TH2Poly i.e including overflow
double OverflowIntegral(TH2Poly*);

/// @brief WP: Helper function for calculating binned Integral of TH2Poly i.e not including overflow
double NoOverflowIntegral(TH2Poly*);

/// @brief WP: Poly Projectors
TH1D* PolyProjectionX(TObject* poly, std::string TempName, std::vector<double> xbins, bool computeErrors = false);
TH1D* PolyProjectionY(TObject* poly, std::string TempName, std::vector<double> ybins, bool computeErrors = false);

/// @brief WP: Helper to Normalise histograms
TH2Poly* NormalisePoly(TH2Poly* Histogram);

/// @brief WP: Helper to scale th2poly analogous to th2d scale with option "width"
TH2Poly* PolyScaleWidth(TH2Poly *Histogram, double scale);
/// @brief WP: Helper to calc integral of th2poly analogous to th2d integra; with option "width"
double PolyIntegralWidth(TH2Poly *Histogram);

/// @brief KS: Sanity check for TH2Poly
void CheckTH2PolyFileVersion(TFile *file);

/// @brief KS: Remove fitted TF1 from hist to make comparison easier
void RemoveFitter(TH1D* hist, std::string name);

/// @brief Helper to check if files exist or not
inline std::string file_exists(std::string filename) {
  std::ifstream infile(filename.c_str());
  if (!infile.good()) {
    std::cerr << "*** ERROR ***" << std::endl;
    std::cerr << "File " << filename << " does not exist" << std::endl;
    std::cerr << "Please try again" << std::endl;
    std::cerr << "*************" << std::endl;
    throw;
  }

  return filename;
}
/// @brief DB Get the Cernekov momentum threshold in MeV
double returnCherenkovThresholdMomentum(int PDG);

double CalculateQ2(double PLep, double PUpd, double EnuTrue, double InitialQ2 = 0.0);
double CalculateEnu(double PLep, double cosTheta, double EB, bool neutrino);


enum CUDAProb_nu {
  e_e = 0,
  e_m = 1,
  e_t = 2,
  m_e = 3,
  m_m = 4,
  m_t = 5,
  t_e = 6,
  t_m = 7,
  t_t = 8
};
 

// ************************************************
/// @brief Get CUDAProb3 flavour from intital and final states
inline CUDAProb_nu GetCUDAProbFlavour(int nu_i, int nu_f) {
//*************************************************  
    
  switch (abs(nu_i)) {
  case 1:
    switch (abs(nu_f)) {
    case 1:
      return CUDAProb_nu::e_e;
      break;
    case 2:
      return CUDAProb_nu::e_m;
      break;
    case 3:
      return CUDAProb_nu::e_t;
      break;
	default:
	  std::cout << "Unknow flavour " << nu_f << std::endl;
	  throw;
    } 
  case 2:
    switch (abs(nu_f)) {
    case 1:
      return CUDAProb_nu::m_e;
      break;
    case 2:
      return CUDAProb_nu::m_m;
      break;
    case 3:
      return CUDAProb_nu::m_t;
      break;
	default:
	  std::cout << "Unknow flavour " << nu_f << std::endl;
	  throw;
    } 
  case 3:
    switch (abs(nu_f)) {
    case 1:
      return CUDAProb_nu::t_e;
      break;
    case 2:
      return CUDAProb_nu::t_m;
      break;
    case 3:
      return CUDAProb_nu::t_t;
      break;
	default:
	  std::cout << "Unknow flavour " << nu_f << std::endl;
	  throw;
    }
  default:
	std::cout << "Unknow flavour " << nu_i << std::endl;
	throw;
  }

}
