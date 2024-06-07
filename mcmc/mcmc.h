#pragma once

#include "FitterBase.h"

class mcmc : public FitterBase {
 public:
   /// @brief Constructor
  mcmc(manager * const fitMan);
  /// @brief Destructor
  virtual ~mcmc();

  /// @brief Actual implementation of MCMC fitting algorithm
  void runMCMC() override;

  /// @brief Set how long chain should be
  inline void setChainLength(unsigned int L) { chainLength = L; };

  /// @brief Set initial step number, used when starting from another chain
  inline void setInitialStepNumber(const unsigned int stepNum = 0){stepStart = stepNum;};
  
  /// @brief Get name of class
  inline std::string GetName()const {return "MCMC";};
 protected:

  /// @brief Process MCMC output
  void ProcessMCMC();

  /// @brief Propose a step
  void ProposeStep();

  /// @brief Do we accept the step
  void CheckStep();

  /// @brief Does end of step accept proceess
  void acceptStep();

  /// @brief Print the progress
  void PrintProgress();

  /// @brief Load starting positions from the end of a previous chain
  inline void ReadParsFromFile(std::string file);

  /// @brief Initialise MCMC variables
  void initialiseChain();

  /// Do we reject based on hitting boundaries in systs
  bool reject;
  bool accept; // HACK This is bad...but useful

  /// number of steps in chain
  unsigned int chainLength;

  /// simulated annealing
  bool anneal;
  /// simulated annealing temperature
  double AnnealTemp;

  /// starting value of a chain, usually 0, unless starting from previous chain
  int stepStart;
};

