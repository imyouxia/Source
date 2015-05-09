#include "httprequestandresponsepackettf.h"
#include <anet/httppacketstreamer.h>

using namespace std;

namespace anet{
CPPUNIT_TEST_SUITE_REGISTRATION(HttpRequestAndResponsePacketTF);

void HttpRequestAndResponsePacketTF::setUp() {
}

void HttpRequestAndResponsePacketTF::tearDown() {
}

HttpResponsePacket* HttpRequestAndResponsePacketTF::doReply(HttpRequestPacket *request) {
    DefaultHttpPacketFactory factory;
    HttpResponsePacket *reply = (HttpResponsePacket *)factory.createPacket(0);
    reply->setStatus(true);
    reply->setKeepAlive(request->isKeepAlive());
    char *query = request->getQuery();
    reply->setBody(query, strlen(query));
    return reply;
}
    
void HttpRequestAndResponsePacketTF::testDecodeAndEncode() {
    HttpRequestPacket *request = new HttpRequestPacket;
    HttpResponsePacket *response;
    DataBuffer output;
    DataBuffer input;
    PacketHeader header;

    // http_load -r 1 -f 2 url
    char s1[] = "GET /  HTTP/1.0\r\n"
                "Host: localhost\r\n"
                "User-Agent: http_load 12mar2006\r\n\r\n";
    header._dataLen = sizeof(s1)-1;
    input.writeBytes(s1, sizeof(s1) - 1);
    CPPUNIT_ASSERT(request->decode(&input, &header));
    CPPUNIT_ASSERT_EQUAL(1, request->_method);
    CPPUNIT_ASSERT_EQUAL(1, request->_version);
    CPPUNIT_ASSERT_EQUAL(false, request->isKeepAlive());
    CPPUNIT_ASSERT(strcmp("localhost", request->findHeader("Host")) == 0);
    CPPUNIT_ASSERT(request->findHeader("User-Agent"));
    CPPUNIT_ASSERT(strcmp("http_load 12mar2006", request->findHeader("User-Agent")) == 0);

    response = doReply(request);
    response->encode(&output);
    char r1[] = "HTTP/1.1 200 OK\r\n"
                "Connection: close\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 1\r\n\r\n/";
    CPPUNIT_ASSERT(strncmp(r1, output.getData(), sizeof(r1) - 1) == 0);
    output.clear();
    delete response;
    request->free();

    request = new HttpRequestPacket;
    //fbench -q url -c 0  -n 15 -k -s 10 localhost 12345
    request->_isKeepAlive = true;
    char s2[] = "GET http://localhost:12345?name=huang&jj=yy HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "User-Agent: fbench/0.9\r\n\r\n";
    header._dataLen = sizeof(s2)-1;
    input.writeBytes(s2, sizeof(s2) - 1);
    CPPUNIT_ASSERT(request->decode(&input, &header));
    CPPUNIT_ASSERT_EQUAL(1, request->_method);
    CPPUNIT_ASSERT_EQUAL(2, request->_version);
    CPPUNIT_ASSERT_EQUAL(true, request->isKeepAlive());
    CPPUNIT_ASSERT(strcmp("localhost", request->findHeader("Host")) == 0);
    CPPUNIT_ASSERT(strcmp("fbench/0.9", request->findHeader("User-Agent")) == 0);
    CPPUNIT_ASSERT(strcmp(request->getQuery(), "http://localhost:12345?name=huang&jj=yy") == 0);

    response = doReply(request);
    response->encode(&output);
    char r2[] = "HTTP/1.1 200 OK\r\n"
                "Connection: Keep-Alive\r\n"
                "Keep-Alive: timeout=10, max=10\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 39\r\n\r\n"
                "http://localhost:12345?name=huang&jj=yy";
    CPPUNIT_ASSERT(strncmp(r2, output.getData(), sizeof(r2) - 1) == 0);
    output.clear();
    delete response;
    request->free();

    request = new HttpRequestPacket;
    //fbench -q url -c 0  -n 15 -s 10 localhost 12345
    char s3[] = "GET http://localhost:12345?name=huang&jj=yy HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "Connection: close\r\n"
                "User-Agent: fbench/0.9\r\n\r\n";
    header._dataLen = sizeof(s3)-1;
    input.writeBytes(s3, sizeof(s3) - 1);
    CPPUNIT_ASSERT(request->decode(&input, &header));
    CPPUNIT_ASSERT_EQUAL(1, request->_method);
    CPPUNIT_ASSERT_EQUAL(2, request->_version);
    CPPUNIT_ASSERT_EQUAL(false, request->isKeepAlive());
    CPPUNIT_ASSERT(strcmp("localhost", request->findHeader("Host")) == 0);
    CPPUNIT_ASSERT(strcmp("fbench/0.9", request->findHeader("User-Agent")) == 0);
    CPPUNIT_ASSERT(strcmp(request->getQuery(), "http://localhost:12345?name=huang&jj=yy") == 0);

    response = doReply(request);
    response->encode(&output);
    char r3[] = "HTTP/1.1 200 OK\r\n"
                "Connection: close\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 39\r\n\r\n"
                "http://localhost:12345?name=huang&jj=yytp://localhost:12345?name=huang&jj=yy";
    CPPUNIT_ASSERT(strncmp(r3, output.getData(), sizeof(r3) - 1) == 0);
    output.clear();
    delete response;
    request->free();

    request = new HttpRequestPacket;
    //firefox
    request->_isKeepAlive = true;
    char s4[] ="GET /?name=huang&jj=yy HTTP/1.0\r\n"
               "Host: localhost:12345\r\n"
               "User-Agent: Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9b5) Gecko/2008042803 Red Hat/3.0b5-0.beta5.6.el5 Firefox/3.0b5\r\n"
               "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
               "Accept-Language: en-us,en;q=0.5\r\n"
               "Accept-Encoding: gzip,deflate\r\n"
               "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
               "Keep-Alive: 300\r\n"
               "Connection: keep-alive\r\n\r\n";
    header._dataLen = sizeof(s4)-1;
    input.writeBytes(s4, sizeof(s4) - 1);
    CPPUNIT_ASSERT(request->decode(&input, &header));
    CPPUNIT_ASSERT_EQUAL(1, request->_method);
    CPPUNIT_ASSERT_EQUAL(1, request->_version);
    CPPUNIT_ASSERT_EQUAL(true, request->isKeepAlive());
    CPPUNIT_ASSERT(strcmp("localhost:12345", request->findHeader("Host")) == 0);
    CPPUNIT_ASSERT(strcmp("Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9b5) Gecko/2008042803 Red Hat/3.0b5-0.beta5.6.el5 Firefox/3.0b5",
                          request->findHeader("User-Agent")) == 0);
    CPPUNIT_ASSERT(strcmp(request->getQuery(), "/?name=huang&jj=yy") == 0);

    response = doReply(request);
    response->encode(&output);
    char r4[] = "HTTP/1.1 200 OK\r\n"
                "Connection: Keep-Alive\r\n"
                "Keep-Alive: timeout=10, max=10\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 18\r\n\r\n"
                "/?name=huang&jj=yy2345?name=huang&jj=yy";
    CPPUNIT_ASSERT(strncmp(r4, output.getData(), sizeof(r4) - 1) == 0);
    output.clear();
    delete response;
    request->free();

    request = new HttpRequestPacket;
    // http_load -r 1 -f 2 url keep-alive:true 
    request->_isKeepAlive = true;
    char s5[] = "GET / HTTP/1.0\r\n"
                "Host: localhost\r\n"
                "Connection: keep-alive\r\n"
                "User-Agent: http_load 12mar2006\r\n\r\n";
    header._dataLen = sizeof(s5)-1;
    input.writeBytes(s5, sizeof(s5) - 1);
    CPPUNIT_ASSERT(request->decode(&input, &header));
    CPPUNIT_ASSERT_EQUAL(1, request->_method);
    CPPUNIT_ASSERT_EQUAL(1, request->_version);
    CPPUNIT_ASSERT_EQUAL(true, request->isKeepAlive());
    CPPUNIT_ASSERT(strcmp("localhost", request->findHeader("Host")) == 0);
    CPPUNIT_ASSERT(request->findHeader("User-Agent"));
    CPPUNIT_ASSERT(strcmp("http_load 12mar2006", request->findHeader("User-Agent")) == 0);

    response = doReply(request);
    response->encode(&output);
    cout<<output.getData();
    char r5[] = "HTTP/1.1 200 OK\r\n"
                "Connection: Keep-Alive\r\n"
                "Keep-Alive: timeout=10, max=10\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 1\r\n\r\n"
                "//?name=huang&jj=yy2345?name=huang&jj=yy";
    CPPUNIT_ASSERT(strncmp(r5, output.getData(), sizeof(r5) - 1) == 0);
    output.clear();
    delete response;
    request->free();

    request = new HttpRequestPacket;
    //wrong query 
    request->_isKeepAlive = true;
    char s6[] = "GET  HTTP/1.0\r\n"
                "Host: localhost\r\n"
                "Connection: keep-alive\r\n";
    header._dataLen = sizeof(s6)-1;
    input.writeBytes(s6, sizeof(s6) - 1);
    CPPUNIT_ASSERT(!request->decode(&input, &header));
    request->free();
}   

void HttpRequestAndResponsePacketTF::testSetBody()
{
	DefaultHttpPacketFactory factory;
	HttpResponsePacket *reply = (HttpResponsePacket *)factory.createPacket(0);
	const char *p = "\0";
	reply->setBody(p, strlen(p));
	
	delete reply;
}
}

