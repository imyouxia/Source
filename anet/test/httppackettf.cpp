#include <string>
#include <anet/log.h>
#include "httppackettf.h"
#include <anet/httpstreamingcontext.h>
#include <anet/httpstreamer.h>
#include <anet/httppacketfactory.h>

using namespace std;
namespace anet {
CPPUNIT_TEST_SUITE_REGISTRATION(HTTPPacketTF);

void HTTPPacketTF::setUp() {
    _packet = new HTTPPacket;
    CPPUNIT_ASSERT(_packet);
    _factory = new HTTPPacketFactory;
    CPPUNIT_ASSERT(_factory);
    _streamer = new HTTPStreamer(_factory);
    CPPUNIT_ASSERT(_streamer);
    _context = dynamic_cast<HTTPStreamingContext*>(_streamer->createContext());
    CPPUNIT_ASSERT(_context);
    _dataBuffer = new DataBuffer;
    CPPUNIT_ASSERT(_dataBuffer);
}

void HTTPPacketTF::tearDown() {
    if (_packet) {
        delete _packet;
    }
    delete _context;
    delete _dataBuffer;
    delete _streamer;
    delete _factory;
}

void HTTPPacketTF::testSetGetPacketType() {
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_INVALID, _packet->getPacketType());
    _packet->setPacketType(HTTPPacket::PT_REQUEST);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_REQUEST, _packet->getPacketType());
    _packet->setPacketType(HTTPPacket::PT_RESPONSE);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_RESPONSE, _packet->getPacketType());
    _packet->setPacketType((HTTPPacket::HTTPPacketType)99999);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_INVALID, _packet->getPacketType());
    _packet->setPacketType((HTTPPacket::HTTPPacketType)-1);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_INVALID, _packet->getPacketType());
}

void HTTPPacketTF::testSetGetVersion() {
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, _packet->getVersion()); 
    _packet->setVersion(HTTPPacket::HTTP_1_0);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_0, _packet->getVersion());
    _packet->setVersion(HTTPPacket::HTTP_1_1);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, _packet->getVersion());    
    _packet->setVersion((HTTPPacket::HTTPVersion)-1);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_UNSUPPORTED, _packet->getVersion()); 
    _packet->setVersion((HTTPPacket::HTTPVersion)999999);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_UNSUPPORTED, _packet->getVersion()); 
}

void HTTPPacketTF::testSetGetMethod() {    
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_UNSUPPORTED, _packet->getMethod()); 
    _packet->setMethod("OPTIONS");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_OPTIONS, _packet->getMethod());
    _packet->setMethod("HEAD");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, _packet->getMethod());
    _packet->setMethod("GET");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_GET, _packet->getMethod());    
    _packet->setMethod("POST");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_POST, _packet->getMethod());
    _packet->setMethod("PUT");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_PUT, _packet->getMethod());    
    _packet->setMethod("DELETE");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_DELETE, _packet->getMethod());
    _packet->setMethod("TRACE");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_TRACE, _packet->getMethod());    
    _packet->setMethod("CONNECT");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_CONNECT, _packet->getMethod());
    _packet->setMethod("UNSUPPORTED");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_UNSUPPORTED, _packet->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("UNSUPPORTED"),
                         string(_packet->getMethodString()));
    _packet->setMethod("NULL");
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_UNSUPPORTED, _packet->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("NULL"), string(_packet->getMethodString()));

    _packet->setMethod(HTTPPacket::HM_OPTIONS);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_OPTIONS, _packet->getMethod());
    _packet->setMethod(HTTPPacket::HM_HEAD);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, _packet->getMethod());
    _packet->setMethod(HTTPPacket::HM_GET);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_GET, _packet->getMethod());    
    _packet->setMethod(HTTPPacket::HM_POST);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_POST, _packet->getMethod());
    _packet->setMethod(HTTPPacket::HM_PUT);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_PUT, _packet->getMethod());    
    _packet->setMethod(HTTPPacket::HM_DELETE);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_DELETE, _packet->getMethod());
    _packet->setMethod(HTTPPacket::HM_TRACE);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_TRACE, _packet->getMethod());    
    _packet->setMethod(HTTPPacket::HM_CONNECT);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_CONNECT, _packet->getMethod());
    _packet->setMethod(HTTPPacket::HM_UNSUPPORTED);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_UNSUPPORTED, _packet->getMethod());
    _packet->setMethod((HTTPPacket::HTTPMethod)-1);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_UNSUPPORTED, _packet->getMethod()); 
    _packet->setMethod((HTTPPacket::HTTPMethod)999999);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_UNSUPPORTED, _packet->getMethod()); 
}

