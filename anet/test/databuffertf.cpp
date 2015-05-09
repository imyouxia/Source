#include "databuffertf.h"


using namespace std;

namespace anet{
CPPUNIT_TEST_SUITE_REGISTRATION(DataBufferTF);

void DataBufferTF::setUp()
{
}

void DataBufferTF::tearDown()
{
}
    

void DataBufferTF::testShrink()
{
    DataBuffer buffer;
    buffer.shrink();
    CPPUNIT_ASSERT(!buffer._pstart);
    buffer.expand(1);
    void *old_pstart = buffer._pstart;
    if (buffer._pend - buffer._pstart <= MAX_BUFFER_SIZE) {
        buffer.shrink();
        CPPUNIT_ASSERT_EQUAL(old_pstart, (void *)buffer._pstart);
    }
    const char *content = "just for test";
    int len  = 14;
    buffer.writeBytes(content, len);
    buffer.expand(MAX_BUFFER_SIZE);
    old_pstart = buffer._pstart;
    int datalen = buffer._pstart - buffer._pfree;
    buffer.shrink();
    CPPUNIT_ASSERT(old_pstart != buffer._pstart);
    CPPUNIT_ASSERT(buffer.findBytes((const char *)content, len) >= 0);
    CPPUNIT_ASSERT(old_pstart != buffer._pstart);
    CPPUNIT_ASSERT_EQUAL(datalen,(int)(buffer._pstart - buffer._pfree));
    CPPUNIT_ASSERT_EQUAL((int)(buffer._pend - buffer._pstart), MAX_BUFFER_SIZE);
}

void DataBufferTF::testExpand()
{
    DataBuffer buffer;
    buffer.expand(0);
    CPPUNIT_ASSERT(buffer._pstart);
    int64_t content = 0x12345678;
    buffer.writeInt64(content);
    int oldDataLen = buffer.getDataLen();
    buffer.expand(MAX_BUFFER_SIZE);
    CPPUNIT_ASSERT_EQUAL(oldDataLen, buffer.getDataLen());
    CPPUNIT_ASSERT(buffer.getFreeLen() >= MAX_BUFFER_SIZE);
    CPPUNIT_ASSERT_EQUAL(content, buffer.readInt64());
}

void DataBufferTF::testFindBytes()
{
    DataBuffer buffer;
    buffer.writeBytes("start", 5);
    const char *content = "just for test";
    int len = 13;
    buffer.writeBytes(content, len);
    CPPUNIT_ASSERT(buffer.findBytes((const char*)content, len >= 0));
}

void DataBufferTF::testWriteAndRead()
{
    DataBuffer buffer;
    int8_t int8 = 0x8;
    int16_t int16 = 0x7fff;
    int32_t int32 = 0x70000000;
    int64_t int64 = 0x9abccccc;
    buffer.writeInt8(int8);
    buffer.writeInt16(int16);
    buffer.writeInt32(int32);
    buffer.writeInt64(int64);

    CPPUNIT_ASSERT_EQUAL(int8, buffer.readInt8());
    CPPUNIT_ASSERT_EQUAL(int16, buffer.readInt16());
    CPPUNIT_ASSERT_EQUAL(int32, buffer.readInt32());
    CPPUNIT_ASSERT_EQUAL(int64, buffer.readInt64());
        
    int32_t content = 0x12345678;
    buffer.writeInt32(0x111111);
    buffer.fillInt32(buffer._pdata, content);
    CPPUNIT_ASSERT_EQUAL(content, buffer.readInt32());
}
void DataBufferTF::testDrainData()
{
    DataBuffer buffer;
    char str[] = "just for test";
    buffer.writeBytes(str, sizeof(str)-1);
    int dataLen = buffer.getDataLen();
    buffer.drainData(0);
    CPPUNIT_ASSERT_EQUAL(dataLen, buffer.getDataLen());
    buffer.drainData(sizeof(str)*2);
    CPPUNIT_ASSERT_EQUAL(0, buffer.getDataLen());
}
    
}
