#include "httpcomponenttf.h"

namespace anet{
CPPUNIT_TEST_SUITE_REGISTRATION(HTTPComponentTF);

void HTTPComponentTF::setUp()
{
}

void HTTPComponentTF::tearDown()
{
}

void HTTPComponentTF::testCreateConnection() {
/*    Transport tran;
    Socket *socket = new Socket();
    HTTPComponent httpComponent(&tran, socket);
    IPacketStreamer *fakeStreamer = new DefaultPacketStreamer;
    IServerAdapter *fakeAdapter = (IServerAdapter*)0xfa43212;
    CPPUNIT_ASSERT(socket->setAddress("localhost",12345));
    CPPUNIT_ASSERT(!httpComponent.createConnection(NULL, NULL));
    CPPUNIT_ASSERT(httpComponent.init(true));
    CPPUNIT_ASSERT(!httpComponent.createConnection(fakeStreamer,NULL));
    CPPUNIT_ASSERT(!httpComponent.createConnection(fakeStreamer, NULL));
    Connection *conn = httpComponent.createConnection(fakeStreamer, fakeAdapter);
    CPPUNIT_ASSERT(conn);
    CPPUNIT_ASSERT( dynamic_cast<HTTPConnection *>(conn));
    CPPUNIT_ASSERT(conn->isServer());*/
    CPPUNIT_ASSERT(false);
}
    
}
