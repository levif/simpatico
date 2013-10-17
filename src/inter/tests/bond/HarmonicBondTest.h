#ifndef HARMONIC_BOND_TEST_H
#define HARMONIC_BOND_TEST_H

#include <inter/bond/HarmonicBond.h>
#include <inter/tests/bond/BondTestTemplate.h>

#include <iostream>
#include <fstream>

using namespace Util;
using namespace Inter;

class HarmonicBondTest : public BondTestTemplate<HarmonicBond>
{

protected:

   BondTestTemplate<HarmonicBond>::setNBondType;
   BondTestTemplate<HarmonicBond>::readParamFile;
   BondTestTemplate<HarmonicBond>::forceOverR;
   BondTestTemplate<HarmonicBond>::energy;

public:

   void setUp()
   {
      eps_ = 1.0E-6;
      setNBondType(1);
      readParamFile("in/HarmonicBond");
   }

   void testSetUp() 
   {
      printMethod(TEST_FUNC);
      if (verbose() > 0) {
         std::cout << std::endl; 
         interaction_.writeParam(std::cout);
      }
   }

   void testEnergy() 
   {
      printMethod(TEST_FUNC);
      double e;

      e = energy(1.0, 0);
      if (verbose() > 0) {
         std::cout << std::endl; 
         std::cout << "energy(1.00, 0) = " << e << std::endl;
      }
      TEST_ASSERT(eq(e, 0.0));

      e = energy(0.81, 0);
      if (verbose() > 0) {
         std::cout << "energy(0.81, 0) = " << e << std::endl;
      }
      TEST_ASSERT(e < 2.0);

      e = energy(1.21, 0);
      if (verbose() > 0) {
         std::cout << "energy(1.25, 0) = " << e << std::endl;
      }
      TEST_ASSERT(e > 0.0);

   }

   void testForceOverR() 
   {
      printMethod(TEST_FUNC);
      double f;
      type_ = 0;
 
      rsq_ = 1.0;
      f = forceOverR();
      TEST_ASSERT(testForce());
      if (verbose() > 0) {
         std::cout << std::endl;
         std::cout << "forceOverR(1.00, 0) = " <<  f << std::endl;
      }
      // TEST_ASSERT(eq(f, 48.0));

      rsq_ = 0.81;
      f = forceOverR();
      TEST_ASSERT(testForce());
      if (verbose() > 0) {
         std::cout << "forceOverR(0.81, 0) = " <<  f << std::endl;
      }
      // TEST_ASSERT(f > 48.0);

      rsq_ = 1.21;
      f = forceOverR();
      TEST_ASSERT(testForce());
      if (verbose() > 0) {
         std::cout << "forceOverR(1.21, 0) = " <<  f << std::endl;
      }
      // TEST_ASSERT(eq(f, 0.0));

   }

