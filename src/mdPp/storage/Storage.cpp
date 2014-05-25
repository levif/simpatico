#ifndef MDPP_STORAGE_CPP
#define MDPP_STORAGE_CPP

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2012, David Morse (morse012@umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "Storage.h"

namespace MdPp 
{

   /*
   * Constructor.
   */
   Storage::Storage()
    : newAtomPtr_(0),
      atomCapacity_(0)
      #ifdef INTER_BOND
      , bondCapacity_(0)
      #endif
      #ifdef INTER_ANGLE
      , angleCapacity_(0)
      #endif
      #ifdef INTER_DIHEDRAL
      , dihedralCapacity_(0)
      #endif
   {  setClassName("Storage"); }

   /*
   * Destructor.
   */
   Storage::~Storage()
   {}

   /*
   * Open, read and close a parameter file.
   */
   void Storage::readParam(const char* filename)
   {
      std::ifstream in;
      in.open(filename);
      readParam(in);
      in.close();
   }

   /*
   * Read parameters from file.
   */
   void Storage::readParameters(std::istream& in)
   {
      read<int>(in, "atomCapacity", atomCapacity_); 

      atoms_.allocate(atomCapacity_);
      atomPtrs_.allocate(atomCapacity_);
      for (int i = 0; i < atomCapacity_; ++i) {
         atomPtrs_[i] = 0;
      }

      bondCapacity_ = 0;
      bool isRequired = false;
      read<int>(in, "bondCapacity", bondCapacity_, isRequired); 
      if (bondCapacity_ > 0) {
         bonds_.allocate(bondCapacity_);
      }

      // etc. for angles, dihedrals
      // etc. for angles dihedrals
  
      nSpecies_ = 0;
      isRequired = false;
      read<int>(in, "nSpecies", nSpecies_, isRequired);
      if (nSpecies_ > 0) {
         species_.allocate(nSpecies_);
         for (int i = 0; i < nSpecies_; ++i) {
            in >> species_[i];
            species_[i].setId(i);
         }
      }

      // readParamComposite(in, analyzerManager_);
   }

   // Atom Management

   /*
   * Return pointer to location for new atom.
   */
   Atom* Storage::newAtomPtr()
   {
      if (newAtomPtr_) {
         UTIL_THROW("Error: an new atom is still active");
      }
      int size = atoms_.size() + 1;
      atoms_.resize(size);
      newAtomPtr_ = &atoms_[size - 1];
      return newAtomPtr_;
   }

   /*
   * Finalize addition of new atom.
   */
   void Storage::addAtom()
   {
      if (!newAtomPtr_) {
         UTIL_THROW("Error: No active new atom");
      }
      int id = newAtomPtr_->id;
      atomPtrs_[id] = newAtomPtr_;
      newAtomPtr_ = 0;
   }

   /*
   * Remove all atoms and bonds - set to empty state.
   */
   void Storage::clear()
   {
      atoms_.clear();
      bonds_.clear();
      for (int i = 0; i < atomCapacity_; ++i) {
         atomPtrs_[i] = 0;
      }
   }

}
#endif