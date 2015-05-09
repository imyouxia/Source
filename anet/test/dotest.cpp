/**
 * @author Zhang Li
 * @date 2008-06-25 17:09:40
 * @version $Id: dotest.cpp 15405 2008-12-12 10:02:04Z zhangli $
 *
 * @Descriptions:
 * Testing Suite for ANet library
 */
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <anet/anet.h>
#include <anet/log.h>
#include <signal.h>

using namespace CppUnit;
using namespace std;

int main( int argc, char **argv)
{
    anet::Logger::logSetup();
    anet::Logger::setLogLevel("DEBUG");
    signal(SIGPIPE, SIG_IGN);
    TextUi::TestRunner runner;
    CompilerOutputter *outputter 
        = new CompilerOutputter(&runner.result(), std::cerr);
    runner.setOutputter(outputter);
    TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    bool ok = runner.run("", false);
    anet::Logger::logTearDown();
    return ok ? 0 : 1;
}
