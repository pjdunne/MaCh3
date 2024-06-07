#pragma once

#include "mcmc.h"

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
    DelayedRejectionMCMC(manager * const man);
    virtual ~DelayedRejectionMCMC();
    void ProposeDelayeedStep();
    void runMCMC() override;
    void CheckDelayedStep();



  protected:
    std::vector<std::vector<double>> rejected_step;
    std::vector<std::vector<double>> current_step;
};
