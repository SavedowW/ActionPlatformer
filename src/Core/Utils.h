#ifndef UTILS_H_
#define UTILS_H_
#include <filesystem>
#include <string>
#include <SDL.h>

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
    class OptionalProperty
    {
    public:
        OptionalProperty() = default;

        OptionalProperty(const T &val_) :
            m_val(val_),
            m_isSet(true)
        {
        }

        OptionalProperty(T &&val_) :
            m_val(std::move(val_)),
            m_isSet(true)
        {
        }

        OptionalProperty &operator=(const T &val_)
        {
            m_val = val_;
            m_isSet = true;
            return *this;
        }

        OptionalProperty &operator=(T &&val_)
        {
            m_val = std::move(val_);
            m_isSet = true;
            return *this;
        }

        bool isSet() const
        {
            return m_isSet;
        }

        T &operator*()
        {
            if (!m_isSet)
                throw std::string("Reading unset property");

            return m_val;
        }

        operator T() const
        {
            if (!m_isSet)
                throw std::string("Reading unset property");

            return m_val;
        }

    private:
        T m_val;
        bool m_isSet = false;
    };

    template <typename T>
    inline T clamp(const T& val, const T &min, const T &max)
    {
    	if (val < min)
    		return min;
    
    	if (val > max)
    		return max;
    
    	return val;
    }

    template <bool ON_NULLS = true, typename T1, typename T2>
    inline bool sameSign(const T1 &v1, const T2 &v2)
    {
    	return (v1 > 0 && v2 > 0 || v1 < 0 && v2 < 0 || v1 == v2 && ON_NULLS);
    }

    template <typename T>
    inline T reverseLerp(const T& val, const T &min, const T &max)
    {
    	T alpha = (val - min) / (max - min);
        return clamp<T>(alpha, 0, 1);
    }

    inline std::string getRelativePath(const std::string &basePath_, const std::string &fullPath_)
	{
		std::filesystem::path path1(basePath_);
    	std::filesystem::path path2(fullPath_);
    	auto p = std::filesystem::relative(path2, path1).string();
    	std::replace(p.begin(), p.end(), '\\', '/');
    	return p;
	}

    inline std::string removeExtention(const std::string &filePath_)
	{
		size_t lastindex = filePath_.find_last_of("."); 
        std::string rawName = filePath_.substr(0, lastindex); 

        return rawName;
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

    /*
        Structure used to encode collision result
        Its technically would be possible to encode every single collision case in 1 byte as it would allow up to 256 cases,
        but that would require complicated logic to decode
        This version spends 6 bits per axis and gives 4 extra bits which are in a very suboptimal way
        General cases can be matched with masks, 0 means no collision and overlap and details can be retrieved with bit shifting
        LE - less or equals, GE - greater or equals, etc
        OOT - overlap or touch
    */
    enum class OverlapResult : uint16_t
    {
        NONE =            0b0000'0000'00'0000'00,

        // Touching 
        TOUCH_MIN1_MAX2 = 0b0000'0000'00'0000'01,
        TOUCH_MAX1_MIN2 = 0b0000'0000'00'0000'10,

        // Overlapping
        MIN2_LE_MIN1 =    0b0000'0000'00'0001'00,
        MIN2_GE_MIN1 =    0b0000'0000'00'0010'00,
        MIN2_EQ_MIN1 =    0b0000'0000'00'0011'00,
        MAX2_LE_MAX1 =    0b0000'0000'00'0100'00,
        MAX2_GE_MAX1 =    0b0000'0000'00'1000'00,
        MAX2_EQ_MAX1 =    0b0000'0000'00'1100'00,

        // Extra
        OOT_BOX =         0b0001'0000'00'0000'00, // Slopes only
        OOT_SLOPE =       0b0010'0000'00'0000'00, // Slopes only
        BOTH_TOUCH =      0b0100'0000'00'0000'00,
        BOTH_OVERLAP =    0b1000'0000'00'0000'00,
        BOTH_OOT =        0b1100'0000'00'0000'00,

        // Read-only masks
        OVERLAP_X =   0b0000'0000'00'1111'00,
        OVERLAP_Y =   0b0000'1111'00'0000'00,
        TOUCH_X =     0b0000'0000'00'0000'11,
        TOUCH_Y =     0b0000'0000'11'0000'00,
        OOT_X =       0b0000'0000'00'1111'11,
        OOT_Y =       0b0000'1111'11'0000'00,
        EXTRAS =      0b1111'0000'00'0000'00
    };

    constexpr inline OverlapResult operator|(const OverlapResult& lhs_, const OverlapResult& rhs_)
    {
        return static_cast<OverlapResult>(static_cast<uint16_t>(lhs_) | static_cast<uint16_t>(rhs_));
    }

    constexpr inline OverlapResult operator&(const OverlapResult& lhs_, const OverlapResult& rhs_)
    {
        return static_cast<OverlapResult>(static_cast<uint16_t>(lhs_) & static_cast<uint16_t>(rhs_));
    }

    constexpr inline OverlapResult &operator|=(OverlapResult& lhs_, const OverlapResult& rhs_)
    {
        return lhs_ = lhs_ | rhs_;
    }

    constexpr inline OverlapResult &operator&=(OverlapResult& lhs_, const OverlapResult& rhs_)
    {
        return lhs_ = lhs_ & rhs_;
    }

    constexpr inline OverlapResult operator<<(const OverlapResult& lhs_, unsigned int offset_)
    {
        return static_cast<OverlapResult>(static_cast<uint16_t>(lhs_) << offset_);
    }

    constexpr inline bool operator!(const OverlapResult& lhs_)
    {
        return static_cast<bool>(!static_cast<uint16_t>(lhs_));
    }

    constexpr inline bool operator&&(const OverlapResult& lhs_, const OverlapResult& rhs_)
    {
        return static_cast<uint16_t>(lhs_) && static_cast<uint16_t>(rhs_);
    }

    constexpr inline bool operator||(const OverlapResult& lhs_, const OverlapResult& rhs_)
    {
        return static_cast<uint16_t>(lhs_) || static_cast<uint16_t>(rhs_);
    }

    constexpr inline bool operator&&(const OverlapResult& lhs_, bool rhs_)
    {
        return static_cast<uint16_t>(lhs_) && rhs_;
    }

    constexpr inline bool operator&&(bool lhs_, const OverlapResult& rhs_)
    {
        return lhs_ && static_cast<uint16_t>(rhs_);
    }

    constexpr inline bool operator||(const OverlapResult& lhs_, bool rhs_)
    {
        return static_cast<uint16_t>(lhs_) || rhs_;
    }

    constexpr inline bool operator||(bool lhs_, const OverlapResult& rhs_)
    {
        return lhs_ || static_cast<uint16_t>(rhs_);
    }

    /*
        Determines an overlap between 2 abstract lines regardless from dimension
        OFFSET determines bit offset
        Produces 6 bits at specified offset
    */
    template<unsigned int OFFSET>
    constexpr inline OverlapResult getOverlap(float l1min_, float l1max_, float l2min_, float l2max_)
    {
        if (l2max_ < l1min_ || l2min_ > l1max_)
            return OverlapResult::NONE;

        OverlapResult res = OverlapResult::NONE;

        if (l2max_ == l1min_)
            return OverlapResult::TOUCH_MIN1_MAX2 << OFFSET;

        if (l2min_ == l1max_)
            return OverlapResult::TOUCH_MAX1_MIN2 << OFFSET;

        auto dif1 = l2min_ - l1min_;
        auto dif2 = l2max_ - l1max_;

        if (dif1 >= 0)
            res |= OverlapResult::MIN2_GE_MIN1;
        if (dif1 <= 0)
            res |= OverlapResult::MIN2_LE_MIN1;

        if (dif2 >= 0)
            res |= OverlapResult::MAX2_GE_MAX1;
        if (dif2 <= 0)
            res |= OverlapResult::MAX2_LE_MAX1;
        
        return (res << OFFSET);

    }

}

template <typename T, size_t len>
std::ostream& operator<< (std::ostream& out, const std::array<T, len>& arr)
{
    std::cout << "[";
    for (int i = 0; i < len; ++i)
    {
        std::cout << arr[i];
        if (i != len - 1)
            std::cout << ", ";
    }
    std::cout << "]";
    return out;
}

#endif