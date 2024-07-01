#ifndef VECTOR2_H_
#define VECTOR2_H_
#include <cmath>
#include <iostream>
#include "Utils.h"
#include <type_traits>

enum class ORIENTATION {RIGHT, LEFT};

template <typename T>
struct Vector2
{
	T x, y;
	constexpr inline Vector2(T nx = 0, T ny = 0)
	{
		static_assert(std::is_arithmetic_v<T>, "Type T must be an arithmetic type");

		x = nx;
		y = ny;
	}

	template <typename TR>
	constexpr inline Vector2(const Vector2<TR> &rhs)
	{
		x = rhs.x;
		y = rhs.y;
	}

	template <typename TR>
	constexpr inline Vector2<T> &operator=(const Vector2<TR> &rhs)
	{
		x = rhs.x;
		y = rhs.y;
		return *this;
	}

	template <typename TR>
	constexpr inline bool operator==(const Vector2<TR> &rhs) const
	{
		return (x == rhs.x && y == rhs.y);
	}

	template<typename TR>
	constexpr inline auto operator+(const Vector2<TR>& rhs) const -> Vector2<decltype(x+rhs.x)>
	{
		return { x + rhs.x, y + rhs.y };
	}

	template<typename TR>
	constexpr inline auto operator-(const Vector2<TR>& rhs) const -> Vector2<decltype(x-rhs.x)>
	{
		return { x - rhs.x, y - rhs.y };
	}

	template<typename TR>
	constexpr inline auto operator*(const TR& num) const -> Vector2<decltype(x*num)>
	{
		return { x * num, y * num };
	}

	template<typename TR>
	constexpr inline auto operator/(const TR& num) const -> Vector2<decltype(x/num)>
	{
		return { x / num, y / num };
	}

    template<typename TR>
	constexpr inline Vector2<T> &operator+=(const Vector2<TR>& rhs)
	{
		x += rhs.x;
		y += rhs.y;
        return *this;
	}

    template<typename TR>
	constexpr inline Vector2<T> operator-=(const Vector2<TR>& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
        return *this;
	}

    template<typename TR>
	constexpr inline Vector2<T> operator*=(const TR& num)
	{
		x *= num;
		y *= num;
        return *this;
	}

    template<typename TR>
	constexpr inline Vector2<T> operator/=(const TR& num)
	{
		x /= num;
		y /= num;
        return *this;
	}

	constexpr inline auto getLen() const
	{
		return sqrt(x * x + y * y);
	}

	constexpr inline auto getSqLen() const
	{
		return x * x + y * y;
	}

	constexpr inline Vector2<decltype(x / sqrt(x))> normalised() const
	{
		float l = sqrt(x * x + y * y);
		if (l == 0.0f)
			return *this;

		return {x / l, y / l};
	}

    template<typename TR>
	constexpr inline auto mulComponents(const Vector2<TR>& rhs) const -> Vector2<decltype(x * rhs.x)>
	{
		return { x * rhs.x, y * rhs.y };
	}
};
template <typename T>
std::ostream& operator<< (std::ostream& out, const Vector2<T>& vec)
{
    out << vec.x << " " << vec.y;
    return out;
}

struct HorizontalOverlapResult
{
	bool selfIsOnLeft = false;
	bool selfIsInside = false;
	bool otherIsInside = false;
	float overlapWidth = 0;
	inline bool doesOverlap() const
	{
		return selfIsOnLeft || selfIsInside || otherIsInside  || overlapWidth;
	}
};

//Only rectangle hitbox
struct Collider
{
	float x, y, w, h;
	constexpr inline Collider(float nx = 0, float ny = 0, float nw = 0, float nh = 0)
	{
		x = nx;
		y = ny;
		w = nw;
		h = nh;
	}
	constexpr inline Collider operator+(const Vector2<float>& rhs) const
	{
		return { x + rhs.x, y + rhs.y, w, h };
	}
	constexpr inline int isCollideWith_x(const Collider& hbox) const
	{
		if (x <= hbox.x && x + w > hbox.x)
			return 1;
		if (x + w >= hbox.x + hbox.w && x < hbox.x + hbox.w)
			return 2;
		if (x >= hbox.x && x + w <= hbox.x + hbox.w)
			return 4;
		else
			return 0;
	}
	constexpr inline int isCollideWith_y(const Collider& hbox) const
	{
		if (y <= hbox.y && y + h > hbox.y)
			return 1;
		if (y + h >= hbox.y + hbox.h && y < hbox.y + hbox.h)
			return 2;
		if (y >= hbox.y && y + h <= hbox.y + hbox.h)
			return 4;
		else
			return 0;
	}
	constexpr inline bool isCollideWith(const Collider& hbox) const
	{
		return isCollideWith_x(hbox) && isCollideWith_y(hbox);
	}