void HTTPPacketTF::testSideEffect() {
    _packet->setMethod(HTTPPacket::HM_GET);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_REQUEST, _packet->getPacketType());
    _packet->setStatusCode(505);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_RESPONSE, _packet->getPacketType());
}

void HTTPPacketTF::testSetGetURI() {
    string uri = "http://localhost:9090/1.php?abd";
    string uri2 = "http://localhost:9090/1.php?abd  ";
    string uri3 = " *";
    string urit = "http://localhost:9090/1.php?abd	";
    string uriroot = "/";
    string empty = "";
    string spaces = "  ";
    char *null = NULL;

    CPPUNIT_ASSERT(!_packet->getURI());
    _packet->setURI(uri.c_str());
    CPPUNIT_ASSERT_EQUAL(uri, string(_packet->getURI()));
    _packet->setURI(uri2.c_str());
    CPPUNIT_ASSERT_EQUAL(uri, string(_packet->getURI()));
    _packet->setURI(urit.c_str());
    CPPUNIT_ASSERT_EQUAL(urit, string(_packet->getURI()));
    _packet->setURI(uri3.c_str());
    CPPUNIT_ASSERT_EQUAL(string("*"), string(_packet->getURI()));
    _packet->setURI(empty.c_str());
    CPPUNIT_ASSERT(!_packet->getURI());
    _packet->setURI(null);
    CPPUNIT_ASSERT(!_packet->getURI());
    _packet->setURI(spaces.c_str());
    CPPUNIT_ASSERT(!_packet->getURI());
    _packet->setURI(uriroot.c_str());
    CPPUNIT_ASSERT_EQUAL(uriroot, string(_packet->getURI()));
}

// void HTTPPacketTF::testSetGetBody() {
//     string body = "http://localhost:9090/1.php?a\x1fvery looooooooooooooong!";
//     string spaces = "  ";
//     char* const null = NULL;
//     char five[5] = { '3', '\0', 'a', '\t', ' ' };
//     size_t len;
//     const char *result = NULL;

//     CPPUNIT_ASSERT(!_packet->getBody(len));
//     CPPUNIT_ASSERT_EQUAL((size_t)0, len);

//     CPPUNIT_ASSERT(_packet->setBody(spaces.c_str(), spaces.length()));
//     CPPUNIT_ASSERT(_packet->getBody(len));
//     CPPUNIT_ASSERT_EQUAL(spaces.length(), len);
    
//     CPPUNIT_ASSERT(!_packet->setBody(null,3));
//     CPPUNIT_ASSERT(_packet->getBody(len));
//     CPPUNIT_ASSERT_EQUAL(spaces.length(), len);

//     CPPUNIT_ASSERT(_packet->setBody(null,0));
//     CPPUNIT_ASSERT(!_packet->getBody(len));
//     CPPUNIT_ASSERT_EQUAL((size_t)0, len);

//     CPPUNIT_ASSERT(_packet->setBody(five, 5));
//     result = _packet->getBody(len);
//     CPPUNIT_ASSERT(result);
//     CPPUNIT_ASSERT_EQUAL((size_t)5, len);
//     CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));

//     CPPUNIT_ASSERT(_packet->setBody(body.c_str(),0));
//     CPPUNIT_ASSERT(!_packet->getBody(len));
//     CPPUNIT_ASSERT_EQUAL((size_t)0, len);

