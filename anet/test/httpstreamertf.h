/**
 * @Filename: httpstreamertf.h
 * @Author: zhangli
 * @Createtime: 2008-12-12 20:02:45
 * $Id: httpstreamertf.h 15617 2008-12-24 11:25:07Z zhangli $
 * 
 * @Description:
 * 
 */

#ifndef ANET_HTTPSTREAMERTF_H_
#define ANET_HTTPSTREAMERTF_H_
#include <cppunit/extensions/HelperMacros.h>

namespace anet {

class IPacketFactory;
class HTTPStreamer;
class DataBuffer;
class StreamingContext;

class HTTPStreamerTF : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(HTTPStreamerTF);
    CPPUNIT_TEST(testCreatePacket);
    CPPUNIT_TEST(requestLineTokenMissing);
    CPPUNIT_TEST(requestLineMoreToken);
    CPPUNIT_TEST(requestLineSpecialCharacter);
    CPPUNIT_TEST(requestLineEOF);
    CPPUNIT_TEST(testVariousMethod);
    CPPUNIT_TEST(testRequestLineMultiSteps);
    CPPUNIT_TEST(testErrorHeader);
    CPPUNIT_TEST(testHeaderEOF);
    CPPUNIT_TEST(testCorrectHeader);
    CPPUNIT_TEST(testHeadersMultiSteps);
    CPPUNIT_TEST(testNoHeader);
    CPPUNIT_TEST(testFindCRLF);
    CPPUNIT_TEST(testIsTokenCharacter);
    CPPUNIT_TEST(testURITooLarge);
    CPPUNIT_TEST(testStartLineTooLarge);
    CPPUNIT_TEST(testHeaderTooLarge);
    CPPUNIT_TEST(testBodyTooLarge);
    CPPUNIT_TEST(testChunkTooLarge);
    CPPUNIT_TEST(testTooManyHeaders);
    CPPUNIT_TEST(testErrorBody);
    CPPUNIT_TEST(testErrorChunkBody);
    CPPUNIT_TEST(testChunkBodyErrorLength);
    CPPUNIT_TEST(testErrorLastChunk);
    CPPUNIT_TEST(testErrorTrailer);
    CPPUNIT_TEST(testNoBody);
    CPPUNIT_TEST(testStopPosition);
    CPPUNIT_TEST(testContentLength);
    CPPUNIT_TEST(testChunkedLength);
    CPPUNIT_TEST(testChunkStopPosition);
    CPPUNIT_TEST(testChunkTooManyTrailers);
    CPPUNIT_TEST(testErrorStatusLine);
    CPPUNIT_TEST(testStatusLine);
    CPPUNIT_TEST(testStatusLineMultiSteps);
    CPPUNIT_TEST_SUITE_END();
public:
    HTTPStreamerTF();
    ~HTTPStreamerTF();

    void setUp();
    void tearDown();
    void testCreatePacket();
    void requestLineTokenMissing(); 
    void requestLineMoreToken();
    void requestLineSpecialCharacter();
    void requestLineEOF();
    void testVariousMethod();
    void testRequestLineMultiSteps();
    void testErrorHeader();
    void testHeaderEOF();
    void testCorrectHeader();
    void testHeadersMultiSteps();
    void testNoHeader();
    void testFindCRLF();
    void testIsTokenCharacter();
    void testURITooLarge();
    void testStartLineTooLarge();
    void testHeaderTooLarge();
    void testBodyTooLarge();
    void testChunkTooLarge();
    void testTooManyHeaders();
    void testErrorBody();
    void testErrorChunkBody();
    void testChunkBodyErrorLength();
    void testErrorLastChunk();
    void testErrorTrailer();
    void testNoBody();
    void testStopPosition();
    void testContentLength();
    void testChunkedLength();
    void testChunkStopPosition();
    void testChunkTooManyTrailers();
    void testErrorStatusLine();
    void testStatusLine();
    void testStatusLineMultiSteps();
private:
    IPacketFactory *_factory;
    HTTPStreamer *_streamer;
    DataBuffer *_dataBuffer;
    StreamingContext *_context;
};

}/*end namespace anet*/
#endif/* ANET_HTTPSTREAMERTF_H_*/
