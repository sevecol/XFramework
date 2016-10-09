
#pragma once
#include "MathComm.h"

namespace Math
{
	template<typename T> struct XVector3
	{ 
		union 
		{
			struct 
			{
				T	x,y,z; // 3 components of the vector
			};

			T	_v[3];     // Array access useful in loops
		};

		// 构造

		XVector3() : x( (T)0 ), y( (T)0 ), z( (T)0 ) {};
		XVector3( T tx, T ty, T tz ) : x(tx), y(ty), z(tz) {};
		XVector3( T t[3] ) : x( t[0] ), y( t[1] ), z( t[2] ) {};
		XVector3( const XVector3<T>& v ) : x(v.x), y(v.y), z(v.z) {};


		// 设置初值
		inline void SetValue( T tx, T ty, T tz )
		{
			x = tx;
			y = ty;
			z = tz;
		}

		inline void Zero()
		{
			memset( (void*)(&_v[0]), 0, 3*sizeof(T) );
		}

		// 运算符重载

		inline T operator [] ( size_t i ) const
		{
			assert( i < 3 );
			return *(&x+i);
		}

		inline T& operator [] ( size_t i )
		{
			assert( i < 3 );
			return *(&x+i);
		}


		inline XVector3<T>& operator = ( const XVector3<T>& v )
		{
			x = v.x;
			y = v.y;    
			z = v.z;

			return *this;
		}

		inline bool operator == ( const XVector3<T>& v ) const
		{
			return ( IsZero( x - v.x ) && IsZero( y - v.y ) && IsZero( z - v.z ) );
		}

		inline bool operator != ( const XVector3<T>& v ) const
		{
			return ( !IsZero( x - v.x ) || !IsZero( y - v.y ) || !IsZero( z - v.z ) );
		}

		inline XVector3<T> operator + ( const XVector3<T>& v ) const
		{
			return XVector3<T>(x + v.x,y + v.y,z + v.z);
		}

		inline XVector3<T> operator - ( const XVector3<T>& v ) const
		{
			return XVector3<T>(x - v.x,y - v.y,z - v.z);
		}

		inline XVector3<T> operator * ( T tScalar ) const
		{
			return XVector3<T>(tScalar*x,tScalar*y,tScalar*z);
		}

		inline XVector3<T> operator / ( T tScalar ) const
		{
			if( IsZero( tScalar) )
			{
				assert( !IsZero(tScalar) );

			}
				
			XVector3<T> vDiv;

			T tInv = 1.0f / tScalar;
			vDiv.x = x * tInv;
			vDiv.y = y * tInv;
			vDiv.z = z * tInv;

			return vDiv;
		}

		inline XVector3<T> operator - () const
		{
			return XVector3<T>(-x,-y,-z);
		}

		inline XVector3<T>& operator += ( const XVector3<T>& v )
		{
			x += v.x;
			y += v.y;
			z += v.z;

			return *this;
		}

		inline XVector3<T>& operator -= ( const XVector3<T>& v )
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;

			return *this;
		}

		inline XVector3<T>& operator *= ( T tScalar )
		{
			x *= tScalar;
			y *= tScalar;
			z *= tScalar;

			return *this;
		}

		inline XVector3<T>& operator /= ( T tScalar )
		{
			assert( !IsZero(tScalar) );

			T tInv = 1.0f / tScalar;

			x *= tInv;
			y *= tInv;
			z *= tInv;

			return *this;
		}


		// 数学运算

		inline float Length() const
		{
			return std::sqrt(x*x + y*y + z*z);
		}

		inline float LengthSq() const
		{
			return (x*x + y*y + z*z);
		}

		// 两点之间距离
		inline float Distance( const XVector3<T>& pos ) const
		{
			static XVector3<T> v;
			v.x = x-pos.x;
			v.y = y-pos.y;
			v.z = z-pos.z;
			return v.Length();
		}

		// 单位向量
		inline XVector3<T> Normal() const
		{
			float len = Length();

			if( !IsZero(len) )
				return *this/len;
			else
				return *this;
		}

		// 单位化
		inline void Normalize()
		{
			float len = Length();

			if( !IsZero(len) )
			{
				float fReci = 1.0f / len;
				x *= fReci;
				y *= fReci;
				z *= fReci;
			}
		}

		// 点乘
		inline float Dot( const XVector3<T>& v ) const
		{
			return ( x*v.x + y*v.y + z*v.z ); 
		} 

		// 叉乘
		inline XVector3<T> Cross( const XVector3<T>& v ) const
		{
			return XVector3( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x );
		}

		// 两向量夹角(弧度)
		inline float CalAngle( const XVector3<T>& v ) const
		{
			float len = Length();
			if( IsZero(len) )
				return HALFPI;
			else
				return std::acos( Dot(v) / (len*v.Length()) );
		}

		// 友元函数
		inline friend XVector3<T> operator * ( T tScalar, const XVector3<T>& v )
		{
			return v*tScalar;
		}

		// casting
		inline operator T* ()
		{
			return &_v[0];
		}
	};

	typedef XVector3<float> Vector3f;
}
using namespace Math;