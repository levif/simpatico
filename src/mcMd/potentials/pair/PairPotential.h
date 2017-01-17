#ifndef MCMD_PAIR_POTENTIAL_H
#define MCMD_PAIR_POTENTIAL_H

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2014, The Regents of the University of Minnesota
* Distributed under the terms of the GNU General Public License.
*/

#include <util/space/Tensor.h>
#include <util/misc/Setable.h>
#include <util/global.h>
#include <string>

namespace Util
{
   class Vector;
}

namespace McMd
{

   using namespace Util;

   /**
   * Interface for a Pair Potential.
   *
   * \ingroup McMd_Pair_Module
   */
   class PairPotential
   {

   public:

      /**
      * Destructor (does nothing)
      */
      virtual ~PairPotential()
      {}

      /// \name Pair Interaction Interface
      //@{ 

      /**
      * Return pair energy for a single pair.
      */
      virtual 
      double energy(double rsq, int iAtomType, int jAtomType) const = 0;

      /**
      * Return force / separation for a single pair.
      */
      virtual 
      double forceOverR(double rsq, int iAtomType, int jAtomType) const = 0;

      /**
      * Modify a parameter, identified by a string.
      *
      * \param name   parameter name
      * \param i      type index of first atom
      * \param j      type index of first atom
      * \param value  new value of parameter
      */
      virtual void set(std::string name, int i, int j, double value) = 0;

      /**
      * Get a parameter value, identified by a string.
      *
      * \param name   parameter name
      * \param i      type index of first atom
      * \param j      type index of first atom
      */
      virtual double get(std::string name, int i, int j) const = 0;

      /**
      * Return maximum cutoff distance.
      */
      virtual double maxPairCutoff() const = 0;

      /**
      * Return name of pair interaction class (e.g., "LJPair").
      */
      virtual std::string interactionClassName() const = 0;

      //@}
      /// \name Global Energy and Stress Evaluators
      //@{ 

      /**
      * Mark the energy as unknown.
      */
      virtual void unsetEnergy();

      /**
      * Calculate the total nonBonded pair energy for the associated System.
      *
      * If energy is already known (set), this function does nothign and returns.
      *
      * In a charged system, this function computes and separately stores the 
      * non-Coulomb pair energy and the short-range Ewald part of the Coulomb 
      * energy.
      */
      virtual void computeEnergy() = 0;

      /**
      * Return the total pair energy for this System.
      *
      * This function calls computeEnergy() and returns the stored value.
      * of the non-Coulombic pair energy.
      */
      double energy();

      /**
      * Mark the stress as unknown.
      */
      virtual void unsetStress();

      /**
      * Compute and store the total nonbonded pressure.
      *
      * If the stress is already known (i.e., set). this function
      * does nothing and returns. Otherwise, it computes all stress
      * arising from short range non-bonded interactions, stores
      * the value and marks it as set.
      *
      * In a charged system, this function computes and separately
      * stores stress contributions arising from non-Coulombic pair
      * interactions and from the short range part of the Coulomb
      * potential. 
      */
      virtual void computeStress() = 0;

      /**
      * Compute and return pair stress tensor.
      *
      * This calls computeStress(), then returns stored value of
      * the stress arising from non-Coulombic pair interactions.
      *
      * \param stress (output) pair stress tensor
      */
      virtual void computeStress(Tensor& stress);

      /**
      * Compute and return xx, yy, zz pair pressures.
      *
      * This calls computeStress(), then returns stored values of
      * the pressures arising from non-Coulombic pair interactions.
      *
      * \param pressures (output) diagonal pair stress components
      */
      virtual void computeStress(Vector& pressures);

      /**
      * Compute and return scalar pair pressure.
      *
      * Pressure is the average of the diagonal stress components. 
      * This function calls computeStress(), then returns stored value
      * of the pressure arising from non-Coulombic pair interactions.
      *
      * \param pressure (output) scalar pair pressure.
      */
      virtual void computeStress(double& pressure);

      //@}

   protected:

      // Setable value of energy for entire system.
      Setable<double> energy_;

      // Setable value of stress tensor for entire system.
      Setable<Tensor> stress_;

   };

}
#endif
