// @author Tomohito Inoue <hypernumbernet@users.noreply.github.com>
#pragma once

template <typename T> 
class Vector3
{
public:
	// メンバ変数
    union
    {
	    struct
        {
		    T x;
		    T y;
		    T z;
	    };
	    T array[3];
    };
    inline Vector3(){}
	inline Vector3(T x_in, T y_in, T z_in) 
        : x(x_in), y(y_in), z(z_in){}
	inline explicit Vector3(const T* const a) 
        : x(a[0]), y(a[1]), z(a[2]){}

	inline void set(T x_in, T y_in, T z_in)
    {
        x = x_in;
        y = y_in;
        z = z_in;
	}

	// 減算
	inline Vector3 operator -(const Vector3& a) const
    {
		return Vector3(x - a.x, y - a.y, z - a.z);
	}

	// 乗算
	template <typename X>
    inline Vector3 operator *(X a) const
    {
		return Vector3(x * a, y * a, z * a);
	}

	// 代入乗算
	template <typename X>
	inline Vector3& operator *=(X a)
    {
		x *= a;
		y *= a;
		z *= a;
		return *this;
	}
};
