#ifndef UTILS_H_
#define UTILS_H_
#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include <cmath>

// resource id - for animations, textures, etc
using ResID = size_t;

using std::literals::string_literals::operator""s;

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

namespace Easing
{
    inline float circ(float val_)
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
        Average(const T &def_ = 0) :
            sum(def_)
        {
        }

        template<typename T2>
        Average &operator+=(const T2 &rhs_) noexcept
        {
            sum += rhs_;
            cnt++;
            return *this;
        }

        template<typename T2>
        operator T2() noexcept
        {
            if (cnt == 0)
                return 0;
            return sum / cnt;
        }

        inline bool isSet() const noexcept
        {
            return cnt > 0;
        }

    private:
        T sum;
        int cnt = 0;

    };

    template<Numeric T>
    auto degreesToRadians(const T &degrees_) noexcept -> decltype(degrees_ * 1.0f)
    {
        return degrees_ * 3.14159f / 180;
    }

    template <Numeric T>
    inline T clamp(const T& val, const T &min, const T &max) noexcept
    {
    	if (val < min)
    		return min;
    
    	if (val > max)
    		return max;
    
    	return val;
    }
    
    template <Numeric T>
    inline T clampMaxPriority(const T& val, const T &min, const T &max) noexcept
    {
        if (val > max)
    		return max;

    	if (val < min)
        {
            if (max <= min)
                return max;
            else
    		    return min;
        }
    
    	return val;
    }

    template <bool ON_NULLS = true, Numeric T1, Numeric T2>
    inline bool sameSign(const T1 &v1, const T2 &v2) noexcept
    {
    	return (v1 > 0 && v2 > 0 || v1 < 0 && v2 < 0 || v1 == v2 && ON_NULLS);
    }

    template <Numeric T>
    inline T signof(const T &val_) noexcept
    {
        if (val_ >= 0)
            return 1;
        else
            return -1;
    }

    template <Numeric T, Numeric aT>
    inline T lerp(const T &min, const T &max, const aT &alpha) noexcept
    {
        return {min + (max - min) * alpha};
    }

    template <Numeric T>
    inline T reverseLerp(const T& val, const T &min, const T &max)
    {
    	T alpha = (val - min) / (max - min);
        return clamp<T>(alpha, 0, 1);
    }

    // Gets portion of l1, overlapped by l2, result is in range [0, 1]
    template<Numeric T>
    inline constexpr auto getOverlapPortion(T l1min_, T l1max_, T l2min_, T l2max_) noexcept -> decltype(1.0f / l1min_)
    {
        assert(l1max_ > l1min_);
        assert(l2max_ > l2min_);

        auto pmin = std::max(l1min_, l2min_);
        auto pmax = std::min(l1max_, l2max_);

        if (pmin >= pmax)
            return 0.0f;

        return static_cast<float>(pmax - pmin) / (l1max_ - l1min_);
    }

    constexpr inline std::string getIntend(size_t intend_)
    {
        return std::string(intend_, ' ');
    }

    constexpr inline std::string wrap(const std::string &src_)
    {
        return "\"" + src_ + "\"";
    }

    // value_ <= bound_ for bound < 0 or value_ >= bound_ otherwise
    template<Numeric T>
    constexpr inline bool isLowerOrGreater(const T& value_, const T& bound_) noexcept
    {
        return bound_ < 0 ? value_ <= bound_ : value_ >= bound_;
    }

#if 0 // Builin regex bad
    inline std::string replaceAll(std::string src_, const std::string &replacable_, const std::string &toReplace_)
    {
        return std::regex_replace(src_, std::regex(replacable_), toReplace_);
    }

    inline std::string normalizeType(const std::string &reg_)
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


    inline void dumpType(std::ostream &os_, const std::string &type_)
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

    inline bool startsWith(const std::string &base_, const std::string &beginning_)
    {
        for (size_t i = 0; i < beginning_.size(); ++i)
        {
            if (base_[i] != beginning_[i])
                return false;
        }

        return true;
    }

    inline std::vector<std::string> tokenize(const std::string &src_, const char splitter_)
    {
        std::stringstream ss(src_);
        std::vector<std::string> res;

        while (ss.good())
        {
            std::string substr;
            std::getline(ss, substr, splitter_);
            res.push_back(substr);
        }

        return res;
    }

    inline std::string padToRight(const size_t &len_, const std::string &line_)
    {
        if (line_.size() >= len_)
            return line_;

        return std::string(len_ - line_.size(), ' ') + line_;
    }

    inline std::string strip(std::string line_)
    {
        //while (!line_.empty() && )
        return line_;
    }

}


template <typename T, size_t len>
inline std::ostream& operator<< (std::ostream& out, const std::array<T, len>& arr)
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
inline std::ostream& operator<< (std::ostream& out, const std::vector<T>& vec)
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

#endif