//     CPPUNIT_ASSERT(_packet->appendBody(five, 5));
//     result = _packet->getBody(len);
//     CPPUNIT_ASSERT_EQUAL((size_t)5, len);
//     CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));

//     CPPUNIT_ASSERT(!_packet->appendBody(null, 4));
//     result = _packet->getBody(len);
//     CPPUNIT_ASSERT_EQUAL((size_t)5, len);
//     CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));

//     CPPUNIT_ASSERT(_packet->appendBody(body.c_str(), body.length()));
//     result = _packet->getBody(len);
//     CPPUNIT_ASSERT_EQUAL(body.length() + 5, len);
//     CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));
//     CPPUNIT_ASSERT_EQUAL(0, memcmp(result + 5, body.c_str(), body.length()));

//     CPPUNIT_ASSERT(!_packet->appendBody(five, 0));
//     result = _packet->getBody(len);
//     CPPUNIT_ASSERT_EQUAL(body.length() + 5, len);
//     CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));
//     CPPUNIT_ASSERT_EQUAL(0, memcmp(result + 5, body.c_str(), body.length()));
// }

void HTTPPacketTF::testAddGetHeader() {
    string key = "Host";
    string key2 = "hoSt";
    string key22 = "hoSt";
    string value = "localhost";
    string emptykey = "emptykey";
    string empty = "";
    char * null = NULL;

    CPPUNIT_ASSERT(!_packet->addHeader(null, value.c_str()));
    CPPUNIT_ASSERT(!_packet->getHeader(null));

    CPPUNIT_ASSERT(_packet->addHeader(key.c_str(), value.c_str()));
    CPPUNIT_ASSERT(_packet->getHeader(key.c_str()));
    CPPUNIT_ASSERT_EQUAL(value, string(_packet->getHeader(key.c_str())));
    CPPUNIT_ASSERT(_packet->getHeader(key2.c_str()));
    CPPUNIT_ASSERT_EQUAL(value, string(_packet->getHeader(key2.c_str())));
    CPPUNIT_ASSERT_EQUAL(key22, key2);

    key = "Connection";
    value = "close";
    ANET_LOG(SPAM,"before addHeader(%s)", key2.c_str());
    _packet->addHeader(key.c_str(), value.c_str());
    CPPUNIT_ASSERT_EQUAL(value, string(_packet->getHeader(key.c_str())));

    key = "Host";
    value = "";
    _packet->addHeader(key.c_str(), value.c_str());
    CPPUNIT_ASSERT_EQUAL(value, string(_packet->getHeader(key.c_str())));
    CPPUNIT_ASSERT(! _packet->getHeader("NoSuchKey"));

    value = "NewValue";
    _packet->addHeader(key2.c_str(), value.c_str());
    CPPUNIT_ASSERT_EQUAL(value, string(_packet->getHeader(key.c_str())));
    CPPUNIT_ASSERT_EQUAL(value, string(_packet->getHeader(key2.c_str())));
}

void HTTPPacketTF::testGetHeaderMemCopy() {
  const char *key = "key";
  const char *value = "value";
  _packet->addHeader(key, value);
  char *newValue = (char *)_packet->getHeader(key);
  CPPUNIT_ASSERT_EQUAL(string(value), string(newValue));
  newValue[0] = 'a';
  newValue[1] = 'b';
  CPPUNIT_ASSERT_EQUAL(string("ablue"),  
		       string(_packet->getHeader(key)));
}

