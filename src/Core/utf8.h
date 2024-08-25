#ifndef UTF8_H_
#define UTF8_H_

#include <stdint.h>
#include <string>
#include <bitset>

namespace utf8
{
    // Size byte masks
    inline constexpr uint8_t oct4 = 0b11110000;
    inline constexpr uint8_t oct3 = 0b11100000;
    inline constexpr uint8_t oct2 = 0b11000000;

    // Represent all bytes of any number in binary form, like "10101010'10101010'101..."
    template <typename T>
    inline std::string numericToString(T value_)
    {
        std::string res = "";
        const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&value_);
        int8_t sz = sizeof(T);
        while (--sz >= 0)
        {
            std::bitset<8> bs(ptr[sz]);
            res += bs.to_string();
            if (sz > 0)
                res += '\'';
        }

        return res;
    }

    template <typename T>
    inline std::string numericToString(T value_, const std::string &chunkSeparator_)
    {
        std::string res = "";
        const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&value_);
        int8_t sz = sizeof(T);
        while (--sz >= 0)
        {
            std::bitset<8> bs(ptr[sz]);
            res += bs.to_string();
            if (sz > 0)
                res += chunkSeparator_;
        }

        return res;
    }

    /*
     Take pointer to bytes with UTF-8 character sequence, read size of current byte
     Does not check whether or not it points at size, returns 1 if its not size at all
    */
    uint8_t readCharSize(const char *ch_);
    uint8_t readCharSize(uint32_t ch_);

    /*
     Read up to 4 UTF-8 encoded character from string
    */
    uint32_t readChar(const char *ch_);
    uint32_t readChar(const char *ch_, uint8_t sz_);

    /*
     Read size of a pointed character
    */
    const char *iterateForward(const char *u8char_);

    /*
     Append characters from sequence of characters to a UTF-8 string
    */
    std::string &appendChar(std::string &s_, const char *u8char_);
    std::string &appendChar(std::string &s_, const char *u8char_, uint8_t sz_);
    std::string &appendChars(std::string &s_, const char *u8char_, size_t cnt_);
    std::string &appendBytes(std::string &s_, const char *u8char_, size_t sz_);

    /*
        Append characters from sequence of characters to a UTF-32 string
    */
    std::wstring &appendChar(std::wstring &s_, const char *u8char_);
    std::wstring &appendChar(std::wstring &s_, const char *u8char_, uint8_t sz_);
    std::wstring &appendChars(std::wstring &s_, const char *u8char_, size_t cnt_);

    /*
        Get UTF-8 encoded character represented in string
    */
    std::string getChar(uint32_t ch_);

    /*
        Convert UTF-8 encoded character in 4 byte representation to UTF-32 character
    */
    uint32_t u8tou32_1(uint32_t ch_);
    uint32_t u8tou32_2(uint32_t ch_);
    uint32_t u8tou32_3(uint32_t ch_);
    uint32_t u8tou32_4(uint32_t ch_);
    uint32_t u8tou32(uint32_t ch_, size_t sz_);
    
    /*
        Convert UTF-8 encoded character represented by a sequence of characters to a UTF-32 character
        Doesn't require string to be null terminated, but it should start with a size byte
    */
    uint32_t tou32_1(const char *ch_);
    uint32_t tou32_2(const char *ch_);
    uint32_t tou32_3(const char *ch_);
    uint32_t tou32_4(const char *ch_);
    uint32_t tou32(const char *ch_);
    uint32_t tou32(const char *ch_, size_t sz_);
}

/*
    String wrapper
    Allows iteration over UTF-8 characters
*/
class U8Wrapper
{
public:
    struct iterator
    {
        const char *m_ch;
        size_t m_byteSize;

        iterator(const char *ch_);
        bool operator!=(const iterator &rhs_) const;
        iterator &operator++();
        iterator &operator*();

        // Get 4-byte representation of a pointed character in UTF-8
        uint32_t getu8() const;

        // Get 4-byte representation of a pointed character in UTF-32
        uint32_t getu32() const;

        // Same as getchar
        std::string getCharAsString() const;
        std::wstring getCharAsWString() const;
    };

    U8Wrapper(const std::string &s_);

    iterator begin();
    iterator end();

    std::wstring toWString() const;

private:
    const std::string &m_s;

    friend std::ostream &operator<<(std::ostream &os_, const U8Wrapper &it_);
};

// Operator overloading

template<typename T>
inline T operator+(const T &lhs_, const U8Wrapper::iterator &rhs_)
{
    auto s = lhs_;
    return utf8::appendChar(s, rhs_.m_ch, rhs_.m_byteSize);
}

std::string operator+(const U8Wrapper::iterator &lhs_, const std::string &rhs_);
std::wstring operator+(const U8Wrapper::iterator &lhs_, const std::wstring &rhs_);

template<typename T>
inline T &operator+=(T &lhs_, const U8Wrapper::iterator &rhs_)
{
    return utf8::appendChar(lhs_, rhs_.m_ch, rhs_.m_byteSize);
}

std::ostream &operator<<(std::ostream &os_, const U8Wrapper::iterator &it_);
std::ostream &operator<<(std::ostream &os_, const U8Wrapper &it_);

#endif