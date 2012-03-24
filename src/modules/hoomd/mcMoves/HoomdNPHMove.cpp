#ifndef HOOMD_NPH_MOVE_CPP
#define HOOMD_NPH_MOVE_CPP

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "HoomdNPHMove.h"

#include <mcMd/mcSimulation/McSystem.h>
#include <mcMd/simulation/Simulation.h>
#include <mcMd/potentials/pair/McPairPotential.h>
#include <util/ensembles/BoundaryEnsemble.h>

namespace McMd
{

   using namespace Util;

   /*
   * Constructor
   */
   HoomdNPHMove::HoomdNPHMove(McSystem& system) :
      HoomdMove(system), W_(0.0)
   {
   }

   /*
   * Destructor.
   */
   HoomdNPHMove::~HoomdNPHMove()
   {}

   /*
   * Read parameters
   */
   void HoomdNPHMove::readParam(std::istream& in)
   {
      if ((! system().boundaryEnsemble().isIsobaric()) || (!energyEnsemble().isIsothermal()))
         UTIL_THROW("Must be in isothermal-isobaric ensemble.");

      readProbability(in);
      read<int>(in, "nStep", nStep_);
      read<double>(in, "dt", dt_);

      read<std::string>(in, "mode", modeIn_);
      if (modeIn_ == "cubic")
         integrationMode_ = TwoStepNPHGPU::cubic;
      else if (modeIn_ == "orthorhombic")
         integrationMode_ = TwoStepNPHGPU::orthorhombic;
      else if (modeIn_ == "tetragonal")
         integrationMode_ = TwoStepNPHGPU::tetragonal;
      else
         UTIL_THROW("Unsupported integration mode.");

      read<double>(in,"W", W_);
      read<double>(in, "skin", skin_);
      char* env;
      if ((env = getenv("OMPI_COMM_WORLD_LOCAL_RANK")) != NULL) {
         GPUId_ = atoi(env);
      } else {
         GPUId_ = -1;
      }

      //read<int>(in, "GPUId", GPUId_);
      // create HOOMD execution configuration
      executionConfigurationSPtr_ = 
        boost::shared_ptr<ExecutionConfiguration>(new ExecutionConfiguration(ExecutionConfiguration::GPU,GPUId_));

      // set CUDA error checking
      #ifdef UTIL_DEBUG
      executionConfigurationSPtr_->setCUDAErrorChecking(true);
      #endif

      // subscribe to molecule set notifications
      system().subscribeMoleculeSetChange(*this);

      #if defined(HOOMD_DEVEL) && defined(MCMD_LINK)
      // subscribe to link notifications
      system().linkMaster().Notifier<LinkAddEvent>::registerObserver(*this);
      system().linkMaster().Notifier<LinkResetEvent>::registerObserver(*this);
      system().linkMaster().Notifier<LinkRemoveEvent>::registerObserver(*this);
      #endif 

   }
 
   void HoomdNPHMove::createIntegrator()     
   {

      // create NVE Integrator 
      integratorSPtr_ = boost::shared_ptr<IntegratorTwoStep>(new IntegratorTwoStep(systemDefinitionSPtr_,dt_));

      boost::shared_ptr<Variant> variantPSPtr(new VariantConst(system().boundaryEnsemble().pressure()));
      // create IntegrationMethod
      twoStepNPHGPUSPtr_ = boost::shared_ptr<TwoStepNPHGPU>(new TwoStepNPHGPU(systemDefinitionSPtr_,groupAllSPtr_, thermoSPtr_, W_, variantPSPtr, integrationMode_,""));

      integratorSPtr_->addIntegrationMethod(twoStepNPHGPUSPtr_);

      // register pair and bond forces in Integrator
      integratorSPtr_->addForceCompute(pairForceSPtr_);
      integratorSPtr_->addForceCompute(bondForceSPtr_);
      #ifdef MCMD_EXTERNAL
      integratorSPtr_->addForceCompute(externalForceSPtr_);
      #endif


      // set flags for thermodynamic quantities requested by integrator
      // additionally, we require computation of potential energy
      PDataFlags peFlag;
      peFlag[pdata_flag::potential_energy] = 1;
      particleDataSPtr_->setFlags(integratorSPtr_->getRequestedPDataFlags() | peFlag);
   }

