#include "filecontroltf.h"

using namespace std;

namespace anet {
    
CPPUNIT_TEST_SUITE_REGISTRATION(FileControlTF);

void FileControlTF::setUp() {
    int rc = pipe(_pipes);
    CPPUNIT_ASSERT_EQUAL(0, rc);
    CPPUNIT_ASSERT(FileControl::clearCloseOnExec(_pipes[0]));
    CPPUNIT_ASSERT(FileControl::setCloseOnExec(_pipes[1]));
}

void FileControlTF::tearDown() {
    close(_pipes[0]);
    close(_pipes[1]);
}

void FileControlTF::testIsNotCloseOnExec() {
    CPPUNIT_ASSERT(!FileControl::isCloseOnExec(_pipes[0]));
}

void FileControlTF::testIsCloseOnExec() {
    CPPUNIT_ASSERT(FileControl::isCloseOnExec(_pipes[1]));
}

void FileControlTF::testIsCloseOnExecNegtiveFd() {
    CPPUNIT_ASSERT(!FileControl::isCloseOnExec(-1));
}

void FileControlTF::testIsCloseOnExecNegtiveTooLarge() {
    CPPUNIT_ASSERT(!FileControl::isCloseOnExec(1024000));
}

void FileControlTF::testClearCloseOnExec() {
    CPPUNIT_ASSERT(FileControl::clearCloseOnExec(_pipes[1]));
    CPPUNIT_ASSERT(!FileControl::isCloseOnExec(_pipes[1]));
}

void FileControlTF::testClearCloseOnExecNegtiveFd() {
    CPPUNIT_ASSERT(!FileControl::clearCloseOnExec(-1));
}

void FileControlTF::testClearCloseOnExecNegtiveTooLarge() {
    CPPUNIT_ASSERT(!FileControl::clearCloseOnExec(1024000));
}

void FileControlTF::testSetCloseOnExec() {
    CPPUNIT_ASSERT(FileControl::setCloseOnExec(_pipes[0]));
    CPPUNIT_ASSERT(FileControl::isCloseOnExec(_pipes[0]));
}

void FileControlTF::testSetCloseOnExecNegtiveFd() {
    CPPUNIT_ASSERT(!FileControl::setCloseOnExec(-1));
}

void FileControlTF::testSetCloseOnExecNegtiveTooLarge() {
    CPPUNIT_ASSERT(!FileControl::setCloseOnExec(1024000));
}

}




