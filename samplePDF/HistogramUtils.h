#pragma once

// ROOT include
#include "TGraphAsymmErrors.h"
#include "TH2Poly.h"
#include "TLorentzVector.h"
#include "TObjString.h"

// MaCh3 inlcudes
#include "samplePDF/Structs.h"

/// @brief WP: Helper function for calculating unbinned Integral of TH2Poly i.e including overflow
double OverflowIntegral(TH2Poly* poly);

/// @brief WP: Helper function for calculating binned Integral of TH2Poly i.e not including overflow
double NoOverflowIntegral(TH2Poly* poly);

/// @brief WP: Poly Projectors
TH1D* PolyProjectionX(TObject* poly, std::string TempName, const std::vector<double>& xbins, const bool computeErrors = false);
/// @brief WP: Poly Projectors
TH1D* PolyProjectionY(TObject* poly, std::string TempName, const std::vector<double>& ybins, const bool computeErrors = false);

/// @brief KS: Convert TH2D to TH2Poly
TH2D* ConvertTH2PolyToTH2D(TH2Poly *poly, TH2D *TH2Dhist);
/// @brief KS: Convert TH2Poly to TH2D
TH2Poly* ConvertTH2DtoTH2Poly(TH2D *TH2Dhist);

/// @brief WP: Helper to Normalise histograms
TH2Poly* NormalisePoly(TH2Poly* Histogram);

/// @brief WP: Helper to scale th2poly analogous to th2d scale with option "width"
TH2Poly* PolyScaleWidth(TH2Poly *Histogram, double scale);
/// @brief WP: Helper to calc integral of th2poly analogous to th2d integra; with option "width"
double PolyIntegralWidth(TH2Poly *Histogram);

/// @brief KS: ROOT changes something with binning when moving from ROOT 5 to ROOT 6. If you open ROOT5 produced file with ROOT6 you will be missing 9 last bins
/// @brief However if you use ROOT6 and have ROOT6 file exactly the same code will work. Something have changed with how TH2Poly bins are stored in TFile
/// @param file ROOT file that we will make version checks
void CheckTH2PolyFileVersion(TFile *file);

/// @brief KS: Remove fitted TF1 from hist to make comparison easier
void RemoveFitter(TH1D* hist, const std::string& name);

/// @brief Used by sigma variation, check how 1 sigma changes spectra
/// @param sigmaArrayLeft sigma var hist at -1 or -3 sigma shift
/// @param sigmaArrayCentr sigma var hist at prior values
/// @param sigmaArrayRight sigma var hist at +1 or +3 sigma shift
/// @param title A tittle for returned object
/// @return A `TGraphAsymmErrors` object that visualizes the sigma variation of spectra, showing confidence intervals between different sigma shifts.
TGraphAsymmErrors* MakeAsymGraph(TH1D* sigmaArrayLeft, TH1D* sigmaArrayCentr, TH1D* sigmaArrayRight, const std::string& title);

/// @brief Helper to check if files exist or not
inline std::string file_exists(std::string filename) {
  std::ifstream infile(filename.c_str());
  if (!infile.good()) {
    MACH3LOG_ERROR("*** ERROR ***");
    MACH3LOG_ERROR("File {} does not exist", filename);
    MACH3LOG_ERROR("Please try again");
    MACH3LOG_ERROR("*************");
    throw MaCh3Exception(__FILE__ , __LINE__ );
  }
  return filename;
}

/// @brief DB Get the Cherenkov momentum threshold in MeV
double returnCherenkovThresholdMomentum(int PDG);

/// @brief Recalculate Q^2 after Eb shift. Takes in shifted lepton momentum, lepton angle, and true neutrino energy
double CalculateQ2(double PLep, double PUpd, double EnuTrue, double InitialQ2 = 0.0);

/// @brief Recalculate Enu after Eb shift. Takes in shifted lepton momentum, lepton angle, and binding energy change, and if nu/anu
double CalculateEnu(double PLep, double cosTheta, double EB, bool neutrino);
