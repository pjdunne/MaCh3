#pragma once

#include "LikelihoodFit.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"


class MinuitFit : public LikelihoodFit {
 public:
  MinuitFit(manager * const fitMan);
  virtual ~MinuitFit();

  void runMCMC() override;

  inline std::string GetName()const {return "MinuitFit";};

  private:
    ROOT::Math::Minimizer* minuit;
};

