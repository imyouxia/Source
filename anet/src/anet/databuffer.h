#ifndef ANET_DATA_BUFFER_H_
#define ANET_DATA_BUFFER_H_
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 65536

namespace anet {

  class DataBuffer {

    friend class DataBufferTF;
    
  public:
    /*
     * 构造函数
     */
    DataBuffer() {
      _pend = _pfree = _pdata = _pstart = NULL;
    }

    /*
     * 析构函数
     */
    ~DataBuffer() {
      if (_pstart) {
	free(_pstart);
	_pstart = NULL;
      }
    }

    char *getData() {
      return (char*)_pdata;
    }

    int getDataLen() {
      return (_pfree - _pdata);
    }

    char *getFree() {
      return (char*)_pfree;
    }

    int getFreeLen() {
      return (_pend - _pfree);
    }

    void drainData(int len) {
      _pdata += len;
      if (_pdata > _pfree) {//~Bug #95
	_pdata = _pfree;
      }

      if (_pdata == _pfree) {
	clear();
      }
    }

    void pourData(int len) {
      assert(_pend - _pfree >= len);
      _pfree += len;
    }

    void stripData(int len) {
      assert(_pfree - _pdata >= len);
      _pfree -= len;
    }

    void clear() {
      _pdata = _pfree = _pstart;
    }

    void shrink() {
      if (_pstart == NULL) {
	return;
      }
      if ((_pend - _pstart) <= MAX_BUFFER_SIZE || (_pfree - _pdata) > MAX_BUFFER_SIZE) {
	return;
      }

      int dlen = _pfree - _pdata;

      unsigned char *newbuf = (unsigned char*)malloc(MAX_BUFFER_SIZE);
      assert(newbuf != NULL);

      memcpy(newbuf, _pdata, dlen);
      free(_pstart);

      _pdata = _pstart = newbuf;
      _pfree = _pstart + dlen;
      _pend = _pstart + MAX_BUFFER_SIZE;

      return;
    }


    /*
     * 写函数
     */
    void writeInt8(int8_t n) {
      expand(1);
      *_pfree++ = (unsigned char)n;
    }

    void writeInt16(int16_t n) {
      expand(2);
      _pfree[1] = (unsigned char)n;
      n >>= 8;
      _pfree[0] = (unsigned char)n;
      _pfree += 2;
    }

    /*
     * 写出整型
     */
    void writeInt32(int32_t n) {
      expand(4);
      _pfree[3] = (unsigned char)n;
      n >>= 8;
      _pfree[2] = (unsigned char)n;
      n >>= 8;
      _pfree[1] = (unsigned char)n;
      n >>= 8;
      _pfree[0] = (unsigned char)n;
      _pfree += 4;
    }

    void writeInt64(int64_t n) {
      expand(8);
      _pfree[7] = (unsigned char)n;
      n >>= 8;
      _pfree[6] = (unsigned char)n;
      n >>= 8;
      _pfree[5] = (unsigned char)n;
      n >>= 8;
      _pfree[4] = (unsigned char)n;
      n >>= 8;
      _pfree[3] = (unsigned char)n;
      n >>= 8;
      _pfree[2] = (unsigned char)n;
      n >>= 8;
      _pfree[1] = (unsigned char)n;
      n >>= 8;
      _pfree[0] = (unsigned char)n;
      _pfree += 8;
    }

    void writeBytes(const void *src, int len) {
      expand(len);
      memcpy(_pfree, src, len);
      _pfree += len;
    }

    /*
     * 在某一位置写一整型
     */
    void fillInt32(unsigned char *dst, uint32_t n) {
      dst[3] = (unsigned char)n;
      n >>= 8;
      dst[2] = (unsigned char)n;
      n >>= 8;
      dst[1] = (unsigned char)n;
      n >>= 8;
      dst[0] = (unsigned char)n;
    }

    /*
     * 读函数
     */
    int8_t readInt8() {
      return (*_pdata++);
    }

    int16_t readInt16() {

      int16_t n = _pdata[0];
      n <<= 8;
      n |= _pdata[1];
      _pdata += 2;
      return n;
    }

    int32_t readInt32() {
      int32_t n = _pdata[0];
      n <<= 8;
      n |= _pdata[1];
      n <<= 8;
      n |= _pdata[2];
      n <<= 8;
      n |= _pdata[3];
      _pdata += 4;
      return n;
    }

    int64_t readInt64() {
      int64_t n = _pdata[0];
      n <<= 8;
      n |= _pdata[1];
      n <<= 8;
      n |= _pdata[2];
      n <<= 8;
      n |= _pdata[3];
      n <<= 8;
      n |= _pdata[4];
      n <<= 8;
      n |= _pdata[5];
      n <<= 8;
      n |= _pdata[6];
      n <<= 8;
      n |= _pdata[7];
      _pdata += 8;
      return n;
    }

    void readBytes(void *dst, int len) {
      memcpy(dst, _pdata, len);
      _pdata += len;
    }

    /*
     * 确保有len的空余空间
     */
    void ensureFree(int len) {
      expand(len);
    }

    /*
     * 寻找字符串
     */
    int findBytes(const char *findstr, int len) {
      int dLen = _pfree - _pdata - len + 1;
      for (int i=0; i<dLen; i++) {
	if (_pdata[i] == findstr[0] && memcmp(_pdata+i, findstr, len) == 0) {
	  return i;
	}
      }
      return -1;
    }

  private:
    /*
     * expand
     */
    inline void expand(int need) {
      if (_pstart == NULL) {
	int len = 256; 
	while(len < need) len <<= 1;
	_pfree = _pdata = _pstart = (unsigned char*)malloc(len);
	assert(_pstart);
	_pend = _pstart + len;
      } else if (_pend - _pfree < need) { // 空间不够
	int flen = (_pend - _pfree) + (_pdata - _pstart);
	int dlen = _pfree - _pdata;

	if (flen < need || flen * 4 < dlen) {
	  int bufsize = (_pend - _pstart) * 2;
	  while (bufsize - dlen < need)
	    bufsize <<= 1;

	  unsigned char *newbuf = (unsigned char *)malloc(bufsize);
	  assert(newbuf != NULL);
	  if (dlen > 0) {
	    memcpy(newbuf, _pdata, dlen);
	  }
	  free(_pstart);

	  _pdata = _pstart = newbuf;
	  _pfree = _pstart + dlen;
	  _pend = _pstart + bufsize;
	} else {
	  memmove(_pstart, _pdata, dlen);
	  _pfree = _pstart + dlen;
	  _pdata = _pstart;
	}
      }
    }


  private:
    unsigned char *_pstart;      // buffer开始
    unsigned char *_pend;        // buffer结束
    unsigned char *_pfree;        // free部分
    unsigned char *_pdata;        // data部分
  };

}

#endif /*PACKET_H_*/
