/*
 * Util_pdu.h
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#ifndef UTILPDU_H_
#define UTILPDU_H_

#include "ostype.h"
#include <list>
#include <map>
#include <set>
#include <string>
using namespace std;

#ifdef WIN32
#ifdef BUILD_PDU
#define DLL_MODIFIER __declspec(dllexport)
#else
#define DLL_MODIFIER __declspec(dllimport)
#endif
#else
#define DLL_MODIFIER
#endif

// exception code
#define ERROR_CODE_PARSE_FAILED 1
#define ERROR_CODE_WRONG_SERVICE_ID 2
#define ERROR_CODE_WRONG_COMMAND_ID 3
#define ERROR_CODE_ALLOC_FAILED 4

class CPduException {
  public:
    CPduException(uint32_t service_id, uint32_t command_id, uint32_t error_code,
                  const char *error_msg) {
        service_id_ = service_id;
        command_id_ = command_id;
        error_code_ = error_code;
        error_msg_ = error_msg;
    }

    CPduException(uint32_t error_code, const char *error_msg) {
        service_id_ = 0;
        command_id_ = 0;
        error_code_ = error_code;
        error_msg_ = error_msg;
    }

    virtual ~CPduException() {}

    uint32_t GetServiceId() { return service_id_; }
    uint32_t GetCommandId() { return command_id_; }
    uint32_t GetErrorCode() { return error_code_; }
    char *GetErrorMsg() { return (char *)error_msg_.c_str(); }

  private:
    uint32_t service_id_;
    uint32_t command_id_;
    uint32_t error_code_;
    string error_msg_;
};

class DLL_MODIFIER CSimpleBuffer {
  public:
    CSimpleBuffer();
    ~CSimpleBuffer();
    uchar_t *GetBuffer() { return buf_; }
    uint32_t GetAllocSize() { return alloc_size_; }
    uint32_t GetWriteOffset() { return write_offset_; }
    void IncWriteOffset(uint32_t len) { write_offset_ += len; }

    void Extend(uint32_t len);
    uint32_t Write(void *buf, uint32_t len);
    uint32_t Read(void *buf, uint32_t len);

  private:
    uchar_t *buf_;
    uint32_t alloc_size_;
    uint32_t write_offset_;
};

class CByteStream {
  public:
    CByteStream(uchar_t *buf, uint32_t len);
    CByteStream(CSimpleBuffer *pSimpBuf, uint32_t pos);
    ~CByteStream() {}

    unsigned char *GetBuf() {
        return simple_buf_ ? simple_buf_->GetBuffer() : buf_;
    }
    uint32_t GetPos() { return pos_; }
    uint32_t GetLen() { return len_; }
    void Skip(uint32_t len) {
        pos_ += len;
        if (pos_ > len_) {
            throw CPduException(ERROR_CODE_PARSE_FAILED,
                                "parase packet failed!");
        }
    }

    static int16_t ReadInt16(uchar_t *buf);
    static uint16_t ReadUint16(uchar_t *buf);
    static int32_t ReadInt32(uchar_t *buf);
    static uint32_t ReadUint32(uchar_t *buf);
    static void WriteInt16(uchar_t *buf, int16_t data);
    static void WriteUint16(uchar_t *buf, uint16_t data);
    static void WriteInt32(uchar_t *buf, int32_t data);
    static void WriteUint32(uchar_t *buf, uint32_t data);

    void operator<<(int8_t data);
    void operator<<(uint8_t data);
    void operator<<(int16_t data);
    void operator<<(uint16_t data);
    void operator<<(int32_t data);
    void operator<<(uint32_t data);

    void operator>>(int8_t &data);
    void operator>>(uint8_t &data);
    void operator>>(int16_t &data);
    void operator>>(uint16_t &data);
    void operator>>(int32_t &data);
    void operator>>(uint32_t &data);

    void WriteString(const char *str);
    void WriteString(const char *str, uint32_t len);
    char *ReadString(uint32_t &len);

    void WriteData(uchar_t *data, uint32_t len);
    uchar_t *ReadData(uint32_t &len);

  private:
    void _WriteByte(void *buf, uint32_t len);
    void _ReadByte(void *buf, uint32_t len);

  private:
    CSimpleBuffer *simple_buf_;
    uchar_t *buf_;
    uint32_t len_;
    uint32_t pos_;
};

char *idtourl(uint32_t id);
uint32_t urltoid(const char *url);

#endif /* UTILPDU_H_ */
