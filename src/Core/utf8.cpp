#include "utf8.h"
#include <iostream>

uint8_t utf8::readCharSize(const char *ch_)
{
    if ((*ch_ & oct4) == oct4)
        return 4;
    else if ((*ch_ & oct3) == oct3)
        return 3;
    else if ((*ch_ & oct2) == oct2)
        return 2;
    else
        return 1;
}

uint8_t utf8::readCharSize(uint32_t ch_)
{
    const auto *tmp = reinterpret_cast<const char *>(&ch_);
    //uint8_t arr[] = {tmp[3], tmp[2], tmp[1], tmp[0]};
    if (tmp[3]) // arr[0]
        return 4;
    else if (tmp[2]) // arr[1]
        return 3;
    else if (tmp[1]) // arr[2]
        return 2;
    else
        return 1;
}

uint32_t utf8::readChar(const char *ch_)
{
    return readChar(ch_, readCharSize(ch_));
}

uint32_t utf8::readChar(const char *ch_, uint8_t sz_)
{
    switch (sz_)
    {
    case 1:
        return (ch_[0] & 0xff);
    case 2:
        return ((ch_[0] & 0xff) << 8) | (ch_[1] & 0xff);
    case 3:
        return ((ch_[0] & 0xff) << 16) | ((ch_[1] & 0xff) << 8) | (ch_[2] & 0xff);
    case 4:
        return ((ch_[0] & 0xff) << 24) | ((ch_[1] & 0xff) << 16) | ((ch_[2] & 0xff) << 8) | (ch_[3] & 0xff);
    }

    return 0;
}

const char *utf8::iterateForward(const char *u8char_)
{
    return u8char_ + utf8::readCharSize(u8char_);
}

std::string &utf8::appendChar(std::string &s_, const char *u8char_)
{
    return s_.append(reinterpret_cast<const char*>(u8char_), readCharSize(u8char_));
}

std::string &utf8::appendChar(std::string &s_, const char *u8char_, uint8_t sz_)
{
    return s_.append(reinterpret_cast<const char*>(u8char_), sz_);
}

std::string &utf8::appendChars(std::string &s_, const char *u8char_, size_t cnt_)
{
    size_t fullsz = 0;
    auto *it = u8char_;
    while (cnt_-- && *it)
    {
        auto sz = readCharSize(it);
        fullsz += sz;
        it += sz;
    }
    s_.append(reinterpret_cast<const char*>(u8char_), fullsz);
    return s_;
}

std::string &utf8::appendBytes(std::string &s_, const char *u8char_, size_t sz_)
{
    return s_.append(u8char_, sz_);
}

std::wstring &utf8::appendChar(std::wstring &s_, const char *u8char_)
{
    return s_ += static_cast<wchar_t>(tou32(u8char_));
}

std::wstring &utf8::appendChar(std::wstring &s_, const char *u8char_, uint8_t sz_)
{
    return s_ += static_cast<wchar_t>(tou32(u8char_, sz_));
}

std::wstring &utf8::appendChars(std::wstring &s_, const char *u8char_, size_t cnt_)
{
    auto *it = u8char_;
    while (cnt_-- && *it)
    {
        auto sz = readCharSize(it);
        s_ += static_cast<wchar_t>(tou32(it, sz));
        it += sz;
    }
    return s_;
}

U8Wrapper::iterator::iterator(const char *ch_) :
    m_ch(ch_),
    m_byteSize(utf8::readCharSize(ch_))
{
}

bool U8Wrapper::iterator::operator!=(const iterator &rhs_) const
{
    return m_ch != rhs_.m_ch;
}

U8Wrapper::iterator &U8Wrapper::iterator::operator++()
{
    m_ch += m_byteSize;
    m_byteSize = utf8::readCharSize(m_ch);
    return *this;
}

U8Wrapper::iterator &U8Wrapper::iterator::operator*()
{
    return *this;
}

uint32_t U8Wrapper::iterator::getu8() const
{
    return utf8::readChar(m_ch, m_byteSize);
}

uint32_t U8Wrapper::iterator::getu32() const
{
    return utf8::tou32(m_ch, m_byteSize);
}

std::string U8Wrapper::iterator::getCharAsString() const
{
    char arr[5] = {m_ch[0], m_ch[1], m_ch[2], m_ch[3], 0};
    arr[m_byteSize] = 0;

    return std::string(arr);
}