void HTTPPacketTF::testSetGetReasonPhrase() {
    string phrase = "OK";
    string phrase2 = "OK ";
    string phrase3 = " OK";
    string empty = "";
    string unknown = "UNKNOWN";
    string spaces = "  ";
    char *null = NULL;

    CPPUNIT_ASSERT_EQUAL(unknown, string(_packet->getReasonPhrase()));
    _packet->setReasonPhrase(null);
    CPPUNIT_ASSERT_EQUAL(unknown, string(_packet->getReasonPhrase()));
    _packet->setReasonPhrase(empty.c_str());
    CPPUNIT_ASSERT_EQUAL(unknown, string(_packet->getReasonPhrase()));
    _packet->setReasonPhrase(spaces.c_str());
    CPPUNIT_ASSERT_EQUAL(unknown, string(_packet->getReasonPhrase()));
    _packet->setReasonPhrase(phrase.c_str());
    CPPUNIT_ASSERT_EQUAL(phrase, string(_packet->getReasonPhrase()));
    _packet->setReasonPhrase(phrase2.c_str());
    CPPUNIT_ASSERT_EQUAL(phrase2, string(_packet->getReasonPhrase()));
    _packet->setReasonPhrase(phrase3.c_str());
    CPPUNIT_ASSERT_EQUAL(phrase, string(_packet->getReasonPhrase()));
}

void HTTPPacketTF::testSetGetStatusCode() {
    CPPUNIT_ASSERT_EQUAL(0, _packet->getStatusCode());
    _packet->setStatusCode(-1);
    CPPUNIT_ASSERT_EQUAL(0, _packet->getStatusCode());
    _packet->setStatusCode(200);
    CPPUNIT_ASSERT_EQUAL(200, _packet->getStatusCode());
    _packet->setStatusCode(600);
    CPPUNIT_ASSERT_EQUAL(0, _packet->getStatusCode());
    _packet->setStatusCode(99);
    CPPUNIT_ASSERT_EQUAL(0, _packet->getStatusCode());
}

void HTTPPacketTF::testInitialPacket() {
    CPPUNIT_ASSERT(!_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(0, _dataBuffer->getDataLen());
}

void HTTPPacketTF::testEncodeNoType() {
   _packet->setURI("/");
    CPPUNIT_ASSERT(!_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(0, _dataBuffer->getDataLen());
}

void HTTPPacketTF::testEncodeRequestNoMethod() {
    _packet->setPacketType(HTTPPacket::PT_REQUEST);
    CPPUNIT_ASSERT(!_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(0, _dataBuffer->getDataLen());   

   _packet->setURI("/");
    CPPUNIT_ASSERT(!_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(0, _dataBuffer->getDataLen());
}

void HTTPPacketTF::testEncodeRequestNoURI() {
    _packet->setPacketType(HTTPPacket::PT_REQUEST);
    _packet->setMethod(HTTPPacket::HM_GET);
    CPPUNIT_ASSERT(!_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(0, _dataBuffer->getDataLen());
}

void HTTPPacketTF::testEncodeGet() {
    HTTPPacket *decodedPacket = NULL;
    size_t bodyLength = 0;
    const char request1[] = "GET / HTTP/1.1\r\nContent-Length: 0\r\n\r\n";
    _packet->setMethod(HTTPPacket::HM_GET);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(strlen(request1), (size_t)_dataBuffer->getDataLen());
    CPPUNIT_ASSERT(memcmp(&request1[0], _dataBuffer->getData(), 
                          _dataBuffer->getDataLen()) == 0);
    _dataBuffer->clear();

    _packet->addHeader("Host", "127.0.0.1");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_GET, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    CPPUNIT_ASSERT(!decodedPacket->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL(string("127.0.0.1"), 
                         string(decodedPacket->getHeader("host")));
    _context->reset();

    _packet->addHeader("key1", "value1");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_GET, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    CPPUNIT_ASSERT(!decodedPacket->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL(string("value1"), 
                         string(decodedPacket->getHeader("key1")));
    CPPUNIT_ASSERT_EQUAL(string("127.0.0.1"), 
                         string(decodedPacket->getHeader("host")));
}

void HTTPPacketTF::testEncodeMethods() {
    HTTPPacket *decodedPacket = NULL;

    _packet->setMethod(HTTPPacket::HM_OPTIONS);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_OPTIONS, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_HEAD);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_GET);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_GET, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_POST);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_POST, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_PUT);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_PUT, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_DELETE);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_DELETE, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_TRACE);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_TRACE, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_CONNECT);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_CONNECT, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod("MY_METHOD");
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_UNSUPPORTED,
                         decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("MY_METHOD"), 
                         string(decodedPacket->getMethodString()));
    CPPUNIT_ASSERT_EQUAL(string("/"), string(decodedPacket->getURI()));
    _context->reset();
}