	//-1 if beyond left bound
	// 1 if beyond right bound
	// 0 if within bounds
	constexpr inline int isWithinHorizontalBounds(float leftBound_, float rightBound_) const
	{
		if (x < leftBound_)
			return -1;
		
		if (x + w > rightBound_)
			return 1;

		return 0;
	}
	
	constexpr inline float rangeToLeftBound(float leftBound_) const
	{
		return x - leftBound_;
	}

	constexpr inline float rangeToRightBound(float rightBound_) const
	{
		return rightBound_ - (x + w);
	}

	constexpr inline bool doesExceedOrTouchBoundaries(float leftBound_, float rightBound_) const
	{
		if (x <= leftBound_)
			return true;
		
		if (x + w >= rightBound_)
			return true;

		return false;
	}

	constexpr inline HorizontalOverlapResult getHorizontalOverlap(const Collider &rhs) const
	{
		HorizontalOverlapResult res;

		if (x > rhs.x && x + w < rhs.x + rhs.w)
		{
			res.selfIsInside = true;
		}
		else if (rhs.x > x && rhs.x + rhs.w < x + w)
		{
			res.otherIsInside = true;
		}
		else if (x <= rhs.x && x + w > rhs.x && x + w <= rhs.x + rhs.w)
		{
			res.selfIsOnLeft = true;
			res.overlapWidth = x + w - rhs.x;
		}
		else if (x > rhs.x && x + w > rhs.x + rhs.w && x < rhs.x + rhs.w)
		{
			res.overlapWidth = rhs.x + rhs.w - x;
		}

		return res;
	}

	constexpr inline Collider getOverlapArea(const Collider &rhs) const
	{
		Collider res;

		res.x = std::max(x, rhs.x);
		res.y = std::max(y, rhs.y);
		res.w = std::min(x + w, rhs.x + rhs.w) - res.x;
		res.h = std::min(y + h, rhs.y + rhs.h) - res.y;

		return res;
	}

	constexpr inline Collider getHitboxAtOffset(const Vector2<float> &offset_, ORIENTATION side_)
	{
		Collider hbox = *this;
		if (side_ == ORIENTATION::RIGHT)
        {
            hbox.x += offset_.x;
            hbox.y += offset_.y;
        }
        else
        {
            hbox.x = offset_.x - hbox.x - hbox.w;
            hbox.y += offset_.y;
        }
		return hbox;
	}


	constexpr inline Vector2<float> getPos() const
	{
		return {x, y};
	}
	constexpr inline Vector2<float> getSize() const
	{
		return {w, h};
	}
	constexpr inline Vector2<float> getCenter() const
	{
		return {x + w / 2.0f, y + h / 2.0f};
	}
	
};

inline std::ostream& operator<< (std::ostream& out_, const Collider& cld_)
{
    out_ << "{ " << cld_.x << ", " << cld_.y << ", " << cld_.w << ", " << cld_.h << " }";
    return out_;
}

namespace utils
{
	template <typename T>
	inline Vector2<T> clamp(const Vector2<T>& val, const Vector2<T> &min, const Vector2<T> &max)
	{
		return {clamp(val.x, min.x, max.x), clamp(val.y, min.y, max.y)};
	}

	template <typename T>
	inline Vector2<T> limitVectorLength(const Vector2<T>& val, const T &limit)
	{
		auto curlen = val.getLen();
		if (curlen > limit)
		{
			return val / (curlen / limit);
		}

		return val;
	}

	template <typename T, typename aT>
	inline Vector2<T> lerp(const Vector2<T> &min, const Vector2<T> &max, const aT &alpha)
	{
		return {min + (max - min) * alpha};
	}

	inline std::vector<Vector2<int>> getAreaEdgePoints(int w_, int h_, const Vector2<int> &origin_, int len_)
    {
        int minx = origin_.x - len_;
        int maxx = origin_.x + len_;

        int miny = origin_.y - len_;
        int maxy = origin_.y + len_;

        std::vector<Vector2<int>> points;

        for (int x = minx; x <= maxx; ++x)
        {
            for (int y : {miny, maxy})
            {
                if (x >= 0 && x < w_ && y >= 0 && y < h_)
                    points.push_back({x, y});
            }
        }

        for (int y = miny + 1; y < maxy; ++y)
        {
            for (int x : {minx, maxx})
            {
                if (x >= 0 && x < w_ && y >= 0 && y < h_)
                    points.push_back({x, y});
            }
        }

        return points;
    }

	template <typename T>
	inline std::string toString(const Vector2<T> &v_)
	{
		std::stringstream s;
		s << "{" << v_.x << ", " << v_.y << "}";
		return s.str();
	}
}

#endif