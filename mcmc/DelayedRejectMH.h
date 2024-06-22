#pragma once

#include "mcmc.h"
#include <valarray>
/*
 * HW : Delayed Rejection (...Adaptive Metropolis) implemented as described in
 * https://link.springer.com/article/10.1007/s11222-006-9438-0
 *
 * For now I'm electing to only allow ONE rejection but in principle this
 * could be extended
 *
 * TLDR: Rather than outright rejecting steps, DRAM uses the rejected step to propose a new
 * step.
 */

class DelayedRejectionMCMC : public mcmc{
  public:
    // @brief Delayed Rejection constructor
    DelayedRejectionMCMC(manager * const man);
    // @brief Delayed Rjection destructor
    virtual ~DelayedRejectionMCMC();
    // @brief Step proposal if first step reject
    void ProposeDelayedStep();
    // @brief MCMC Runner
    void runMCMC();
    // @brief checks if step is accepted/rejectd
    void CheckDelayedStep();

  protected:
    // @brief Sets step-size for delayed step
    void setSystStepScale(double scale);

    // @brief Sets separate step scales for each systematic
    void setSystStepScale(std::valarray<double> scale);

    // @brief gets scales for each systematic
    std::valarray<double> getSystStepScale();

    std::vector<std::vector<double>> current_step;
    double logLReject;
};
