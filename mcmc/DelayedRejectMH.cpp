#include "DelayedRejectMH.h"

/**
 * TODO :
 *   -> Work out how to make N iterations arbitrary
 *   -> Cache alpha and q(whatever)
 *   -> Interface nicely with covarianceBase
 *   -> Sort out the spaghetti inheritance from mcmc
 */


// *********************
DelayedRejectionMCMC::DelayedRejectionMCMC(manger man) : mcmc(man){
// *********************
    // For now this is empty!
    // TODO: Add methods for setting scale etc.

    MACH3LOG_INFO("Using Delayed Rejection Metropolis Hastings!")


}


// *********************
DelayedRejectionMCMC::~DelayedRejectionMCMC{
// *********************
// TODO: Nothing necessary rn
}

void DelayedRejectionMCMC::setSystStepScale(double scale){
// Lets us rescale our proposal matrices
  if(osc) osc->setStepScale(scale);
  for(auto syst : systematics) { syst->setStepScale(scale);}
}

// *********************
void runMCMC(){
// HW: TODO: Rewrite MCMC so they can use inherited structures for this!
// ********************
  initialiseChain();

  // Begin MCMC
  for (step = stepStart; step < stepStart+chainLength; ++step)
  {
    stepClock->Start();
    // Set the initial rejection to false
    reject = false;

    // Print 10 steps in total
    if ( (step-stepStart) % (chainLength/10) == 0) {
      PrintProgress();
    }

    // Propose current step variation and save the systematic likelihood that results in this step being taken
    // Updates logLProp

    ProposeStep();
    CheckStep();

    // HW : TODO Generalise this..
    if(!accept){
      setSystStepScale(0.01); // Shrink
      ProposeDelayedStep();
      CheckDelayedStep();
      setSystStepScale(1.0); // Grow
    }

    // Auto save the output
    if (step % auto_save == 0) outTree->AutoSave();
  }

  // Save all the MCMC output
  SaveOutput();

  // Process MCMC
  ProcessMCMC();
}

void DelayedRejectionMCMC::ProposeDelayedStep(){
  // HW: Now for the tricky bit. REALLY we should
  // let this work for N iterations, as proof of concept we'll try 1

  // For a single stage process the formula is
  //     a = [q(y2, y1)(p(y2)-p(y1))]/[q(x,y1)[p(x)-p(y1)]]
  // Where y1 and x are the step we just rejected and the current step(resp.)
  // and y2 is the step we're about to propose. q is the proposal function and p the prior

  // So this will be hacky BUT should work. Firstly let's get the stuff we already have



}

void DelayedRejectionMCMC::CheckDelayedStep(){
  //
}
