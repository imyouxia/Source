#include "httpstreamingcontexttf.h"
#include <anet/httpstreamingcontext.h>
#include <anet/aneterror.h>

namespace anet {
CPPUNIT_TEST_SUITE_REGISTRATION(HTTPStreamingContextTF);

void HTTPStreamingContextTF::setUp() {}

void HTTPStreamingContextTF::tearDown() {}

void HTTPStreamingContextTF::testHTTPStreamingContext() {
}

void HTTPStreamingContextTF::testSetGetErrorNo() {
    HTTPStreamingContext *context = new HTTPStreamingContext;
    CPPUNIT_ASSERT_EQUAL(0, context->getErrorNo());
    context->setErrorNo(AnetError::PKG_TOO_LARGE);
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE, context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE_S,
                         context->getErrorString());
    context->setErrorNo(AnetError::LENGTH_REQUIRED);
    CPPUNIT_ASSERT_EQUAL(AnetError::LENGTH_REQUIRED, context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::LENGTH_REQUIRED_S,
                         context->getErrorString());
    context->setErrorNo(AnetError::VERSION_NOT_SUPPORT);
    CPPUNIT_ASSERT_EQUAL(AnetError::VERSION_NOT_SUPPORT,
                         context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::VERSION_NOT_SUPPORT_S,
                         context->getErrorString());
    context->setErrorNo(-400);
    CPPUNIT_ASSERT_EQUAL(-400, context->getErrorNo());
    CPPUNIT_ASSERT(!context->getErrorString());
    delete context;
}

void HTTPStreamingContextTF::testSetGetErrorString() {
    StreamingContext *context = new StreamingContext;
    CPPUNIT_ASSERT_EQUAL((const char*)NULL, context->getErrorString());
    context->setErrorString("");
    CPPUNIT_ASSERT_EQUAL((const char*)"", context->getErrorString());
    context->setErrorString("abc");
    CPPUNIT_ASSERT_EQUAL((const char*)"abc", context->getErrorString());
    context->setErrorString(AnetError::LENGTH_REQUIRED_S);
    CPPUNIT_ASSERT_EQUAL(AnetError::LENGTH_REQUIRED_S, context->getErrorString());
    delete context;
}

}/*end namespace anet*/
