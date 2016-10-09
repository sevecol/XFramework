
#pragma once
#include "MathComm.h"
#include "Vector3.h"

namespace Math
{
	template< typename T >
	struct XVector4
	{
		union 
		{
			struct 
			{
				T  x ,y, z ,w; // 4 components of the vector
			};
			struct
			{
				T  s, q, r, t; // for texture coordinate 				
			};

			T	_v[4];     // Array access useful in loops
		};

		// 构造

		XVector4() : x( (T)0 ), y( (T)0 ), z( (T)0 ), w( (T)1 ) {};
		XVector4( T tx, T ty, T tz, T tw ) : x(tx), y(ty), z(tz), w(tw) {};
		XVector4( T t[4] ) : x( t[0] ), y( t[1] ), z( t[2] ), w( t[3] ) {};
		XVector4( const XVector4& v ) : x(v.x), y(v.y), z(v.z), w(v.w) {};


		// 设置初值
		inline void SetValue( T tx, T ty, T tz, T tw )
		{
			x = tx;
			y = ty;
			z = tz;
			w = tw;
		}

		inline void Zero()
		{
			memset( (void*)(&_v[0]), 0, 4*sizeof(T) );
		}

		// 运算符重载

		inline T operator [] ( size_t i ) const
		{
			assert( i < 4 );
			return *(&x+i);
		}

		inline T& operator [] ( size_t i )
		{
			assert( i < 4 );
			return *(&x+i);
		}


		inline XVector4& operator = ( const XVector4& v )
		{
			x = v.x;
			y = v.y;    
			z = v.z;
			w = v.w;

			return *this;
		}

		inline XVector4& operator = ( const Vector3f& v )
		{
			x = v.x;
			y = v.y;    
			z = v.z;
			w = 1.0f;

			return *this;
		}

		inline bool operator == ( const XVector4& v ) const
		{
			return ( IsZero( x - v.x ) && IsZero( y - v.y ) && IsZero( z - v.z )  && IsZero( w - v.w ) );
		}

		inline bool operator != ( const XVector4& v ) const
		{
			return ( !IsZero( x - v.x ) || !IsZero( y - v.y ) || !IsZero( z - v.z ) || !IsZero( w - v.w ) );
		}

		inline XVector4 operator + ( const XVector4& v ) const
		{
			XVector4 vSum;

			vSum.x = x + v.x;
			vSum.y = y + v.y;
			vSum.z = z + v.z;
            vSum.w = w + v.w;
		
			return vSum;
		}

		inline XVector4 operator - ( const XVector4& v ) const
		{
			XVector4 vDiff;

			vDiff.x = x - v.x;
			vDiff.y = y - v.y;
			vDiff.z = z - v.z;
            vDiff.w = w - v.w;

			return vDiff;
		}

		inline XVector4 operator * ( T tScalar ) const
		{
			XVector4 vProd;

			vProd.x = tScalar*x;
			vProd.y = tScalar*y;
			vProd.z = tScalar*z;
            vProd.w = tScalar*w;

			return vProd;
		}

		inline XVector4 operator / ( T tScalar ) const
		{
			assert( !IsZero(tScalar) );

			XVector4 vDiv;

			T tInv = 1.0 / tScalar;
			vDiv.x = x * tInv;
			vDiv.y = y * tInv;
			vDiv.z = z * tInv;
            vDiv.w = w * tInv;

			return vDiv;
		}

		inline XVector4 operator - () const
		{
			XVector4 vNeg;

			vNeg.x = -x;
			vNeg.y = -y;
			vNeg.z = -z;
            vNeg.w = -w;

			return vNeg;
		}

		inline XVector4& operator += ( const XVector4& v )
		{
			x += v.x;
			y += v.y;
			z += v.z;
            w += v.w;

			return *this;
		}

		inline XVector4& operator -= ( const XVector4& v )
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
            w -= v.w;

			return *this;
		}

		inline XVector4& operator *= ( T tScalar )
		{
			x *= tScalar;
			y *= tScalar;
			z *= tScalar;
            w *= tScalar;

			return *this;
		}

		inline XVector4& operator /= ( T tScalar )
		{
			assert( !IsZero(tScalar) );

			T tInv = 1.0 / tScalar;

			x *= tInv;
			y *= tInv;
			z *= tInv;
            w *= tInv;

			return *this;
		}


		// 数学运算

		inline float Length() const
		{
			return std::sqrt(x*x + y*y + z*z + w*w);
		}

		// 两点之间距离
		inline float Distance( const XVector4& pos ) const
		{
			XVector4 v( x-pos.x, y-pos.y, z-pos.z, w-pos.w );
			return v.Length();
		}

		// 单位向量
		inline XVector4& Normal() const
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
				w *= fReci;
			}
		}

		// 点乘
		inline float Dot( const XVector4& v ) const
		{
			return ( x*v.x + y*v.y + z*v.z + w*w ); 
		} 

		// 叉乘
		//inline XVector4 Cross( const XVector4& v ) const
		//{
		//	return XVector4( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x );
		//}

		// 两向量夹角(弧度)
		inline float CalAngle( const XVector4& v ) const
		{
			float len = Length();
			if( IsZero(len) )
				return HALFPI;
			else
				return std::acos( Dot(v) / (len*v.Length()) );
		}

		// 友元函数
		inline friend XVector4 operator * ( T tScalar, const XVector4& v )
		{
			return v*tScalar;
		}	

		// casting
		inline operator T* ()
		{
			return &_v[0];
		}
	};

	typedef XVector4<float>  Vector4f;
}
using namespace Math;