void HTTPPacketTF::testEncodeURI() { 
    HTTPPacket *decodedPacket = NULL;

    _packet->setMethod(HTTPPacket::HM_OPTIONS);
    _packet->setURI("/abc");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_OPTIONS, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("/abc"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_HEAD);
    _packet->setURI("HTTP://www.xx.com/a");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("HTTP://www.xx.com/a"), string(decodedPacket->getURI()));
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_GET);
    _packet->setURI("");
    CPPUNIT_ASSERT(!_packet->encode(_dataBuffer));

    _packet->setMethod(HTTPPacket::HM_GET);
    _packet->setURI(NULL);
    CPPUNIT_ASSERT(!_packet->encode(_dataBuffer));
}

void HTTPPacketTF::testEncodeVersion() {
    HTTPPacket *decodedPacket = NULL;
    _packet->setMethod(HTTPPacket::HM_HEAD);
    _packet->setURI("HTTP://www.xx.com/a");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("HTTP://www.xx.com/a"), string(decodedPacket->getURI()));
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, decodedPacket->getVersion());
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_OPTIONS);
    _packet->setURI("/abc");
    _packet->setVersion(HTTPPacket::HTTP_1_0);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_OPTIONS, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_0, decodedPacket->getVersion());
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_HEAD);
    _packet->setURI("HTTP://www.xx.com/a");
    _packet->setVersion(HTTPPacket::HTTP_1_1);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("HTTP://www.xx.com/a"), string(decodedPacket->getURI()));
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, decodedPacket->getVersion());
    _context->reset();


    _packet->setMethod(HTTPPacket::HM_HEAD);
    _packet->setURI("HTTP://www.xx.com/a");
    _packet->setVersion(HTTPPacket::HTTP_UNSUPPORTED);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("HTTP://www.xx.com/a"), string(decodedPacket->getURI()));
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, decodedPacket->getVersion());
    _context->reset();

    _packet->setMethod(HTTPPacket::HM_HEAD);
    _packet->setURI("HTTP://www.xx.com/a");
    _packet->setVersion((HTTPPacket::HTTPVersion)999);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HM_HEAD, decodedPacket->getMethod());
    CPPUNIT_ASSERT_EQUAL(string("HTTP://www.xx.com/a"), string(decodedPacket->getURI()));
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, decodedPacket->getVersion());
    _context->reset();
}

