#pragma once
#include "Color.hpp"
#include <ostream>
#include <algorithm>
#include <numbers>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>

// resource id - for animations, textures, etc
using ResID = size_t;

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

namespace Easing
{
    constexpr float circ(float val_) noexcept
    {
        return sqrt(1.0f - powf(val_ - 1.0f, 2.0f));
    }
}

namespace utils
{

    template<typename T>
    class Average
    {
    public:
        constexpr Average(const T &def_ = 0) :
            sum(def_)
        {
        }

        template<typename T2>
        constexpr Average &operator+=(const T2 &rhs_) noexcept
        {
            sum += rhs_;
            cnt++;
            return *this;
        }

        template<typename T2>
        constexpr operator T2() noexcept
        {
            if (cnt == 0)
                return 0;
            return sum / cnt;
        }

        constexpr bool isSet() const noexcept
        {
            return cnt > 0;
        }

    private:
        T sum;
        int cnt = 0;

    };

    template<Numeric T>
    constexpr auto degreesToRadians(const T &degrees_) noexcept -> decltype(degrees_ * 1.0f)
    {
        return degrees_ * std::numbers::pi_v<float> / 180.0f;
    }

    template <Numeric T>
    constexpr T clamp(const T& val, const T &min, const T &max) noexcept
    {
        if (val < min)
            return min;
    
        if (val > max)
            return max;
    
        return val;
    }
    
    template <Numeric T>
    constexpr T clampMaxPriority(const T& val, const T &min, const T &max) noexcept
    {
        if (val > max)
            return max;

        if (val < min)
        {
            if (max <= min)
                return max;

            return min;
        }
    
        return val;
    }

    template <bool ON_NULLS = true, Numeric T1, Numeric T2>
    constexpr bool sameSign(const T1 &v1, const T2 &v2) noexcept
    {
        return (v1 > 0 && v2 > 0 || v1 < 0 && v2 < 0 || v1 == v2 && ON_NULLS);
    }

    template <Numeric T>
    constexpr T signof(const T &val_) noexcept
    {
        if (val_ >= 0)
            return 1;

        return -1;
    }

    template <Numeric T, Numeric aT>
    constexpr T lerp(const T &min, const T &max, const aT &alpha) noexcept
    {
        return {min + (max - min) * alpha};
    }

    template <Numeric T>
    constexpr T reverseLerp(const T& val, const T &min, const T &max)
    {
        T alpha = (val - min) / (max - min);
        return clamp<T>(alpha, 0, 1);
    }

    // Gets portion of l1, overlapped by l2, result is in range [0, 1]
    template<Numeric T>
    constexpr auto getOverlapPortion(T l1min_, T l1max_, T l2min_, T l2max_) noexcept -> decltype(1.0f / l1min_)
    {
        assert(l1max_ > l1min_);
        assert(l2max_ > l2min_);

        auto pmin = std::max(l1min_, l2min_);
        auto pmax = std::min(l1max_, l2max_);

        if (pmin >= pmax)
            return 0.0f;

        return static_cast<float>(pmax - pmin) / (l1max_ - l1min_);
    }

    std::string getIntend(size_t intend_);

    std::string wrap(const std::string &src_);

    // value_ <= bound_ for bound < 0 or value_ >= bound_ otherwise
    template<Numeric T>
    constexpr bool isLowerOrGreater(const T& value_, const T& bound_) noexcept
    {
        return bound_ < 0 ? value_ <= bound_ : value_ >= bound_;
    }

#if 0 // Builin regex bad
    std::string replaceAll(std::string src_, const std::string &replacable_, const std::string &toReplace_)
    {
        return std::regex_replace(src_, std::regex(replacable_), toReplace_);
    }

    std::string normalizeType(const std::string &reg_)
    {

        auto res = replaceAll(reg_, "struct ", "");
        res = replaceAll(res, "class ", "");
        res = replaceAll(res, " ", "");

        res = replaceAll(res, "([^ ]),([^ ])", "$1 , $2");
        res = replaceAll(res, " ,([^ ])", " , $1");
        res = replaceAll(res, "([^ ]), ", "$1 , ");

        res = replaceAll(res, "([^ ])<([^ ])", "$1 < $2");
        res = replaceAll(res, " <([^ ])", " < $1");
        res = replaceAll(res, "([^ ])< ", "$1 < ");

        res = replaceAll(res, "([^ ])>([^ ])", "$1 > $2");
        res = replaceAll(res, " >([^ ])", " > $1");
        res = replaceAll(res, "([^ ])> ", "$1 > ");

        return res;
    }


    void dumpType(std::ostream &os_, const std::string &type_)
    {
        auto res = replaceAll(type_, " ", "");
        size_t intendLevel = 0;
        for (const auto &ch : type_)
        {
            if (ch == '>')
            {
                if (intendLevel < 4)
                    throw std::logic_error("Unexpected '>' before '<'");

                intendLevel -= 4;
                os_ << "\n" << getIntend(intendLevel);
                os_ << ch;
            }
            else
            {
                os_ << ch;

                if (ch == '<')
                    intendLevel += 4;
            }
                

            if (ch == '<' || ch == ',')
                os_ << "\n" << getIntend(intendLevel);
        }
    }
#endif

    constexpr bool startsWith(const std::string &base_, const std::string &beginning_)
    {
        for (size_t i = 0; i < beginning_.size(); ++i)
        {
            if (base_[i] != beginning_[i])
                return false;
        }

        return true;
    }

    template<Integral T>
    std::string prettyNum(const T &val_)
    {
        std::string res = std::to_string(val_);
        if (res.size() <= 3)
            return res;
    
        size_t id = res.size() - 3;
        while (true)
        {
            res.insert(id, "'");
            if (id <= 3)
                return res;
            id -= 3;
        }

    };

    /*
        17712345
        01234567  8 - 3 = 5
             ^

        17712'345
        012345678   5 - 3 = 2
          ^

        17'712'345
        0123456789

        
        7712345
        0123456  7 - 3 = 4
            ^

        7712'345
        01234567  4 - 3 = 1
         ^

        7'712'345
        012345678


        712345
        012345  6 - 3 = 3
           ^

        712'345
        0123456
    */


    std::vector<std::string> tokenize(const std::string &src_, char splitter_);

    std::string padToRight(const size_t &len_, const std::string &line_);
}


template <typename T, size_t len>
constexpr std::ostream& operator<< (std::ostream& out, const std::array<T, len>& arr)
{
    out << "[";
    for (int i = 0; i < len; ++i)
    {
        out << arr[i];
        if (i != len - 1)
            out << ", ";
    }
    out << "]";
    return out;
}

template <typename T>
constexpr std::ostream& operator<< (std::ostream& out, const std::vector<T>& vec)
{
    out << "[";
    for (int i = 0; i < vec.size(); ++i)
    {
        out << vec[i];
        if (i != vec.size() - 1)
            out << ", ";
    }
    out << "]";
    return out;
}
