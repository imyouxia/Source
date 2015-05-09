/**
 * File name: httpstreamertf.cpp
 * Author: zhangli
 * Create time: 2008-12-12 20:02:45
 * $Id: httpstreamertf.cpp 15798 2008-12-31 08:39:01Z zhangli $
 * 
 * Description: ***add description here***
 * 
 */

#include "httpstreamertf.h"
#include <anet/httpstreamer.h>
#include <anet/databuffer.h>
#include <anet/httpstreamingcontext.h>
#include <anet/ipacketfactory.h>
#include <anet/httppacketfactory.h>
#include <string>
#include <anet/packet.h>
#include <anet/httppacket.h>
#include <anet/log.h>
#include <anet/aneterror.h>
#include <sstream>

using namespace std;

namespace anet {
CPPUNIT_TEST_SUITE_REGISTRATION(HTTPStreamerTF);

HTTPStreamerTF::HTTPStreamerTF() {
    _factory = new HTTPPacketFactory;
    CPPUNIT_ASSERT(_factory);
}

HTTPStreamerTF::~HTTPStreamerTF() {
    delete _factory;
}

void HTTPStreamerTF::setUp() {
    CPPUNIT_ASSERT(_streamer = new HTTPStreamer(_factory));
    CPPUNIT_ASSERT(_dataBuffer = new DataBuffer);
    CPPUNIT_ASSERT(_context = new HTTPStreamingContext);
}

void HTTPStreamerTF::tearDown() {
    delete _streamer;
    delete _dataBuffer;
    delete _context;
}

void HTTPStreamerTF::testCreatePacket() {
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT(!_context->getPacket());

    _dataBuffer->writeBytes("GET /  HTTP/1.0\r\n", 17);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT(_context->getPacket());

    _dataBuffer->writeBytes("Host: localhost\r\n", 17); 
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT(_context->getPacket());
}

void HTTPStreamerTF::requestLineTokenMissing() { 
    const char *noURI = "GET HTTP/1.1\r\n";
    _dataBuffer->writeBytes(noURI, strlen(noURI));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();

    const char *noVersion = "GET /uri \r\n";
    _dataBuffer->writeBytes(noVersion, strlen(noVersion));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();

    const char *noMethod = "/abc HTTP/1.2\r\n";
    _dataBuffer->writeBytes(noMethod, strlen(noMethod));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::requestLineMoreToken() {
    ANET_LOG(DEBUG,"Start HTTPStreamerTF::requestLineMoreToken()");
    const char *moreToken = "GET /abc HTTP/1.1 EXTRA TOKEN\r\n";
    _dataBuffer->writeBytes(moreToken, strlen(moreToken));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    
    _dataBuffer->clear();
    _context->reset();

    const char *moreToken2 = "GET /abc  EXTRA HTTP/1.1\r\n";
    _dataBuffer->writeBytes(moreToken2, strlen(moreToken2));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::requestLineSpecialCharacter() {
    ANET_LOG(DEBUG,"Start  HTTPStreamerTF::requestLineSpecialCharacter()");
    const char *invalidURI = "HEAD /ab\000z HTTP/1.1\r\n";
    _dataBuffer->writeBytes(invalidURI, 21);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    //replace '\0' with SPAE
    CPPUNIT_ASSERT(packet->getURI());
    CPPUNIT_ASSERT_EQUAL(string("/ab z"), string(packet->getURI())); 
    _dataBuffer->clear();
    _context->reset();

    const char *invalidVersion = "HEAD /ab\000z HTTP/A.1\r\n";
    _dataBuffer->writeBytes(invalidVersion, 21);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    _dataBuffer->clear();
    _context->reset();

    const char *invalidMethod = "HE\000AD /ab\000z HTTP/1.0\r\n";
    _dataBuffer->writeBytes(invalidMethod, 22);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::requestLineEOF() {
    const char *twoTokens = "HEAD /abc";
    _dataBuffer->writeBytes(twoTokens, strlen(twoTokens));
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    
    _dataBuffer->clear();
    _context->reset();

    const char *missingLF = "HEAD /abc HTTP/1.1\r";
    _dataBuffer->writeBytes(missingLF, strlen(missingLF));
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testVariousMethod() {
    const char *optionsMethod = "OPTIONS * HTTP/1.1\r\n";
    _dataBuffer->writeBytes(optionsMethod, strlen(optionsMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_OPTIONS, packet->getMethod());
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, packet->getVersion());
    CPPUNIT_ASSERT(packet->getURI());
    CPPUNIT_ASSERT_EQUAL(string("*"), string(packet->getURI()));
    _dataBuffer->clear();
    _context->reset();

    const char *headMethod = "HEAD http://www.w3.org/i.html HTTP/1.0\n";
    _dataBuffer->writeBytes(headMethod, strlen(headMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, packet->getMethod());
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_0, packet->getVersion());
    CPPUNIT_ASSERT(packet->getURI());
    CPPUNIT_ASSERT_EQUAL(string("http://www.w3.org/i.html"), 
                         string(packet->getURI()));
    _dataBuffer->clear();
    _context->reset();

    const char *getMethod = "GET /abc.php?name=value HTTP/1.1\r\n";
    _dataBuffer->writeBytes(getMethod, strlen(getMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_GET, packet->getMethod()); 
    CPPUNIT_ASSERT(packet->getURI());
    CPPUNIT_ASSERT_EQUAL(string("/abc.php?name=value"),
                         string(packet->getURI()));
    _dataBuffer->clear();
    _context->reset();

    const char *postMethod = "POST /abc.php?name=v&sex=m HTTP/1.1\r\n";
    _dataBuffer->writeBytes(postMethod, strlen(postMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_POST, packet->getMethod());
    CPPUNIT_ASSERT(packet->getURI());
    CPPUNIT_ASSERT_EQUAL(string("/abc.php?name=v&sex=m"),
                         string(packet->getURI()));
    _dataBuffer->clear();
    _context->reset();

    const char *putMethod = "PUT /%20%AF%DE.asp HTTP/1.1\r\n";
    _dataBuffer->writeBytes(putMethod, strlen(putMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_PUT, packet->getMethod()); 
    CPPUNIT_ASSERT(packet->getURI());
    CPPUNIT_ASSERT_EQUAL(string("/%20%AF%DE.asp"), string(packet->getURI()));
    _dataBuffer->clear();
    _context->reset();

    const char *deleteMethod = "DELETE /abc HTTP/1.1\r\n";
    _dataBuffer->writeBytes(deleteMethod, strlen(deleteMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_DELETE, packet->getMethod());
    _dataBuffer->clear();
    _context->reset();   

    const char *traceMethod = "TRACE /abc HTTP/1.1\r\n";
    _dataBuffer->writeBytes(traceMethod, strlen(traceMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_TRACE, packet->getMethod());
    _dataBuffer->clear();
    _context->reset();

    const char *connectMethod = "CONNECT /abc HTTP/1.1\r\n";
    _dataBuffer->writeBytes(connectMethod, strlen(connectMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_CONNECT, packet->getMethod());
    _dataBuffer->clear();
    _context->reset();

    const char *unsupportedMethod = "UNSUPPORTED /abc HTTP/1.1\r\n";
    _dataBuffer->writeBytes(unsupportedMethod, strlen(unsupportedMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_UNSUPPORTED, packet->getMethod());
    CPPUNIT_ASSERT(packet->getMethodString());
    CPPUNIT_ASSERT_EQUAL(string("UNSUPPORTED"),
                         string(packet->getMethodString()));
    _dataBuffer->clear();
    _context->reset();

    const char *sarsMethod = "SARS /abc HTTP/1.1\r\n";
    _dataBuffer->writeBytes(sarsMethod, strlen(sarsMethod));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_UNSUPPORTED, packet->getMethod());
    CPPUNIT_ASSERT(packet->getMethodString());
    CPPUNIT_ASSERT_EQUAL(string("SARS"), string(packet->getMethodString()));
    _dataBuffer->clear();
    _context->reset();
}
 

void HTTPStreamerTF::testRequestLineMultiSteps() {
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("G", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("E", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("T", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes(" ", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("/ab", 3);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("c", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("  ", 2);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("HTTP/", 5);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("1.1", 3);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("\r", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("\n", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_REQUEST, packet->getPacketType()); 
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_GET, packet->getMethod()); 
    CPPUNIT_ASSERT(packet->getURI());
    CPPUNIT_ASSERT_EQUAL(string("/abc"), string(packet->getURI()));
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, packet->getVersion());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testErrorStatusLine() {
    ANET_LOG(DEBUG, "BEGIN testErrorStatusLine()");
    const char *invalidVersion = "HTTP/a 200 OK \r\n";
    _dataBuffer->writeBytes(invalidVersion, strlen(invalidVersion));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    _dataBuffer->clear();
    _context->reset();

    const char *invalidVersion2 = "HTTP/11.1 200 OK \r\n";
    _dataBuffer->writeBytes(invalidVersion2, strlen(invalidVersion2));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    _dataBuffer->clear();
    _context->reset();

    const char *invalidVersion3 = "HTTP/1.2 200 OK \r\n";
    _dataBuffer->writeBytes(invalidVersion3, strlen(invalidVersion2));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    _dataBuffer->clear();
    _context->reset();

    const char *invalidCode1 = "HTTP/1.1 OK \r\n";
    _dataBuffer->writeBytes(invalidCode1, strlen(invalidCode1));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    _dataBuffer->clear();
    _context->reset();

    const char *invalidCode2 = "HTTP/1.1 1 OK \r\n";
    _dataBuffer->writeBytes(invalidCode2, strlen(invalidCode2));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    _dataBuffer->clear();
    _context->reset();

    const char *invalidCode3 = "HTTP/1.1 1234 OK \r\n";
    _dataBuffer->writeBytes(invalidCode3, strlen(invalidCode3));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    _dataBuffer->clear();
    _context->reset();
    ANET_LOG(DEBUG,"END testErrorStatusLine()");
}

void HTTPStreamerTF::testStatusLine() {
    ANET_LOG(DEBUG, "BEGIN testStatusLine()");
    HTTPPacket *packet = NULL;
    const char *response1 = "HTTP/1.1 200 OK\r\n\r\n";
    _dataBuffer->writeBytes(response1, strlen(response1));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, packet->getVersion());
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_RESPONSE, packet->getPacketType());
    CPPUNIT_ASSERT_EQUAL(200, packet->getStatusCode());
    CPPUNIT_ASSERT_EQUAL(string("OK"), string(packet->getReasonPhrase()));
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;

    const char *response2 = "HTTP/1.0 404\r\n\r\n";
    _dataBuffer->writeBytes(response2, strlen(response2));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_0, packet->getVersion());
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_RESPONSE, packet->getPacketType());
    CPPUNIT_ASSERT_EQUAL(404, packet->getStatusCode());
    CPPUNIT_ASSERT_EQUAL(string("NOT FOUND"), string(packet->getReasonPhrase()));
    _dataBuffer->clear();
    _context->reset();
    _dataBuffer->clear();
    _context->reset();
    packet=NULL;
    
    const char *response3 = "HTTP/1.1 333\r\n\r\n";
    _dataBuffer->writeBytes(response3, strlen(response2));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, packet->getVersion());
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_RESPONSE, packet->getPacketType());
    CPPUNIT_ASSERT_EQUAL(333, packet->getStatusCode());
    CPPUNIT_ASSERT_EQUAL(string("UNKNOWN"), string(packet->getReasonPhrase()));
    _dataBuffer->clear();
    _context->reset();
    _dataBuffer->clear();
    _context->reset();
    packet=NULL;

    ANET_LOG(DEBUG,"END testStatusLine()");
}

void HTTPStreamerTF::testStatusLineMultiSteps() {
    ANET_LOG(DEBUG, "BEGIN testStatusLineMultiSteps()");

    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("H", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("T", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("TP/", 3);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("1", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());

    _dataBuffer->writeBytes(".", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("1", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes(" ", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("2", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("00", 2);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("  ", 2);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("O", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("K", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("!", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("\r", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("\n", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_RESPONSE, packet->getPacketType()); 
    CPPUNIT_ASSERT_EQUAL(200, packet->getStatusCode()); 
    CPPUNIT_ASSERT(packet->getReasonPhrase());
    CPPUNIT_ASSERT_EQUAL(string("OK!"), string(packet->getReasonPhrase()));
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, packet->getVersion());
    _dataBuffer->clear();
    _context->reset();
    ANET_LOG(DEBUG,"END testStatusLineMultiSteps()");
}

void HTTPStreamerTF::testErrorHeader() {
    ANET_LOG(DEBUG,"Start HTTPStreamerTF::testErrorHeader()");
    const char *requestLine = "HEAD /def HTTP/1.1\r\n";
    const char *CRLF = "\r\n";

    const char *errHeader1 = "Conne@ction: close\r\n";
    const char *errHeader2 = "Conne ction: close\r\n";
    const char *errHeader3 = "Connectionclose\r\n";
    const char *errHeader4 = "Connection : close\r\n";
    const char *errHeader5 = "Connection\t: close\r\n";
    const char *corrHeader1 = "Transfer-Encoding: chunked\r\n";
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes(errHeader1, strlen(errHeader1));
    _dataBuffer->writeBytes(corrHeader1, strlen(corrHeader1));
    _dataBuffer->writeBytes(errHeader2, strlen(errHeader2));
    _dataBuffer->writeBytes(errHeader3, strlen(errHeader3));
    _dataBuffer->writeBytes(errHeader4, strlen(errHeader4));
    _dataBuffer->writeBytes(errHeader5, strlen(errHeader5));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(packet->getHeader("transfer-encoding"));
    CPPUNIT_ASSERT_EQUAL(string("chunked"), 
                         string(packet->getHeader("transfer-encoding")));
    CPPUNIT_ASSERT(!packet->getHeader("Connection"));
    _dataBuffer->clear();
    _context->reset();

    ANET_LOG(DEBUG,"test error content length()");
    const char *errorContentLength = "Content-Length: aa\r\n";
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes(errorContentLength, strlen(errorContentLength));
    _dataBuffer->writeBytes(CRLF, strlen(CRLF));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    
    _dataBuffer->clear();
    _context->reset();

    const char *errorContentLength2 = "Content-Length: -30\r\n";
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes(errorContentLength2, strlen(errorContentLength2));
    _dataBuffer->writeBytes(CRLF, strlen(CRLF));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    
    _dataBuffer->clear();
    _context->reset();

    const char *errorTransferEncoding = "Transfer-Encoding: chunk\r\n";
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes(errorTransferEncoding, 
                            strlen(errorTransferEncoding));
    _dataBuffer->writeBytes(CRLF, strlen(CRLF));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testHeaderEOF() {
    ANET_LOG(DEBUG, "Begin testHeaderEOF()");
    const char *requestLine = "HEAD /def HTTP/1.1\n";   
    
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes("Connection", 10);
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes("Connection:", 11);
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();
    
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes("Connection: close", 17);
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes("Connection: close\r", 18);
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes("Connection: close\r\n", 19);
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();

    ANET_LOG(DEBUG, "Before broken request EOF");
    //if client close connection. we should set the connection broken
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes("\r\n", 2);
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    _dataBuffer->clear();
    _context->reset();
    ANET_LOG(DEBUG, "End testHeaderEOF()");
}


void HTTPStreamerTF::testCorrectHeader() {
    ANET_LOG(DEBUG, "testCorrectHeader()");
    const char *requestLine = "HEAD /def HTTP/1.1\n";
    const char *noBody = "Content-Length: 0\r\n\r\n";
    HTTPPacket *packet;

    const char *headerLF = "Connection: close\n";   
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes(headerLF, strlen(headerLF));
    _dataBuffer->writeBytes(noBody, strlen(noBody));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(packet->getHeader("connection"));
    CPPUNIT_ASSERT_EQUAL(string("close"),
                         string(packet->getHeader("connection")));
    _dataBuffer->clear();
    _context->reset();

    const char *emptyheader = "emptykey:\r\n";   
    const char *whiteSpaces = "key2: \t valu e\r\n";
    const char *nullCharacter = "key1: v\000alue\r\n";
    const char *colonInvalue = "key3:  val:ue \n";
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);

    _dataBuffer->writeBytes(emptyheader, strlen(emptyheader));
    _dataBuffer->writeBytes(whiteSpaces, strlen(whiteSpaces));
    _dataBuffer->writeBytes(nullCharacter, 14);
    _dataBuffer->writeBytes(colonInvalue, strlen(colonInvalue));
    _dataBuffer->writeBytes(noBody, strlen(noBody));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    CPPUNIT_ASSERT(packet->getHeader("emptykey"));
    CPPUNIT_ASSERT(packet->getHeader("emptykey"));
    CPPUNIT_ASSERT_EQUAL(string(""), string(packet->getHeader("emptykey")));
    //replace '\0' with ' '
    CPPUNIT_ASSERT(packet->getHeader("key1"));
    CPPUNIT_ASSERT_EQUAL(string("v alue"), string(packet->getHeader("key1")));
    CPPUNIT_ASSERT(packet->getHeader("key2"));
    CPPUNIT_ASSERT_EQUAL(string("valu e"), string(packet->getHeader("key2")));
    CPPUNIT_ASSERT(packet->getHeader("key3"));
    CPPUNIT_ASSERT_EQUAL(string("val:ue"), string(packet->getHeader("key3")));
    _dataBuffer->clear();
    _context->reset();

    //for ticket #146
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes("\r\n", 2);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testHeadersMultiSteps() {
    ANET_LOG(DEBUG, "Start testHeadersMultiSteps()");
    const char *noBody = "Content-Length: 0\r\n\r\n";
    const char *requestLine = "HEAD /def HTTP/1.1\r\n";

    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("C", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("onnectio", 8);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("n", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes(":", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes(" ", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("c", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("los", 3);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("e ", 2);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("\r", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes("\n", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());    

    _dataBuffer->writeBytes(noBody, strlen(noBody));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());    
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(packet->getHeader("connection"));
    CPPUNIT_ASSERT_EQUAL(string("close"),
                         string(packet->getHeader("connection")));
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, packet->getMethod());
    CPPUNIT_ASSERT(packet->getURI());
    CPPUNIT_ASSERT_EQUAL(string("/def"), string(packet->getURI()));
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, packet->getVersion());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testNoHeader() {
    ANET_LOG(DEBUG, "Start testNoHeader()");
    const char *statusLine = "HTTP/1.1 200 OK\r\n";
    const char *CRLF = "\n";
    _dataBuffer->writeBytes(statusLine, strlen(statusLine));
    _dataBuffer->writeBytes(CRLF,strlen(CRLF));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(_context->isEndOfFile());
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    ANET_LOG(DEBUG,"_context(%p)",_context);
    CPPUNIT_ASSERT(_context->isEndOfFile());
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isEndOfFile());
    CPPUNIT_ASSERT(_context->isCompleted());    
}

void HTTPStreamerTF::testFindCRLF() {
    ANET_LOG(DEBUG, "Start testFindCRLF");
    char *cr = NULL;
    size_t length = 0;
    char crlf[] = "\r\n";
    CPPUNIT_ASSERT(_streamer->findCRLF(crlf, &crlf[2], cr, length));
    CPPUNIT_ASSERT_EQUAL((size_t)2, length);
    CPPUNIT_ASSERT_EQUAL((void*)&crlf[0], (void*)cr);

    char lf[] = "\n";
    CPPUNIT_ASSERT(_streamer->findCRLF(lf, &lf[1], cr, length));
    CPPUNIT_ASSERT_EQUAL((size_t)1, length);
    CPPUNIT_ASSERT_EQUAL((void*)&lf[0], (void*)cr);

    char aZeroByte[] = "\000\r\nm";
    CPPUNIT_ASSERT(_streamer->findCRLF(aZeroByte, &aZeroByte[4], cr, length));
    CPPUNIT_ASSERT_EQUAL((size_t)3, length);
    CPPUNIT_ASSERT_EQUAL((void*)&aZeroByte[1], (void*)cr);

    char noLF[] = "\x1f \r\r\r";
    CPPUNIT_ASSERT(!_streamer->findCRLF(noLF, &noLF[5], cr, length));
    
    char twoLF[] = "abc\r\n\n\n";
    CPPUNIT_ASSERT(_streamer->findCRLF(twoLF, &twoLF[7], cr, length));
    CPPUNIT_ASSERT_EQUAL((size_t)5, length);
    CPPUNIT_ASSERT_EQUAL((void*)&twoLF[3], (void*)cr);
    CPPUNIT_ASSERT(_streamer->findCRLF(&twoLF[5], &twoLF[7], cr, length));
    CPPUNIT_ASSERT_EQUAL((size_t)1, length);
    CPPUNIT_ASSERT_EQUAL((void*)&twoLF[5], (void*)cr);
    CPPUNIT_ASSERT(_streamer->findCRLF(&twoLF[6], &twoLF[7], cr, length));
    CPPUNIT_ASSERT_EQUAL((size_t)1, length);
    CPPUNIT_ASSERT_EQUAL((void*)&twoLF[6], (void*)cr);

    char empty[] ="";
    CPPUNIT_ASSERT(!_streamer->findCRLF(empty, &empty[1], cr, length));
}

bool isTokenCharacter(char c) {
    if ( c >= 127 || c <= 31) {
        return false;
    } 
    if (c == '(' || c == ')' || c == '<' 
        || c == '>' || c == '@' || c == ',' 
        || c == ';' || c == ':' || c == '\\' 
        || c == '"' || c == '/' || c == '[' 
        || c == ']' || c == '?' || c == '=' 
        || c == '{' || c == '}' || c == ' ' 
        || c == '\t') {
        return false;
    }
    return true;
}

void HTTPStreamerTF::testIsTokenCharacter() {
    HTTPPacketFactory factory;
    HTTPStreamer streamer(&factory);
    for (int i = 0; i < 256; i++) {
        stringstream s1, s2;
        char c = i;
        s1 << "Character '" << c << "'(" << i << ") IS " ;
        s1 << (isTokenCharacter(c) ? "" : "NOT ");
        s1 << "a token character.";
        
        s2 << "Character '" << c << "'(" << i << ") IS " ;
        s2 << (streamer.isTokenCharacter(c) ? "" : "NOT ");
        s2 << "a token character.";

        CPPUNIT_ASSERT_EQUAL(s1.str(), s2.str());
    }
}

void HTTPStreamerTF::testURITooLarge() {
    const char *uri="/123456789";
    const char *method = "HEAD ";
    const char *version = " HTTP/1.1\r\n";
    _dataBuffer->writeBytes(method, strlen(method));
    for (int i = 0; i < 1024; i ++) {
        _dataBuffer->writeBytes(uri, 4);
    }
    _dataBuffer->writeBytes(version, strlen(version));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(method, strlen(method));
    for (int i = 0; i < 1024; i ++) {
        _dataBuffer->writeBytes(uri, 4);
    }
    _dataBuffer->writeBytes(uri,1);
    _dataBuffer->writeBytes(version, strlen(version));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT_EQUAL(AnetError::URI_TOO_LARGE, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::URI_TOO_LARGE_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testStartLineTooLarge() {
    for(int i = 0; i<1024*1024*8; i++) {
        _dataBuffer->writeBytes("01234567", 8);
    }
    _dataBuffer->writeBytes(" /a HTTP/1.1", 12);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();

    for(int i = 0; i<1024*1024*8; i++) {
        _dataBuffer->writeBytes("01234567", 8);
    }
    _dataBuffer->writeBytes(" /a HTTP/1.1\r\n", 14);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testHeaderTooLarge() {
    const char *requestLine="POST /123456789  HTTP/1.1\r\n"; //27
    _dataBuffer->writeBytes(requestLine, strlen(requestLine)); 
    _dataBuffer->writeBytes("key: ", 5);  //5
    for(int i = 0; i<1024*1024*8 - 4; i++) {
        _dataBuffer->writeBytes("01234567", 8);
    }
    _dataBuffer->writeBytes("\r\n", 2);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    for(int i = 0; i < 4; i++) {
        for (int j = 0; j < 1024 * 1024; j++) {
            _dataBuffer->writeBytes("key: 01234567890", 16);
        }
        _dataBuffer->writeBytes("\r\n", 16);
    }

    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testBodyTooLarge() {
    const char *requestLine="POST /123456789  HTTP/1.1\r\n";
    char contentLength[64];
    sprintf(contentLength, "Content-Length:%10lu\r\n\r\n",
            (67108864 - (10 + strlen("Content-Length:\r\n\r\n"))
             - strlen(requestLine)));
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes(contentLength, strlen(contentLength));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();

    sprintf(contentLength, "Content-Length:%10lu\r\n\r\n",
            (67108865 - (10 + strlen("Content-Length:\r\n\r\n"))
             - strlen(requestLine)));
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes(contentLength, strlen(contentLength));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testChunkTooLarge() {
    ANET_LOG(DEBUG, "Start testChunkTooLarge()");
    const char *statusLine = "HTTP/1.1 200 OK\r\n"; //17
    const char *encoding = "Transfer-Encoding: chunked\r\n\r\n"; //30
    const char *chunkToken1M = "0FFFF6\r\n";
    const char *chunkComplement = "0FFFBA\r\n";
    const char *lastChunk = "000000000\r\n\r\n"; // 13
    const char *lastChunk2= "0000000000\r\n\r\n"; // 14
    
    _dataBuffer->writeBytes(statusLine, strlen(statusLine));
    _dataBuffer->writeBytes(encoding, strlen(encoding));    
    for (int i = 0; i < 63; i++) {//63M data
        _dataBuffer->writeBytes(chunkToken1M, strlen(chunkToken1M));
        for (int j=1; j < 1024 * 1024 / 32; j ++) {
            _dataBuffer->writeBytes("zabcdefg12zabcdefg12zabcdefg12df",32);
        }
        _dataBuffer->writeBytes("zabcdefg12zabcdefg12zabcdefg12df",22);
        _dataBuffer->writeBytes("\r\n",2);
    }
    _dataBuffer->writeBytes(chunkComplement, strlen(chunkComplement));
    for (int j=3; j < 1024 * 1024 / 32; j ++) {//1M - 64byte
        _dataBuffer->writeBytes("zabcdefg12zabcdefg12zabcdefg12df",32);
    }
    _dataBuffer->writeBytes("zabcdefg12zabcdefg12zabcdefg12df",22);
    _dataBuffer->writeBytes("abcd\r\n",6);
    _dataBuffer->writeBytes(lastChunk, strlen(lastChunk));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(statusLine, strlen(statusLine));
    _dataBuffer->writeBytes(encoding, strlen(encoding));    
    for (int i = 0; i < 63; i++) {//63M data
        _dataBuffer->writeBytes(chunkToken1M, strlen(chunkToken1M));
        for (int j=1; j < 1024 * 1024 / 32; j ++) {
            _dataBuffer->writeBytes("zabcdefg12zabcdefg12zabcdefg12df",32);
        }
        _dataBuffer->writeBytes("zabcdefg12zabcdefg12zabcdefg12df",22);
        _dataBuffer->writeBytes("\r\n",2);
    }
    _dataBuffer->writeBytes(chunkComplement, strlen(chunkComplement));
    for (int j=3; j < 1024 * 1024 / 32; j ++) {//1M - 64byte
        _dataBuffer->writeBytes("zabcdefg12zabcdefg12zabcdefg12df",32);
    }
    _dataBuffer->writeBytes("zabcdefg12zabcdefg12zabcdefg12df",22);
    _dataBuffer->writeBytes("abcd\r\n",6);
    _dataBuffer->writeBytes(lastChunk2, strlen(lastChunk2));
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();

}

void HTTPStreamerTF::testTooManyHeaders() {
    const char *requestLine="POST /123456789  HTTP/1.1\r\n"; //27
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    for(size_t i = 0; i < HTTPStreamer::HEADERS_LIMIT; i++) {
        _dataBuffer->writeBytes("key: 012345678\r\n", 16);
    }
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());

    _dataBuffer->writeBytes("key: 012345678\r\n", 16);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());

    CPPUNIT_ASSERT_EQUAL(AnetError::TOO_MANY_HEADERS, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::TOO_MANY_HEADERS_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testErrorBody() {
    ANET_LOG(DEBUG, "testErrorBody()");
    const char *postRequest = "POST /def HTTP/1.1\n";
    const char *noBody = "Content-Length: 0\r\n\n";
    const char *oneByteBody = "Content-Length: 1\n\r\n";
    const char *octets = "0123456789abcd\r\n";
    HTTPPacket *packet = NULL;
    size_t bodyLength = 0;

    _dataBuffer->writeBytes(postRequest, strlen(postRequest));
    _dataBuffer->writeBytes(noBody, strlen(noBody));
    _dataBuffer->writeBytes(octets, 16);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(!packet->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL(16, _dataBuffer->getDataLen());
    _context->reset();
    packet = NULL;

    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
    
    _dataBuffer->writeBytes(postRequest, strlen(postRequest));
    _dataBuffer->writeBytes(oneByteBody, strlen(oneByteBody));
    _dataBuffer->writeBytes(octets, 16);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    _context->reset();
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
    
    _dataBuffer->writeBytes(postRequest, strlen(postRequest));
    _dataBuffer->writeBytes(oneByteBody, strlen(oneByteBody));
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::CONNECTION_CLOSED, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::CONNECTION_CLOSED_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testErrorChunkBody() {
    ANET_LOG(DEBUG, "testErrorChunkBody()");
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n"; //30
    const char *transferEncoding = "Transfer-Encoding: chunked\r\n\r\n";//30
    const char *invalidChunk = "-1234\r\n";
    const char *invalidChunk2 = "12x34\r\n";
    const char *chunk64M = "3FFFFBB\r\n";//9
    const char *chunk64MPlusOne = "3FFFFBC\r\n";//9
    const char *octets = "zHIGJKLmnopq0123456789abcdef\r\n";//30

    
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(chunk64M, strlen(chunk64M));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(chunk64MPlusOne, strlen(chunk64MPlusOne));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(octets, 30);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
    
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    for (size_t i = 0; i < 1204 * 1024 * 4; i ++) {
        _dataBuffer->writeBytes(octets, 16);
    }
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::PKG_TOO_LARGE_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(invalidChunk, strlen(invalidChunk));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
    
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(invalidChunk2, strlen(invalidChunk2));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testChunkBodyErrorLength() {
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n"; //30
    const char *transferEncoding = "Transfer-Encoding: chunked\r\n\r\n";//30
    const char *chunkNormal = "10;d=4;a\r\n";
    const char *octets = "zHIGJKLmnopq0123456789abcdef\r\n";//30
    
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(chunkNormal, strlen(chunkNormal));
    _dataBuffer->writeBytes(octets,9);
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::CONNECTION_CLOSED, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::CONNECTION_CLOSED_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();

    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(chunkNormal, strlen(chunkNormal));
    _dataBuffer->writeBytes(octets,20);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testErrorLastChunk() {
    ANET_LOG(DEBUG, "Start testErrorLastChunk()");
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n"; //30
    const char *transferEncoding = "Transfer-Encoding: chunked\r\n\r\n";//30
    const char *chunkNormal = "10;d=4;a\r\n";
    const char *octets = "zHIGJKLmnopq0123456789abcdef\r\n";//30
    
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(chunkNormal, strlen(chunkNormal));
    _dataBuffer->writeBytes(octets, 16);
    _dataBuffer->writeBytes("\n", 1);
    _dataBuffer->writeBytes("\n", 1);
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::INVALID_DATA_S,
                         _context->getErrorString());
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testErrorTrailer() {
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n"; //30
    const char *transferEncoding = "Transfer-Encoding: chunked\r\n\r\n";//30
    const char *chunkNormal = "10;d=4;a\r\n";
    const char *octets = "zHIGJKLmnopq0123456789abcdef\r\n";//30
    const char *lastChunk = "0;d=4;a\r\n";

    const char *errTrailer1 = "Conne@ction: close\r\n";
    const char *errTrailer2 = "Conne ction: close\r\n";
    const char *errTrailer3 = "Connectionclose\r\n";
    const char *errTrailer4 = "Connection : close\r\n";
    const char *errTrailer5 = "Connection\t: close\r\n";
    const char *corrTrailer1 = "Key1: value1\r\n";
    const char *value1 = "value1";
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(chunkNormal, strlen(chunkNormal));
    _dataBuffer->writeBytes(octets, 16);
    _dataBuffer->writeBytes("\r\n", 2);
    _dataBuffer->writeBytes(lastChunk, strlen(lastChunk));

    _dataBuffer->writeBytes(errTrailer1, strlen(errTrailer1));
    _dataBuffer->writeBytes(corrTrailer1, strlen(corrTrailer1));
    _dataBuffer->writeBytes(errTrailer2, strlen(errTrailer2));
    _dataBuffer->writeBytes(errTrailer3, strlen(errTrailer3));
    _dataBuffer->writeBytes(errTrailer4, strlen(errTrailer4));
    _dataBuffer->writeBytes(errTrailer5, strlen(errTrailer5));
    
    _dataBuffer->writeBytes("\r\n", 2);
    
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(!packet->getHeader("Connection"));
    CPPUNIT_ASSERT(packet->getHeader("Key1"));
    CPPUNIT_ASSERT_EQUAL(string(value1), string(packet->getHeader("Key1")));
    _dataBuffer->clear();
    _context->reset();
}

void HTTPStreamerTF::testNoBody() {
    const char *requestLine="POST /123456789  HTTP/1.1\r\n"; //27
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n"; //30
    const char *noBody = "Content-Length: 0\n\r\n";
    const char *transferEncoding = "Transfer-Encoding: chunked\r\n\r\n";//30
    const char *lastChunk = "0;d=4;a\r\n";
    size_t bodyLength = 0;
    _dataBuffer->writeBytes(requestLine, strlen(requestLine));
    _dataBuffer->writeBytes(noBody, strlen(noBody));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(!packet->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL((size_t)0, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;

    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes("\r\n", 2);
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(!packet->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL((size_t)0, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;
    
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(lastChunk, strlen(lastChunk));
    _dataBuffer->writeBytes("\r\n", 2);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(!packet->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL((size_t)0, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;
}  

void HTTPStreamerTF::testStopPosition() {
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n"; //30
    const char *contentLength = "Content-Length: 3\n\r\n";
    size_t bodyLength = 0;
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(contentLength, strlen(contentLength));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("a", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("b", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("c", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(memcmp("abc", packet->getBody(bodyLength), 3) == 0);
    CPPUNIT_ASSERT_EQUAL((size_t)3, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;

    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(contentLength, strlen(contentLength));
    _dataBuffer->writeBytes("abcd", 4);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(memcmp("abc", packet->getBody(bodyLength), 3) == 0);
    CPPUNIT_ASSERT_EQUAL((size_t)3, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;

    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(contentLength, strlen(contentLength));
    _dataBuffer->writeBytes("abcdooo", 7);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(memcmp("abc", packet->getBody(bodyLength), 3) == 0);
    CPPUNIT_ASSERT_EQUAL((size_t)3, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;
}

void HTTPStreamerTF::testContentLength() {
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n"; //30
    const char *contentLength = "Content-Length: 1\n\r\n";
    const char *contentLength2 = "Content-Length: 010\n\r\n";
    size_t bodyLength = 0;
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(contentLength, strlen(contentLength));
    _dataBuffer->writeBytes("a", 1);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(memcmp("a", packet->getBody(bodyLength), 1) == 0);
    CPPUNIT_ASSERT_EQUAL((size_t)1, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;

    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(contentLength2, strlen(contentLength2));
    _dataBuffer->writeBytes("0123456789", 10);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(memcmp("0123456789", packet->getBody(bodyLength), 10) 
                   == 0);
    CPPUNIT_ASSERT_EQUAL((size_t)10, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;
}

void HTTPStreamerTF::testChunkedLength() {
    ANET_LOG(DEBUG,"Start testChunkedLength()");
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n";
    const char *transferEncoding = "Transfer-Encoding: chunked\r\n\r\n";
    const char *chunk1 = "1\r\na\r\n";
    const char *chunk2 = "a;james=zhangli;a\r\n1234567890\n";
    const char *chunk3 = "7;james=zhangli;a\r\nabcdefg\n";
    const char *lastChunk = "0;d=4;a\r\n\r\n";
    size_t bodyLength = 0;
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(chunk1, strlen(chunk1));
    _dataBuffer->writeBytes(lastChunk, strlen(lastChunk));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(memcmp("a", packet->getBody(bodyLength), 1) == 0);
    CPPUNIT_ASSERT_EQUAL((size_t)1, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;

    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(chunk1, strlen(chunk1));
    _dataBuffer->writeBytes(chunk2, strlen(chunk2));
    _dataBuffer->writeBytes(chunk3, strlen(chunk3));
    _dataBuffer->writeBytes(lastChunk, strlen(lastChunk));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(memcmp("a1234567890abcdefg", 
                          packet->getBody(bodyLength), 18)== 0);
    CPPUNIT_ASSERT_EQUAL((size_t)18, bodyLength);
    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
}

void HTTPStreamerTF::testChunkStopPosition() {
    ANET_LOG(DEBUG,"Start testChunkStopPosition()");
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n";
    const char *transferEncoding = "Transfer-Encoding: chunked\r\n\r\n";
    const char *chunk2 = "a;james=zhangli;a\r\n0123456789\n";
    const char *chunk3 = "7;james=zhangli;a\nabcdefg\r\n";
    const char *chunk4 = "a\r\n0123456789\n";
    size_t bodyLength = 0;
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("00", 2); //[00]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("2;", 2);//[002;]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("a=b", 3);//[002;a=b]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("f\r", 2);//[002;a=bf\r]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("\n", 1);//[002;a=bf\r\n]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("a", 1);//[002;a=bf\r\na]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("b", 1);//[002;a=bf\r\nab]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("\r", 1);//[002;a=bf\r\nab\r]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    ANET_LOG(DEBUG, "last byte in chunk1");
    _dataBuffer->writeBytes("\n", 1);//[002;a=bf\r\nab\r\n]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());//first chunk end!
    ANET_LOG(DEBUG, "chunk 2 begin");
    _dataBuffer->writeBytes(chunk2, strlen(chunk2));
    _dataBuffer->writeBytes(chunk3, strlen(chunk3));
    _dataBuffer->writeBytes(chunk4, strlen(chunk4));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    
    //test stop in last chunk
    _dataBuffer->writeBytes("0", 1);//[0]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes(";", 1);//[0;]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("james", 5);//[0;james]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("=", 1);//[0;james=]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("a;",1);//[0;james=a;]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("1=1",3);//[0;james=a;1=1]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("\r\n",2);//[0;james=a;1=1\r\n]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());

    //test stop in trailer
    _dataBuffer->writeBytes("key1: value1\r\n", 14);
    _dataBuffer->writeBytes("k2", 2);  //[k2]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes(":", 1);   //[k2:]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes(" \t", 2); //[k2: \t]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("v2", 2);  //[k2: \tv2]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes(" ", 1);   //[k2: \tv2 ]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("\r", 1);  //[k2: \tv2 \r]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes("\n", 1);  //[k2: \tv2 \r\n]
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());

    _dataBuffer->writeBytes("\r\n",2);
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(_context->isCompleted());
    HTTPPacket *packet = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(memcmp("ab0123456789abcdefg0123456789", 
                          packet->getBody(bodyLength), 29) == 0);
    CPPUNIT_ASSERT_EQUAL((size_t)29, bodyLength);
    CPPUNIT_ASSERT(packet->getHeader("key1"));
    CPPUNIT_ASSERT_EQUAL(string("value1"), string(packet->getHeader("key1")));
    CPPUNIT_ASSERT(packet->getHeader("k2"));
    CPPUNIT_ASSERT_EQUAL(string("v2"), string(packet->getHeader("k2")));

    _dataBuffer->clear();
    _context->reset();
    packet = NULL;
    bodyLength = 0;
}

void HTTPStreamerTF::testChunkTooManyTrailers() {
    ANET_LOG(DEBUG, "Start testChunkTooManyTrailers()");
    const char *responseLine = "HTTP/1.1 200 OK Chunked Body\r\n";
    const char *transferEncoding = "Transfer-Encoding: chunked\r\n\r\n";
    const char *chunk = "2;james=zhangli;a\r\n12\n0\n";
    const char *trailer = "k:v\n";
    _dataBuffer->writeBytes(responseLine, strlen(responseLine));
    _dataBuffer->writeBytes(transferEncoding, strlen(transferEncoding));
    _dataBuffer->writeBytes(chunk, strlen(chunk));
    //we had one header transferEncoding
    for (size_t i = 1; i < HTTPStreamer::HEADERS_LIMIT; i ++) {
        _dataBuffer->writeBytes(trailer, strlen(trailer));
    }
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    _dataBuffer->writeBytes(trailer, strlen(trailer));
    CPPUNIT_ASSERT(!_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isBroken());
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT_EQUAL(AnetError::TOO_MANY_HEADERS, _context->getErrorNo());
    CPPUNIT_ASSERT_EQUAL(AnetError::TOO_MANY_HEADERS_S,
                         _context->getErrorString());
}

}/*end namespace anet*/