std::wstring U8Wrapper::iterator::getCharAsWString() const
{
    std::wstring s = L"";
    return utf8::appendChar(s, m_ch, m_byteSize);
}

U8Wrapper::U8Wrapper(const std::string &s_) :
    m_s(s_)
{
}

U8Wrapper::iterator U8Wrapper::begin()
{
    return {m_s.c_str()};
}

U8Wrapper::iterator U8Wrapper::end()
{
    return {m_s.c_str() + m_s.size()};
}

std::wstring U8Wrapper::toWString() const
{
    std::wstring res(L"");

    auto *it = m_s.c_str();
    while (*it)
    {
        auto sz = utf8::readCharSize(it);
        utf8::appendChar(res, it, sz);
        it += sz;
    }

    return res;
}

std::string operator+(const U8Wrapper::iterator &lhs_, const std::string &rhs_)
{
    auto s = lhs_.getCharAsString();
    return s + rhs_;
}

std::wstring operator+(const U8Wrapper::iterator &lhs_, const std::wstring &rhs_)
{
    return lhs_.getCharAsWString() + rhs_;
}

std::ostream &operator<<(std::ostream &os_, const U8Wrapper::iterator &it_)
{
    for (int i = 0; i < it_.m_byteSize; ++i)
        os_ << it_.m_ch[i];

    return os_;
}

std::ostream &operator<<(std::ostream &os_, const U8Wrapper &it_)
{
    os_ << it_.m_s;
    return os_;
}

std::string utf8::getChar(uint32_t ch_)
{
    const auto *tmp = reinterpret_cast<const char *>(&ch_);
    char arr[5] = {tmp[3], tmp[2], tmp[1], tmp[0], 0};
    uint8_t i = 0;
    while (!arr[i])
        ++i;

    return std::string(arr + i);
}

uint32_t utf8::u8tou32_1(uint32_t ch_)
{
    return ch_;
}

uint32_t utf8::u8tou32_2(uint32_t ch_)
{
    return (ch_ & 0xFF & 0b00111111)
    | ((ch_ & 0xFF00 & 0b00011111'00000000) >> 2);
}

uint32_t utf8::u8tou32_3(uint32_t ch_)
{
    return (ch_ & 0xFF & 0b00111111)
    | ((ch_ & 0xFF00 & 0b00111111'00000000) >> 2)
    | ((ch_ & 0xFF0000 & 0b00001111'00000000'00000000) >> 4);
}

uint32_t utf8::u8tou32_4(uint32_t ch_)
{
    return (ch_ & 0xFF & 0b00111111)
    | ((ch_ & 0xFF00 & 0b00111111'00000000) >> 2)
    | ((ch_ & 0xFF0000 & 0b00111111'00000000'00000000) >> 4)
    | ((ch_ & 0xFF000000 & 0b00000111'00000000'00000000'00000000) >> 6);
}

uint32_t utf8::u8tou32(uint32_t ch_, size_t sz_)
{
    switch (sz_)
    {
    case 1:
        return u8tou32_1(ch_);

    case 2:
        return u8tou32_2(ch_);

    case 3:
        return u8tou32_3(ch_);

    case 4:
        return u8tou32_4(ch_);
    }

    return 0;
}

uint32_t utf8::tou32_1(const char *ch_)
{
    return (*ch_);
}

uint32_t utf8::tou32_2(const char *ch_)
{
    return (ch_[1] & 0b00111111)
    | ((ch_[0] & 0b00011111) << 6);
}

uint32_t utf8::tou32_3(const char *ch_)
{
    return (ch_[2] & 0b00111111)
    | ((ch_[1] & 0b00111111) << 6)
    | ((ch_[0] & 0b00001111) << 12);
}

uint32_t utf8::tou32_4(const char *ch_)
{
    return (ch_[3] & 0b00111111)
    | ((ch_[2] & 0b00111111) << 6)
    | ((ch_[1] & 0b00111111) << 12)
    | ((ch_[0] & 0b00000111) << 18);
}

uint32_t utf8::tou32(const char *ch_)
{
    return tou32(ch_, readCharSize(ch_));
}

uint32_t utf8::tou32(const char *ch_, uint8_t sz_)
{
    switch (sz_)
    {
    case 1:
        return tou32_1(ch_);

    case 2:
        return tou32_2(ch_);

    case 3:
        return tou32_3(ch_);

    case 4:
        return tou32_4(ch_);
    }

    return 0;
}