   /*
   * Generate, attempt and accept or reject a Hybrid MD/MC move.
   */
   bool HoomdNPHMove::move()
   {
      if ((!HoomdIsInitialized_) || moleculeSetHasChanged_) {
         initSimulation();
         moleculeSetHasChanged_ = false;
      }

      // We need to create the Integrator every time since we are starting
      // with new coordinates, velocities etc.
      // this does not seem to incur a significant performance decrease
      createIntegrator();

      System::MoleculeIterator molIter;
      Molecule::AtomIterator   atomIter;
      int nSpec = simulation().nSpecies();

      // Increment counter for attempted moves
      incrementNAttempt();

      // set hoomd simulation box
      BoxDim box;
      lengths_ = system().boundary().lengths();
      box.xlo = -lengths_[0]/Scalar(2.); box.xhi = lengths_[0]/Scalar(2.);
      box.ylo = -lengths_[1]/Scalar(2.); box.yhi = lengths_[1]/Scalar(2.);
      box.zlo = -lengths_[2]/Scalar(2.); box.zhi = lengths_[2]/Scalar(2.);
      particleDataSPtr_->setBox(box);
      {
      // copy atom coordinates into hoomd
      ArrayHandle<Scalar4> h_pos(particleDataSPtr_->getPositions(), access_location::host, access_mode::readwrite);
      ArrayHandle<Scalar4> h_vel(particleDataSPtr_->getVelocities(), access_location::host, access_mode::readwrite);
      ArrayHandle<unsigned int> h_tag(particleDataSPtr_->getTags(), access_location::host, access_mode::readwrite);
      ArrayHandle<unsigned int> h_rtag(particleDataSPtr_->getRTags(), access_location::host, access_mode::readwrite);

      for (int iSpec =0; iSpec < nSpec; ++iSpec) {
         system().begin(iSpec, molIter);
         for ( ; molIter.notEnd(); ++ molIter) {
            for (molIter->begin(atomIter); atomIter.notEnd(); ++atomIter) {
               unsigned int idx = (unsigned int) atomIter->id();
               Vector& pos = atomIter->position();
               h_pos.data[idx].x = pos[0] - lengths_[0]/2.;
               h_pos.data[idx].y = pos[1] - lengths_[1]/2.;
               h_pos.data[idx].z = pos[2] - lengths_[2]/2.;
    
               int type = atomIter->typeId();
               h_vel.data[idx].w = simulation().atomType(type).mass();
               h_pos.data[idx].w = __int_as_scalar(type);
               h_tag.data[idx] = idx;
               h_rtag.data[idx] = idx; 
            }
         }
      }

      // Generate random velocities
      generateRandomVelocities(h_vel);
      }

      // Done with the data

      // generate integrator variables from a Gaussian distribution
      double etax = 0.0;
      double etay = 0.0;
      double etaz = 0.0;
      Random& random = simulation().random();
      double temp = energyEnsemble().temperature();
      if (integrationMode_ == TwoStepNPHGPU::cubic) {
         // one degree of freedom
         // barostat_energy = 1/2 (1/W) eta_x^2
         double sigma = sqrt(temp/W_);
         etax = sigma*random.gaussian();
      } else if (integrationMode_ == TwoStepNPHGPU::tetragonal) {
         // two degrees of freedom
         // barostat_energy = 1/2 (1/W) eta_x^2 + 1/2 (1/(2W)) eta_y^2
         double sigma1 = sqrt(temp/W_);
         etax = sigma1*random.gaussian();
         double sigma2 = sqrt(temp/W_/2.0);
         etay = sigma2*random.gaussian();
      } else if (integrationMode_ == TwoStepNPHGPU::orthorhombic) {
         // three degrees of freedom 
         // barostat_energy = 1/2 (1/W) (eta_x^2 + eta_y^2 + eta_z^2)
         double sigma = sqrt(temp/W_);
         etax = sigma*random.gaussian();
         etay = sigma*random.gaussian();
         etaz = sigma*random.gaussian();
      } 
      twoStepNPHGPUSPtr_->setEta(etax,etay,etaz);
      
      // Notify that the particle order might have changed
      particleDataSPtr_->notifyParticleSort();

      // Calculate oldH (the conserved quantity of the Andersen barostat)
      double volume = lengths_[0]*lengths_[1]*lengths_[2];

      // Initialize integrator (calculate forces and potential energy for step 0)
      integratorSPtr_->prepRun(0);

      // H = U + PV + barostat_energy
      thermoSPtr_->compute(0);
      double oldH = thermoSPtr_->getLogValue("kinetic_energy",0);
      oldH += thermoSPtr_->getLogValue("potential_energy",0);
      oldH += system().boundaryEnsemble().pressure() * volume;
      oldH += integratorSPtr_->getLogValue("nph_barostat_energy",0);
//      std::cout << "Ekin = " << thermoSPtr_->getLogValue("kinetic_energy",nStep_) << " ";
//      std::cout << "Epot = " << thermoSPtr_->getLogValue("potential_energy",nStep_) << " ";
//      std::cout << "PV = " << system().boundaryEnsemble().pressure() * volume << " ";
//      std::cout << "Ebaro = " << integratorSPtr_->getLogValue("nph_barostat_energy",nStep_) << " ";
//      std::cout << "H = " << oldH << std::endl;

      // Integrate nStep_ steps forward
      for (int iStep = 0; iStep < nStep_; ++iStep) {
         integratorSPtr_->update(iStep);

         // do we need to sort the particles?
         // do not sort at time step 0 to speed up short runs
         if (! (iStep % sorterPeriod_) && iStep)
           sorterSPtr_->update(iStep);
      }

      Vector newLengths;
      box = particleDataSPtr_->getBox();
      newLengths[0] = box.xhi - box.xlo;
      newLengths[1] = box.yhi - box.ylo;
      newLengths[2] = box.zhi - box.zlo;
      volume = newLengths[0]*newLengths[1]*newLengths[2];
 

      // Calculate new value of the conserved quantity
      thermoSPtr_->compute(nStep_);
      double newH = thermoSPtr_->getLogValue("kinetic_energy",nStep_);
      newH += thermoSPtr_->getLogValue("potential_energy",nStep_);
      newH += system().boundaryEnsemble().pressure() * volume;
      newH += integratorSPtr_->getLogValue("nph_barostat_energy",nStep_);
//      std::cout << "Ekin_2 = " << thermoSPtr_->getLogValue("kinetic_energy",nStep_) << " ";
//      std::cout << "Epot_2 = " << thermoSPtr_->getLogValue("potential_energy",nStep_) << " ";
//      std::cout << "PV_2 = " << system().boundaryEnsemble().pressure() * volume << " ";
//      std::cout << "Ebaro_2 = " << integratorSPtr_->getLogValue("nph_barostat_energy",nStep_) << " ";
//      std::cout << "H_2 = " << newH << std::endl;

      // Decide whether to accept or reject
      bool accept = random.metropolis( boltzmann(newH-oldH) );

      if (accept) {
         // read back new boundary
         box = particleDataSPtr_->getBox();
         lengths_ = newLengths;
         system().boundary().setLengths(lengths_);
         
         // read back integrated positions
         ArrayHandle<Scalar4> h_pos(particleDataSPtr_->getPositions(), access_location::host, access_mode::read);
         ArrayHandle<Scalar4> h_vel(particleDataSPtr_->getVelocities(), access_location::host, access_mode::read);
         ArrayHandle<unsigned int> h_tag(particleDataSPtr_->getTags(), access_location::host, access_mode::read);
         ArrayHandle<unsigned int> h_rtag(particleDataSPtr_->getRTags(), access_location::host, access_mode::read);
         
         for (int iSpec = 0; iSpec < nSpec; ++iSpec) {
            system().begin(iSpec, molIter);
            for ( ; molIter.notEnd(); ++molIter) {
               for (molIter->begin(atomIter); atomIter.notEnd(); ++atomIter) {
                  unsigned int idx = h_rtag.data[atomIter->id()]; 
                  atomIter->position() = Vector(h_pos.data[idx].x+lengths_[0]/2.,
                                                h_pos.data[idx].y+lengths_[1]/2.,
                                                h_pos.data[idx].z+lengths_[2]/2.);
               }
            }
         }
         // Done with data
         system().pairPotential().buildCellList();
         incrementNAccept();
      } else {
          // not accepted, do nothing
      }

//      std::cout << "Lx = " << lengths_[0] << " Ly = " << lengths_[1] << " Lz = " << lengths_[2] << std::endl;
      return accept;
   }
 
}
#endif
