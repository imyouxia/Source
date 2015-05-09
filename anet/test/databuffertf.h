#ifndef DATABUFFERTF_H_
#define DATABUFFERTF_H_

#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>

namespace anet {
  class DataBufferTF : public CppUnit::TestFixture{
  
    CPPUNIT_TEST_SUITE(DataBufferTF);
    CPPUNIT_TEST(testShrink);
    CPPUNIT_TEST(testExpand);
    CPPUNIT_TEST(testFindBytes);
    CPPUNIT_TEST(testWriteAndRead);
    CPPUNIT_TEST(testDrainData);
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();
    void testShrink();
    void testExpand();
    void testFindBytes();
    void testFindBytes_KMP();
    void testWriteAndRead();
    void testDrainData();
  };
}


#endif /*DATABUFFERTF_H*/
