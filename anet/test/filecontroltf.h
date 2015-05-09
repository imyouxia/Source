/**
 * $Id:$
 */
   
#ifndef FILECONTROLTF_H_
#define FILECONTROLTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>
#include <anet/filecontrol.h>

namespace anet {
class FileControlTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(FileControlTF);
    CPPUNIT_TEST(testIsNotCloseOnExec);
    CPPUNIT_TEST(testIsCloseOnExec);
    CPPUNIT_TEST(testIsCloseOnExecNegtiveFd);
    CPPUNIT_TEST(testIsCloseOnExecNegtiveTooLarge);
    CPPUNIT_TEST(testClearCloseOnExec);
    CPPUNIT_TEST(testClearCloseOnExecNegtiveFd);
    CPPUNIT_TEST(testClearCloseOnExecNegtiveTooLarge);
    CPPUNIT_TEST(testSetCloseOnExec);
    CPPUNIT_TEST(testSetCloseOnExecNegtiveFd);
    CPPUNIT_TEST(testSetCloseOnExecNegtiveTooLarge);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testIsNotCloseOnExec();
    void testIsCloseOnExec();
    void testIsCloseOnExecNegtiveFd();
    void testIsCloseOnExecNegtiveTooLarge();
    void testClearCloseOnExec();
    void testClearCloseOnExecNegtiveFd();
    void testClearCloseOnExecNegtiveTooLarge();
    void testSetCloseOnExec();
    void testSetCloseOnExecNegtiveFd();
    void testSetCloseOnExecNegtiveTooLarge();
private:
    int _pipes[2];
};
}

#endif /*FILECONTROLTF_H_*/
