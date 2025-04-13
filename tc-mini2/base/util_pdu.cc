/*
 * UtilPdu.cpp
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#include "util_pdu.h"
#include <stdlib.h>
#include <string.h>

///////////// CSimpleBuffer ////////////////
CSimpleBuffer::CSimpleBuffer() {
    buf_ = NULL;

    alloc_size_ = 0;
    write_offset_ = 0;
}

CSimpleBuffer::~CSimpleBuffer() {
    alloc_size_ = 0;
    write_offset_ = 0;
    if (buf_) {
        free(buf_);
        buf_ = NULL;
    }
}

void CSimpleBuffer::Extend(uint32_t len) {
    alloc_size_ = write_offset_ + len;
    alloc_size_ += alloc_size_ >> 2; // increase by 1/4 allocate size
    uchar_t *new_buf = (uchar_t *)realloc(buf_, alloc_size_);
    buf_ = new_buf;
}

uint32_t CSimpleBuffer::Write(void *buf, uint32_t len) {
    if (write_offset_ + len > alloc_size_) {
        Extend(len);
    }

    if (buf) {
        memcpy(buf_ + write_offset_, buf, len);
    }

    write_offset_ += len;

    return len;
}

uint32_t CSimpleBuffer::Read(void *buf, uint32_t len) {
    if (0 == len)
        return len;
    if (len > write_offset_)
        len = write_offset_;

    if (buf)
        memcpy(buf, buf_, len);

    write_offset_ -= len;
    memmove(buf_, buf_ + len, write_offset_);
    return len;
}

////// CByteStream //////
CByteStream::CByteStream(uchar_t *buf, uint32_t len) {
    buf_ = buf;
    len_ = len;
    simple_buf_ = NULL;
    pos_ = 0;
}

CByteStream::CByteStream(CSimpleBuffer *pSimpBuf, uint32_t pos) {
    simple_buf_ = pSimpBuf;
    pos_ = pos;
    buf_ = NULL;
    len_ = 0;
}

int16_t CByteStream::ReadInt16(uchar_t *buf) {
    int16_t data = (buf[0] << 8) | buf[1];
    return data;
}

uint16_t CByteStream::ReadUint16(uchar_t *buf) {
    uint16_t data = (buf[0] << 8) | buf[1];
    return data;
}

int32_t CByteStream::ReadInt32(uchar_t *buf) {
    int32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return data;
}

uint32_t CByteStream::ReadUint32(uchar_t *buf) {
    uint32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return data;
}

void CByteStream::WriteInt16(uchar_t *buf, int16_t data) {
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteUint16(uchar_t *buf, uint16_t data) {
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteInt32(uchar_t *buf, int32_t data) {
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteUint32(uchar_t *buf, uint32_t data) {
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::operator<<(int8_t data) { _WriteByte(&data, 1); }

void CByteStream::operator<<(uint8_t data) { _WriteByte(&data, 1); }

void CByteStream::operator<<(int16_t data) {
    unsigned char buf[2];
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 2);
}

void CByteStream::operator<<(uint16_t data) {
    unsigned char buf[2];
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 2);
}

void CByteStream::operator<<(int32_t data) {
    unsigned char buf[4];
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 4);
}

void CByteStream::operator<<(uint32_t data) {
    unsigned char buf[4];
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 4);
}

void CByteStream::operator>>(int8_t &data) { _ReadByte(&data, 1); }

void CByteStream::operator>>(uint8_t &data) { _ReadByte(&data, 1); }

void CByteStream::operator>>(int16_t &data) {
    unsigned char buf[2];

    _ReadByte(buf, 2);

    data = (buf[0] << 8) | buf[1];
}

void CByteStream::operator>>(uint16_t &data) {
    unsigned char buf[2];

    _ReadByte(buf, 2);

    data = (buf[0] << 8) | buf[1];
}

void CByteStream::operator>>(int32_t &data) {
    unsigned char buf[4];

    _ReadByte(buf, 4);

    data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void CByteStream::operator>>(uint32_t &data) {
    unsigned char buf[4];

    _ReadByte(buf, 4);

    data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void CByteStream::WriteString(const char *str) {
    uint32_t size = str ? (uint32_t)strlen(str) : 0;

    *this << size;
    _WriteByte((void *)str, size);
}

void CByteStream::WriteString(const char *str, uint32_t len) {
    *this << len;
    _WriteByte((void *)str, len);
}

char *CByteStream::ReadString(uint32_t &len) {
    *this >> len;
    char *pStr = (char *)GetBuf() + GetPos();
    Skip(len);
    return pStr;
}

void CByteStream::WriteData(uchar_t *data, uint32_t len) {
    *this << len;
    _WriteByte(data, len);
}

uchar_t *CByteStream::ReadData(uint32_t &len) {
    *this >> len;
    uchar_t *pData = (uchar_t *)GetBuf() + GetPos();
    Skip(len);
    return pData;
}

void CByteStream::_ReadByte(void *buf, uint32_t len) {
    if (pos_ + len > len_) {
        throw CPduException(ERROR_CODE_PARSE_FAILED, "parase packet failed!");
    }

    if (simple_buf_)
        simple_buf_->Read((char *)buf, len);
    else
        memcpy(buf, buf_ + pos_, len);

    pos_ += len;
}

void CByteStream::_WriteByte(void *buf, uint32_t len) {
    if (buf_ && (pos_ + len > len_))
        return;

    if (simple_buf_)
        simple_buf_->Write((char *)buf, len);
    else
        memcpy(buf_ + pos_, buf, len);

    pos_ += len;
}

/*
 * Warning!!!
 * This function return a static char pointer, caller must immediately copy the
 * string to other place
 */
char *idtourl(uint32_t id) {
    static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    static char buf[64];
    char *ptr;
    uint32_t value = id * 2 + 56;

    // convert to 36 number system
    ptr = buf + sizeof(buf) - 1;
    *ptr = '\0';

    do {
        *--ptr = digits[value % 36];
        value /= 36;
    } while (ptr > buf && value);

    *--ptr = '1'; // add version number

    return ptr;
}

uint32_t urltoid(const char *url) {
    uint32_t url_len = strlen(url);
    char c;
    uint32_t number = 0;
    for (uint32_t i = 1; i < url_len; i++) {
        c = url[i];

        if (c >= '0' && c <= '9') {
            c -= '0';
        } else if (c >= 'a' && c <= 'z') {
            c -= 'a' - 10;
        } else if (c >= 'A' && c <= 'Z') {
            c -= 'A' - 10;
        } else {
            continue;
        }

        number = number * 36 + c;
    }

    return (number - 56) >> 1;
}
