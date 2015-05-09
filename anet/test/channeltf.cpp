#include "channeltf.h"

using namespace std;

namespace anet {
    
CPPUNIT_TEST_SUITE_REGISTRATION(CHANNELTF);

void CHANNELTF::setUp() {
}

void CHANNELTF::tearDown() {
}

void CHANNELTF::testGetId() {
	srand(time(NULL));
	uint32_t cid = rand();
	_channel.setId(cid);
	CPPUNIT_ASSERT_EQUAL(cid, _channel.getId());
}

void CHANNELTF::testGetArgs() {
}

void CHANNELTF::testSetHandler() {
}

void CHANNELTF::testGetHandler() {
}

void CHANNELTF::testSetExpireTime() {
}

void CHANNELTF::testGetNext() {
}



}