   #if 0
   void testGetSet() {
      printMethod(TEST_FUNC);

      TEST_ASSERT(eq(interaction_.epsilon(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), 2.0));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), 2.0));
      TEST_ASSERT(eq(interaction_.sigma(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(0, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 0), 1.0));

      interaction_.setEpsilon(0, 1, 1.3);
      TEST_ASSERT(eq(interaction_.epsilon(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), 1.3));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), 1.3));
      
      interaction_.setEpsilon(0, 0, 1.1);
      TEST_ASSERT(eq(interaction_.epsilon(0, 0), 1.1));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), 1.3));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), 1.3));
      
      interaction_.setSigma(0, 1, 1.05);
      TEST_ASSERT(eq(interaction_.sigma(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(0, 1), 1.05));
      TEST_ASSERT(eq(interaction_.sigma(1, 0), 1.05));

   }

   void testModify() {
      printMethod(TEST_FUNC);

      TEST_ASSERT(eq(interaction_.epsilon(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), 2.0));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), 2.0));
      TEST_ASSERT(eq(interaction_.sigma(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(0, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 0), 1.0));

      interaction_.set("epsilon", 0, 1, 1.3);
      TEST_ASSERT(eq(interaction_.epsilon(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), 1.3));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), 1.3));
      
      interaction_.set("epsilon", 0, 0, 1.1);
      TEST_ASSERT(eq(interaction_.epsilon(0, 0), 1.1));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), 1.3));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), 1.3));
      TEST_ASSERT(eq(interaction_.sigma(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(0, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 0), 1.0));
      
      interaction_.set("epsilon", 0, 1, 1.05);
      TEST_ASSERT(eq(interaction_.epsilon(0, 0), 1.1));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), 1.05));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), 1.05));
      TEST_ASSERT(eq(interaction_.sigma(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(0, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 0), 1.0));

      interaction_.set("sigma", 0, 1, 1.05);
      TEST_ASSERT(eq(interaction_.epsilon(0, 0), 1.1));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), 1.05));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), 1.05));
      TEST_ASSERT(eq(interaction_.sigma(0, 0), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(1, 1), 1.0));
      TEST_ASSERT(eq(interaction_.sigma(0, 1), 1.05));
      TEST_ASSERT(eq(interaction_.sigma(1, 0), 1.05));

      TEST_ASSERT(eq(interaction_.epsilon(0, 0), interaction_.get("epsilon", 0, 0)));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), interaction_.get("epsilon", 1, 1)));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), interaction_.get("epsilon", 1, 0)));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), interaction_.get("epsilon", 0, 1)));
      TEST_ASSERT(eq(interaction_.sigma(0, 0), interaction_.get("sigma", 0, 0)));
      TEST_ASSERT(eq(interaction_.sigma(1, 1), interaction_.get("sigma", 1, 1)));
      TEST_ASSERT(eq(interaction_.sigma(1, 0), interaction_.get("sigma", 1, 0)));
      TEST_ASSERT(eq(interaction_.sigma(0, 1), interaction_.get("sigma", 0, 1)));

   }

   void testSaveLoad() {
      printMethod(TEST_FUNC);

      Serializable::OArchive oar;
      openOutputFile("out/serial", oar.file());
      interaction_.save(oar);
      oar.file().close();
      
      Serializable::IArchive iar;
      openInputFile("out/serial", iar.file());

      HarmonicBond clone;
      clone.load(iar);

      TEST_ASSERT(eq(interaction_.epsilon(0, 0), clone.epsilon(0,0)));
      TEST_ASSERT(eq(interaction_.epsilon(1, 0), clone.epsilon(1,0)));
      TEST_ASSERT(eq(interaction_.epsilon(0, 1), clone.epsilon(0,1)));
      TEST_ASSERT(eq(interaction_.epsilon(1, 1), clone.epsilon(1,1)));
      TEST_ASSERT(eq(interaction_.sigma(0, 0), clone.sigma(0,0)));
      TEST_ASSERT(eq(interaction_.sigma(1, 0), clone.sigma(1,0)));
      TEST_ASSERT(eq(interaction_.sigma(0, 1), clone.sigma(0,1)));
      TEST_ASSERT(eq(interaction_.sigma(1, 1), clone.sigma(1,1)));

      TEST_ASSERT(eq(interaction_.energy(0.95, 0, 1), clone.energy(0.95, 0, 1)));
      TEST_ASSERT(eq(interaction_.forceOverR(0.95, 0, 1), clone.forceOverR(0.95, 0, 1)));
      TEST_ASSERT(eq(interaction_.energy(1.25, 0, 1), clone.energy(1.25, 0, 1)));
      TEST_ASSERT(eq(interaction_.forceOverR(1.25, 0, 1), clone.forceOverR(1.25, 0, 1)));
      TEST_ASSERT(eq(interaction_.energy(1.26, 0, 1), clone.energy(1.26, 0, 1)));
      TEST_ASSERT(eq(interaction_.forceOverR(1.26, 0, 1), clone.forceOverR(1.26, 0, 1)));
   }

   void testRandomBondLength() 
   {
      int    type, i;
      double r, beta;
  
      Random *random;
 
      printMethod(TEST_FUNC);
 
      std::ifstream in;
      openInputFile("potentials/bond/in/Random", in);
      random = new Random;
      random->readParam(in);

      beta = 1.0;
      type = 0;
      for (i=0; i < 20; i++) {
         r = bondPotential.randomBondLength(random, beta, type);
         std::cout << "random bond length = " << r << std::endl;
      }
   }
     
   #endif

};

TEST_BEGIN(HarmonicBondTest)
TEST_ADD(HarmonicBondTest, testSetUp)
TEST_ADD(HarmonicBondTest, testEnergy)
TEST_ADD(HarmonicBondTest, testForceOverR)
//TEST_ADD(HarmonicBondTest, testGetSet)
//TEST_ADD(HarmonicBondTest, testModify)
//TEST_ADD(HarmonicBondTest, testSaveLoad)
TEST_END(HarmonicBondTest)

#endif