void HTTPPacketTF::testEncodeBody() {
     HTTPPacket *decodedPacket = NULL;
     size_t bodyLength = 0;
     const char *body = NULL;

    _packet->setMethod(HTTPPacket::HM_POST);
    _packet->setURI("/");
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT(!decodedPacket->getBody(bodyLength));
    _context->reset();

    const char *body1 = "a";
    _packet->setMethod(HTTPPacket::HM_POST);
    _packet->setURI("/");
    _packet->setBody(body1, 1);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT(body = decodedPacket->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL(body[0], 'a');
    CPPUNIT_ASSERT_EQUAL((size_t)1, bodyLength);
    _context->reset();

    const char *body2 = "\x1f""a";
    _packet->setMethod(HTTPPacket::HM_POST);
    _packet->setURI("/");
    _packet->setBody(body2, 2);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT(body = decodedPacket->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL(body[0], '\x1f');
    CPPUNIT_ASSERT_EQUAL(body[1], 'a');
    CPPUNIT_ASSERT_EQUAL((size_t)2, bodyLength);
    _context->reset();
}

void HTTPPacketTF::testResponseNoStatusCode() {
    _packet->setPacketType(HTTPPacket::PT_RESPONSE);
    _packet->setReasonPhrase("OK");
    CPPUNIT_ASSERT(!_packet->encode(_dataBuffer));
 }

void HTTPPacketTF::testResponseNoReasonPhrase() {
    ANET_LOG(DEBUG, "BEGIN testResponseNoReasonPhrase()");
    HTTPPacket *decodedPacket;
    _packet->setPacketType(HTTPPacket::PT_RESPONSE);
    _packet->setStatusCode(200);   
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    ANET_LOG(DEBUG, "data buffer:|%s|",
             string(_dataBuffer->getData(), _dataBuffer->getDataLen()).c_str());
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_RESPONSE, _packet->getPacketType());
    CPPUNIT_ASSERT_EQUAL(string("OK"), string(_packet->getReasonPhrase()));
    _context->reset();
    ANET_LOG(DEBUG,"END testResponseNoReasonPhrase()");
}

void HTTPPacketTF::testSetResponseVersion() {
    ANET_LOG(DEBUG, "BEGIN testSetResponseVersion()");
     HTTPPacket *decodedPacket = NULL;
    _packet->setPacketType(HTTPPacket::PT_RESPONSE);
    _packet->setStatusCode(200);   
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::PT_RESPONSE, _packet->getPacketType());
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, decodedPacket->getVersion());
    _context->reset();

    _packet->setPacketType(HTTPPacket::PT_RESPONSE);
    _packet->setStatusCode(404);
    _packet->setVersion(HTTPPacket::HTTP_1_0);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_0, decodedPacket->getVersion());
    _context->reset();

    _packet->setPacketType(HTTPPacket::PT_RESPONSE);
    _packet->setStatusCode(404);
    _packet->setVersion(HTTPPacket::HTTP_1_1);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, decodedPacket->getVersion());
    _context->reset();
    ANET_LOG(DEBUG, "BEGIN testSetResponseVersion()");
}

void HTTPPacketTF::testResponseEncode() {
    ANET_LOG(DEBUG, "BEGIN testResponseEncode()");
    HTTPPacket *decodedPacket = NULL;
    size_t length = 0;
    _packet->setBody("body", 4);
    _packet->setPacketType(HTTPPacket::PT_RESPONSE);
    _packet->setStatusCode(200);
    _packet->setVersion(HTTPPacket::HTTP_1_1);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(200, decodedPacket->getStatusCode());
    CPPUNIT_ASSERT_EQUAL(string("OK"), 
                         string(decodedPacket->getReasonPhrase()));
    CPPUNIT_ASSERT_EQUAL(string("body"), 
                         string(decodedPacket->getBody(length),4));
    _context->reset();

    _packet->setPacketType(HTTPPacket::PT_RESPONSE);
    _packet->setStatusCode(404);
    _packet->setVersion(HTTPPacket::HTTP_1_1);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, 
                         decodedPacket->getVersion());
    CPPUNIT_ASSERT_EQUAL(string("NOT FOUND"), 
                         string(decodedPacket->getReasonPhrase()));
    _context->reset();

    _packet->setPacketType(HTTPPacket::PT_RESPONSE);
    _packet->setStatusCode(414);
    _packet->setVersion(HTTPPacket::HTTP_1_1);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT(_streamer->processData(_dataBuffer, _context));
    CPPUNIT_ASSERT(_context->isCompleted());
    decodedPacket = dynamic_cast<HTTPPacket*>(_context->getPacket());
    CPPUNIT_ASSERT(decodedPacket);
    CPPUNIT_ASSERT_EQUAL(HTTPPacket::HTTP_1_1, 
                         decodedPacket->getVersion());
    ANET_LOG(DEBUG,"END testResponseEncode()");
}

void HTTPPacketTF::testKeepAlive() {
    ANET_LOG(DEBUG, "Begin testKeepAlive()");
    HTTPPacket packet;
    CPPUNIT_ASSERT(packet.isKeepAlive());
    packet.setKeepAlive(false);
    CPPUNIT_ASSERT(!packet.isKeepAlive());
    packet.setKeepAlive(true);
    CPPUNIT_ASSERT(packet.isKeepAlive());
    ANET_LOG(DEBUG, "End testKeepAlive()");
}

