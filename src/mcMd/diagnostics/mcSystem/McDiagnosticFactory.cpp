#ifndef MC_DIAGNOSTIC_FACTORY
#define MC_DIAGNOSTIC_FACTORY

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "McDiagnosticFactory.h" // Class header

#include "McEnergyOutput.h"
#include "McEnergyAverage.h"
#include "McPairEnergyAverage.h"
#include "McExternalEnergyAverage.h"
#include "McBondEnergyAverage.h"
#include "McPressureAverage.h"
#include "McStressAutoCorr.h"
#include "McIntraBondStressAutoCorr.h"
#include "McIntraBondTensorAutoCorr.h"
#include "McWriteRestart.h"

#include <mcMd/diagnostics/mutable/TypeDistribution.h>
#include <mcMd/diagnostics/mutable/SemiGrandDistribution.h>

namespace McMd
{

   using namespace Util;

   /*
   * Constructor.
   */
   McDiagnosticFactory::McDiagnosticFactory(McSimulation& simulation, 
                                            McSystem& system)
    : systemFactory_(simulation, system),
      simulationPtr_(&simulation),
      systemPtr_(&system)
   {}

   /* 
   * Return a pointer to a instance of Diagnostic subclass className.
   */
   Diagnostic* McDiagnosticFactory::factory(const std::string &className) const
   {
      Diagnostic* ptr = 0;

      // Try subfactories first (if any) 
      ptr = trySubfactories(className);
      if (ptr) return ptr;

      if (className == "McEnergyOutput") {
         ptr = new McEnergyOutput(system());
      } else
      #ifndef INTER_NOPAIR
      if (className == "McPairEnergyAverage") {
         ptr = new McPairEnergyAverage(system());
      } else
      #endif
      if (className == "McBondEnergyAverage") {
         ptr = new McBondEnergyAverage(system());
      } else
      #ifdef INTER_EXTERNAL
      if (className == "McExternalEnergyAverage") {
         ptr = new McExternalEnergyAverage(system());
      } else
      #endif
      if (className == "McEnergyAverage") {
         ptr = new McEnergyAverage(system());
      } else
      if (className == "McPressureAverage") {
         ptr = new McPressureAverage(system());
      } else
      if (className == "McStressAutoCorr") {
         ptr = new McStressAutoCorr(system());
      } else
      if (className == "McIntraBondStressAutoCorr") {
         ptr = new McIntraBondStressAutoCorr(system());
      } else
      if (className == "McIntraBondTensorAutoCorr") {
         ptr = new McIntraBondTensorAutoCorr(system());
      } else
      if (className == "TypeDistribution") {
         ptr = new TypeDistribution(system());
      } else 
      if (className == "SemiGrandDistribution") {
         ptr = new SemiGrandDistribution(system());
      } else
      if (className == "McWriteRestart") {
         ptr = new McWriteRestart(simulation());
      } 

      // If none of the above are matched, try SystemDiagnosticFactory
      if (!ptr) {
         ptr = systemFactory_.factory(className);
      }

      return ptr;
   }

}

#endif