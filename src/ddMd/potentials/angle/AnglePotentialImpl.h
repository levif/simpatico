#ifndef DDMD_ANGLE_POTENTIAL_IMPL_H
#define DDMD_ANGLE_POTENTIAL_IMPL_H

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2012, David Morse (morse012@umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "AnglePotential.h" // base class

namespace Util
{
   class Vector;
   class Tensor;
}

namespace DdMd
{

   using namespace Util;

   class Simulation;
   template <int N> class GroupStorage;

   /**
   * Implementation template for a AnglePotential.
   *
   * \ingroup DdMd_Angle_Module
   */
   template <class Interaction>
   class AnglePotentialImpl : public AnglePotential
   {

   public:

      /** 
      * Constructor.
      */
      AnglePotentialImpl(Simulation& simulation);

      /** 
      * Default constructor.
      */
      AnglePotentialImpl();

      /** 
      * Destructor.
      */
      virtual ~AnglePotentialImpl();

      /**
      * Set the maximum number of atom types.
      */
      virtual void setNAngleType(int nAtomType);
  
      /**
      * Read potential energy parameters.
      * 
      * This method reads the angle potential Interaction parameter
      * block. Before calling Interaction::readParam(), it passes
      * simulation().nAngleType() to Interaction::setNAtomType().
      */
      virtual void readParam(std::istream& in);

      /// \name Interaction interface
      //@{

      /**
      * Returns potential energy for one angle.
      *
      * \param cosTheta  cosine of the bend angle.
      * \param type      type of bend angle.
      */
      double angleEnergy(double cosTheta, int type) const;
 
      /**
      * Returns forces along two bonds at the angle, for use in MD and stress
      * calculation.
      *
      * \param R1     bond vector from atom 1 to 2.
      * \param R2     bond vector from atom 2 to 3.
      * \param F1     return force along R1 direction.
      * \param F2     return force along R2 direction.
      * \param type   type of angle.
      */
      void angleForce(const Vector& R1, const Vector& R2,
                      Vector& F1, Vector& F2, int type) const;

      #if 0
      /**
      * Return pair interaction class name (e.g., "CosineAngle").
      */
      virtual std::string interactionClassName() const;
      #endif

      /**
      * Return angle interaction by const reference.
      */
      const Interaction& interaction() const;

      /**
      * Return angle interaction by reference.
      */
      Interaction& interaction();

      //@}
      /// \name Total Energy, Force and Stress 
      //@{

      /**
      * Add the angle forces for all atoms.
      */
      virtual void addForces();

      /**
      * Add the angle forces for all atoms and compute energy.
      */
      virtual void addForces(double& energy);

      /**
      * Compute the total angle energy for all processors
      * 
      * Call on all processors (MPI reduce operation).
      */
      #ifdef UTIL_MPI
      virtual void computeEnergy(MPI::Intracomm& communicator);
      #else
      virtual void computeEnergy();
      #endif

      /**
      * Compute the covalent bond stress.
      * 
      * Call on all processors.
      */
      #ifdef UTIL_MPI
      virtual void computeStress(MPI::Intracomm& communicator);
      #else
      virtual void computeStress();
      #endif

      //@}

   private:

      /**
      * Pointer to Interaction (evaluates the angle potential function).
      */ 
      Interaction* interactionPtr_;

      /*
      * Compute forces and/or energy.
      *
      * Return energy if energy is computed.
      */
      double addForces(bool needForce, bool needEnergy);

   };

}

#include "AnglePotential.h"
#include <ddMd/simulation/Simulation.h>
#include <ddMd/storage/GroupStorage.h>
#include <ddMd/storage/GroupIterator.h>

#include <util/boundary/Boundary.h>
#include <util/space/Dimension.h>
#include <util/space/Vector.h>
#include <util/space/Tensor.h>
#include <util/global.h>

#include <fstream>

namespace DdMd
{

   using namespace Util;

   /* 
   * Constructor.
   */
   template <class Interaction>
   AnglePotentialImpl<Interaction>::AnglePotentialImpl(Simulation& simulation)
    : AnglePotential(simulation),
      interactionPtr_(0)
   {  interactionPtr_ = new Interaction(); }
 
   /* 
   * Default constructor.
   */
   template <class Interaction>
   AnglePotentialImpl<Interaction>::AnglePotentialImpl()
    : AnglePotential(),
      interactionPtr_(0)
   {  interactionPtr_ = new Interaction(); }
 
   /* 
   * Destructor. 
   */
   template <class Interaction>
   AnglePotentialImpl<Interaction>::~AnglePotentialImpl() 
   {
      if (interactionPtr_) {
         delete interactionPtr_;
         interactionPtr_ = 0;
      }
   }

   /*
   * Set the maximum number of atom types.
   */
   template <class Interaction>
   void AnglePotentialImpl<Interaction>::setNAngleType(int nAngleType)
   {  interaction().setNAngleType(nAngleType); }

   /*
   * Read angle interaction parameters.
   */
   template <class Interaction>
   void AnglePotentialImpl<Interaction>::readParam(std::istream& in)
   {
      readBegin(in,"AnglePotential");
      bool nextIndent = false;
      readParamComposite(in, interaction(), nextIndent);
      readEnd(in);
   }

   /*
   * Return energy for a single angle.
   */
   template <class Interaction>
   double 
   AnglePotentialImpl<Interaction>::angleEnergy(double cosTheta, int angleTypeId) 
      const
   {  return interaction().energy(cosTheta, angleTypeId); }

   /*
   * Return forces for a single angle.
   */
   template <class Interaction> void 
   AnglePotentialImpl<Interaction>::angleForce(const Vector& R1, 
                                               const Vector& R2, 
                                               Vector& F1, Vector& F2, 
                                               int typeId) const
   {  interaction().force(R1, R2, F1, F2, typeId); }

   #if 0
   /*
   * Return angle potential interaction class name.
   */
   template <class Interaction>
   std::string AnglePotentialImpl<Interaction>::interactionClassName() const
   {  return interaction().className(); }
   #endif

   /**
   * Get Interaction by reference.
   */
   template <class Interaction>
   inline Interaction& AnglePotentialImpl<Interaction>::interaction()
   {  return *interactionPtr_; }

   /**
   * Get Interaction by const reference.
   */
   template <class Interaction>
   inline const Interaction& AnglePotentialImpl<Interaction>::interaction() const
   {  return *interactionPtr_; }

   /*
   * Increment atomic forces, without calculating energy.
   */
   template <class Interaction>
   void AnglePotentialImpl<Interaction>::addForces()
   {  addForces(true, false);  }

   /*
   * Increment atomic forces and compute pair energy for this processor.
   */
   template <class Interaction>
   void AnglePotentialImpl<Interaction>::addForces(double& energy)
   {  energy = addForces(true, true);  }

   /*
   * Compute total angle energy on all processors.
   */
   template <class Interaction>
   #ifdef UTIL_MPI
   void 
   AnglePotentialImpl<Interaction>::computeEnergy(MPI::Intracomm& communicator)
   #else
   void AnglePotentialImpl<Interaction>::computeEnergy()
   #endif
   {
      double localEnergy = 0.0; 
      double totalEnergy = 0.0; 
      localEnergy = addForces(false, true); 
      #ifdef UTIL_MPI
      communicator.Reduce(&localEnergy, &totalEnergy, 1, 
                          MPI::DOUBLE, MPI::SUM, 0);
      if (communicator.Get_rank() != 0) {
         totalEnergy = 0.0;
      }
      setEnergy(totalEnergy);
      #else
      setEnergy(localEnergy);
      #endif
   }

   /*
   * Increment atomic forces and/or pair energy (private).
   */
   template <class Interaction> double 
   AnglePotentialImpl<Interaction>::addForces(bool needForce, bool needEnergy)
   {
      // Preconditions
      //if (!storage().isInitialized()) {
      //   UTIL_THROW("AtomStorage must be initialized");
      //}

      Vector dr1; // R[1] - R[0]
      Vector dr2; // R[2] - R[1]
      Vector f1, f2;
      double rsq1, rsq2, cosTheta;
      double energy = 0.0;
      double angleEnergy;
      double fraction;
      double third = 1.0/3.0;
      GroupIterator<3> iter;
      Atom* atom0Ptr;
      Atom* atom1Ptr;
      Atom* atom2Ptr;
      int   type, isLocal0, isLocal1, isLocal2;

      storage().begin(iter);
      for ( ; iter.notEnd(); ++iter) {
         type = iter->typeId();
         atom0Ptr = iter->atomPtr(0);
         atom1Ptr = iter->atomPtr(1);
         atom2Ptr = iter->atomPtr(2);
         isLocal0 = !(atom0Ptr->isGhost());
         isLocal1 = !(atom1Ptr->isGhost());
         isLocal2 = !(atom2Ptr->isGhost());
         // Calculate minimimum image separations
         rsq1 = boundary().distanceSq(atom1Ptr->position(),
                                         atom0Ptr->position(), dr1);
         rsq2 = boundary().distanceSq(atom2Ptr->position(),
                                         atom1Ptr->position(), dr2);
         if (needEnergy) {
            cosTheta = dr1.dot(dr2) / sqrt(rsq1 * rsq2);
            angleEnergy = interaction().energy(cosTheta, type);
            fraction = (isLocal0 + isLocal1 + isLocal2)*third;
            energy += fraction*angleEnergy;
         }
         if (needForce) {
            interaction().force(dr1, dr2, f1, f2, type);
            if (isLocal0) {
               atom0Ptr->force() += f1;
            }
            if (isLocal1) {
               atom1Ptr->force() -= f1;
               atom1Ptr->force() += f2;
            }
            if (isLocal2) {
               atom2Ptr->force() -= f2;
            }
         }
      }
      return energy;
   }

   /*
   * Compute total pair stress (Call on all processors).
   */
   template <class Interaction>
   #ifdef UTIL_MPI
   void AnglePotentialImpl<Interaction>::computeStress(MPI::Intracomm& communicator)
   #else
   void AnglePotentialImpl<Interaction>::computeStress()
   #endif
   {
      Tensor localStress;
      Vector dr1, dr2;
      Vector f1, f2;
      double factor;
      double prefactor = -1.0/3.0;
      GroupIterator<3> iter;
      Atom*  atom0Ptr;
      Atom*  atom1Ptr;
      Atom*  atom2Ptr;
      int    type;
      int    isLocal0, isLocal1, isLocal2;

      localStress.zero();
      // Iterate over bonds
      storage().begin(iter);
      for ( ; iter.notEnd(); ++iter) {
         atom0Ptr = iter->atomPtr(0);
         atom1Ptr = iter->atomPtr(1);
         atom2Ptr = iter->atomPtr(2);
         type = iter->typeId();
         isLocal0 = !(atom0Ptr->isGhost());
         isLocal1 = !(atom1Ptr->isGhost());
         isLocal2 = !(atom2Ptr->isGhost());
         boundary().distanceSq(atom1Ptr->position(),
                                  atom0Ptr->position(), dr1);
         boundary().distanceSq(atom2Ptr->position(),
                                  atom1Ptr->position(), dr2);

         // f1 -- along dr1; f2 -- along dr2.
         interaction().force(dr1, dr2, f1, f2, type);

         factor = prefactor*(isLocal0 + isLocal1 + isLocal2);
         dr1 *= factor;
         dr2 *= factor;
         incrementPairStress(f1, dr1, localStress);
         incrementPairStress(f2, dr2, localStress);

      }

      // Normalize by volume 
      localStress /= boundary().volume();

      #ifdef UTIL_MPI
      // Reduce results from all processors
      Tensor totalStress;
      int    root = 0;
      communicator.Reduce(&localStress(0, 0), &totalStress(0, 0), 
                          Dimension*Dimension, MPI::DOUBLE, MPI::SUM, root);
      if (communicator.Get_rank() != root) {
         totalStress.zero();
      }
      setStress(totalStress);
      #else
      setStress(localStress);
      #endif

   }

}
#endif