void HTTPPacketTF::testKeepAliveVersion() {
    ANET_LOG(DEBUG, "Begin testKeepAliveVersion()");
    HTTPPacket packet;
    CPPUNIT_ASSERT(packet.isKeepAlive());
    packet.setVersion(HTTPPacket::HTTP_1_0);
    CPPUNIT_ASSERT(!packet.isKeepAlive());
    packet.addHeader("Connection","Keep-Alive");
    CPPUNIT_ASSERT(packet.isKeepAlive());
    packet.removeHeader("Connection");
    CPPUNIT_ASSERT(!packet.isKeepAlive());
    packet.setVersion(HTTPPacket::HTTP_1_1);
    CPPUNIT_ASSERT(packet.isKeepAlive());
    packet.addHeader("Connection","close");
    CPPUNIT_ASSERT(!packet.isKeepAlive());
    packet.addHeader("Connection","Keep-Alive");
    CPPUNIT_ASSERT(packet.isKeepAlive());
    ANET_LOG(DEBUG, "End testKeepAliveVersion()");
}

void HTTPPacketTF::testSetKeepAlive() {
    HTTPPacket packet;
    const char *value = NULL;
    packet.setVersion(HTTPPacket::HTTP_1_0);
    CPPUNIT_ASSERT(!packet.isKeepAlive());
    packet.setKeepAlive(true);
    value = packet.getHeader("connection");
    CPPUNIT_ASSERT(value);
    CPPUNIT_ASSERT_EQUAL(string("Keep-Alive"), string(value));
    packet.setKeepAlive(false);
    CPPUNIT_ASSERT(!packet.isKeepAlive());
    value = packet.getHeader("connection");
    CPPUNIT_ASSERT(!value);
                         
    packet.setVersion(HTTPPacket::HTTP_1_1);
    CPPUNIT_ASSERT(packet.isKeepAlive());
    value = packet.getHeader("connection");
    if (value) {
        CPPUNIT_ASSERT(string("close") != string(value));
    }
    packet.setKeepAlive(false);
    CPPUNIT_ASSERT(!packet.isKeepAlive());
    value = packet.getHeader("connection");
    CPPUNIT_ASSERT(value);
    CPPUNIT_ASSERT_EQUAL(string("close"), string(value));
    packet.setKeepAlive(true);
    CPPUNIT_ASSERT(packet.isKeepAlive());
    value = packet.getHeader("connection");
    CPPUNIT_ASSERT(!value);
}

void HTTPPacketTF::testHeaderIteration() {
    HTTPPacket packet;
    HTTPPacket::ConstHeaderIterator it = packet.headerBegin();
    
    CPPUNIT_ASSERT(packet.headerEnd() == it);
    
    packet.addHeader("Connection", "close");
    it = packet.headerBegin();
    CPPUNIT_ASSERT(packet.headerEnd() != it);
    CPPUNIT_ASSERT_EQUAL(string("Connection"), string(it->first));
    CPPUNIT_ASSERT_EQUAL(string("close"), string(it->second));
    CPPUNIT_ASSERT(packet.headerEnd() == ++it);
    
    packet.addHeader("key", "value"); 
    it = packet.headerBegin();
    CPPUNIT_ASSERT(packet.headerEnd() != it);
    if (0 == strcmp("key", it->first)) {
        CPPUNIT_ASSERT_EQUAL(string("key"), string(it->first));
        CPPUNIT_ASSERT_EQUAL(string("value"), string(it->second));
        it ++;
        CPPUNIT_ASSERT_EQUAL(string("Connection"), string(it->first));
        CPPUNIT_ASSERT_EQUAL(string("close"), string(it->second));
    } else {
        CPPUNIT_ASSERT_EQUAL(string("Connection"), string(it->first));
        CPPUNIT_ASSERT_EQUAL(string("close"), string(it->second));
        it ++;
        CPPUNIT_ASSERT_EQUAL(string("key"), string(it->first));
        CPPUNIT_ASSERT_EQUAL(string("value"), string(it->second));
    }
    CPPUNIT_ASSERT(packet.headerEnd() == ++it);
}

}/*end namespace anet*/
