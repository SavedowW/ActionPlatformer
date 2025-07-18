#ifndef UTILS_H_
#define UTILS_H_
#include <string>
#include <SDL.h>
#include <regex>
#include <sstream>
#include <string>

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
        Average &operator+=(const T2 &rhs_)
        {
            sum += rhs_;
            cnt++;
            return *this;
        }

        template<typename T2>
        operator T2()
        {
            return sum / cnt;
        }

        inline bool isSet() const
        {
            return cnt > 0;
        }

    private:
        T sum;
        int cnt = 0;

    };

    template<typename T>
    T degreesToRadians(const T &degrees_)
    {
        return degrees_ * 3.14159 / 180;
    }

    template <typename T>
    inline T clamp(const T& val, const T &min, const T &max)
    {
    	if (val < min)
    		return min;
    
    	if (val > max)
    		return max;
    
    	return val;
    }
    
    template <typename T>
    inline T clampMaxPriority(const T& val, const T &min, const T &max)
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

    template <bool ON_NULLS = true, typename T1, typename T2>
    inline bool sameSign(const T1 &v1, const T2 &v2)
    {
    	return (v1 > 0 && v2 > 0 || v1 < 0 && v2 < 0 || v1 == v2 && ON_NULLS);
    }

    template <typename T>
    inline T signof(const T &val_)
    {
        if (val_ >= 0)
            return 1;
        else
            return -1;
    }

    template <typename T, typename aT>
    inline T lerp(const T &min, const T &max, const aT &alpha)
    {
        return {min + (max - min) * alpha};
    }

    template <typename T>
    inline T reverseLerp(const T& val, const T &min, const T &max)
    {
    	T alpha = (val - min) / (max - min);
        return clamp<T>(alpha, 0, 1);
    }

    template<bool REMOVE_HASHTAG>
    inline SDL_Color hexToCol(const std::string &s_)
    {
        int offset = (REMOVE_HASHTAG ? 1 : 0);

        SDL_Color col = {255, 255, 255, 255};
        unsigned tmp = 0;

        std::istringstream(s_.substr(0 + offset, 2)) >> std::hex >> tmp;
        col.r = tmp;

        std::istringstream(s_.substr(2 + offset, 2)) >> std::hex >> tmp;
        col.g = tmp;

        std::istringstream(s_.substr(4 + offset, 2)) >> std::hex >> tmp;
        col.b = tmp;

        return col;
    }

    // Gets portion of l1, overlapped by l2, result is in range [0, 1]
    inline float getOverlapPortion(float l1min_, float l1max_, float l2min_, float l2max_)
    {
        auto pmin = std::max(l1min_, l2min_);
        auto pmax = std::min(l1max_, l2max_);

        if (pmin >= pmax)
            return 0;

        return (pmax - pmin) / (l1max_ - l1min_);
    }

    constexpr inline std::string getIntend(int intend_)
    {
        return std::string(intend_, ' ');
    }

    inline std::string replaceAll(std::string src_, const std::string &replacable_, const std::string &toReplace_)
    {
        return std::regex_replace(src_, std::regex(replacable_), toReplace_);
    }

    constexpr inline std::string wrap(const std::string &src_)
    {
        return "\"" + src_ + "\"";
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
        int intendLevel = 0;
        for (const auto &ch : type_)
        {
            if (ch == '>')
            {
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

    inline bool startsWith(const std::string &base_, const std::string &beginning_)
    {
        for (int i = 0; i < beginning_.size(); ++i)
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
            std::getline(ss, substr, ',');
            res.push_back(substr);
        }

        return res;
    }

    inline std::string strip(std::string line_)
    {
        //while (!line_.empty() && )
        return line_;
    }

}

// Assert uniqueness
template <typename...>
inline constexpr auto is_unique = std::true_type{};

template <typename T, typename... Rest>
inline constexpr auto is_unique<T, Rest...> = std::bool_constant<
    (!std::is_same_v<T, Rest> && ...) && is_unique<Rest...>
>{};


template <typename T, size_t len>
std::ostream& operator<< (std::ostream& out, const std::array<T, len>& arr)
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
std::ostream& operator<< (std::ostream& out, const std::vector<T>& vec)
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
