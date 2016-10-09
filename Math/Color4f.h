
#pragma once
#include "MathComm.h"

namespace Math
{
	struct Color4f
	{
		union 
		{
			struct 
			{
				float r, g, b, a; // Red, Green, and Blue color data
			};
			float c[4];
		};

		Color4f(): r(1.f), g(1.f), b(1.f), a(1.f){};

		Color4f( float inR, float inG, float inB, float inA ) :
		r( inR ), g( inG ), b( inB ), a( inA )
		{
		}

		Color4f( UINT32 color )
		{
			b = (float)(color&255) / 255.f;
			color >>= 8;
			g = (float)(color&255) / 255.f;
			color >>= 8;
			r = (float)(color&255) / 255.f;
			color >>= 8;
			a = (float)(color&255) / 255.f;
		}


		void Assign( float inR, float inG, float inB, float inA )
		{
			r = inR;
			g = inG;
			b = inB;
			a = inA;
		}

        void Assign(UINT32 color )
        {
            b = (float)(color&255) / 255.f;
            color >>= 8;
            g = (float)(color&255) / 255.f;
            color >>= 8;
            r = (float)(color&255) / 255.f;
            color >>= 8;
            a = (float)(color&255) / 255.f;
        }

		UINT32 MakeDWORD() const
		{
			UINT32 iA = (int)(a * 255.f ) << 24;
			UINT32 iR = (int)(r * 255.f ) << 16;
			UINT32 iG = (int)(g * 255.f ) << 8;
			UINT32 iB = (int)(b * 255.f );
			return iA | iR | iG | iB;
		}

		UINT32 MakeDWordSafe()
		{
			Color4f temp = *this;
			temp.Sat();
			return temp.MakeDWORD();
		}

		// if any of the values are >1, cap them.
		void Sat()
		{
			if( r > 1 ) 
				r = 1.f;
			if( g > 1 ) 
				g = 1.f;
			if( b > 1 ) 
				b = 1.f;
			if( a > 1 ) 
				a = 1.f;
			if( r < 0 ) 
				r = 0.f;
			if( g < 0 ) 
				g = 0.f;
			if( b < 0 ) 
				b = 0.f;
			if( a < 0 ) 
				a = 0.f;
		}

		Color4f& operator += ( const Color4f& in );
		Color4f& operator -= ( const Color4f& in );
		Color4f& operator *= ( const Color4f& in );
		Color4f& operator /= ( const Color4f& in );
		Color4f& operator *= ( const float& in );
		Color4f& operator /= ( const float& in );
	};


	//--------------------------  CColor4 Operators  --------------------------//


	/**
	* Accumulative addition of two colors
	*/
	inline Color4f& Color4f::operator += ( const Color4f& in )
	{
		r += in.r;   
		g += in.g;   
		b += in.b;
		a += in.a;
		return *this;
	}


	/**
	* Accumulative subtraction of two colors
	*/
	inline Color4f& Color4f::operator -= ( const Color4f& in )
	{
		r -= in.r;   
		g -= in.g;   
		b -= in.b;
		a -= in.a;
		return *this;
	}


	/**
	* Accumulative multiplication of a color by a color
	*/
	inline Color4f& Color4f::operator *= ( const Color4f& in )
	{
		r *= in.r;   
		g *= in.g;   
		b *= in.b;
		a *= in.a;
		return *this;
	}


	/**
	* Accumulative division of a color by a color
	*/
	inline Color4f& Color4f::operator /= ( const Color4f& in )
	{
		r /= in.r;   
		g /= in.g;   
		b /= in.b;
		a /= in.a;
		return *this;
	}


	/**
	* Accumulative multiplication of a color by a scalar
	*/
	inline Color4f& Color4f::operator *= ( const float& in )
	{
		r *= in;   
		g *= in;   
		b *= in;
		a *= in;
		return *this;
	}


	/**
	* Accumulative division of a color by a scalar
	*/
	inline Color4f& Color4f::operator /= ( const float& in )
	{
		float inv = 1.f / in;
		r *= inv;   
		g *= inv;   
		b *= inv;
		a *= inv;
		return *this;
	}


	/**
	* Adds two colors together: ret = a + b
	*/
	inline Color4f operator+(Color4f const &a, Color4f const &b)
	{
		return Color4f
			(
			a.r+b.r,
			a.g+b.g,
			a.b+b.b,
			a.a+b.a
			);
	}; 


	/**
	* Subtracts two colors : ret = a - b
	*/
	inline Color4f operator-(Color4f const &a, Color4f const &b)
	{
		return Color4f
			(
			a.r-b.r,
			a.g-b.g,
			a.b-b.b,
			a.a-b.a
			);
	}; 


	/**
	* Scales a color by a float : ret = a * b
	*/
	inline Color4f operator*(Color4f const &a, float const &b)
	{
		return Color4f
			(
			a.r*b,
			a.g*b,
			a.b*b,
			a.a*b
			);
	}; 


	/**
	* Scales a color by a float : ret = a * b
	*/
	inline Color4f operator*(float  const &a, Color4f const &b)
	{
		return Color4f
			(
			a*b.r,
			a*b.g,
			a*b.b,
			a*b.a
			);
	}; 


	/**
	* Divides a color by a float : ret = a / b
	*/
	inline Color4f operator/(Color4f const &a, float const &b)
	{
		float inv = 1.f / b;
		return Color4f
			(
			a.r*inv,
			a.g*inv,
			a.b*inv,
			a.a*inv
			);
	}; 

	/**
	* Divides a color by a color (piecewise): ret = a / b
	*/
	inline Color4f operator/(Color4f const &a, Color4f const &b)
	{
		return Color4f
			(
			a.r / b.r,
			a.g / b.g,
			a.b / b.b,
			a.a / b.a
			);
	}; 

	/**
	* Multiplies a color by a color (piecewise): ret = a / b
	*/
	inline Color4f operator*(Color4f const &a, Color4f const &b)
	{
		return Color4f
			(
			a.r * b.r,
			a.g * b.g,
			a.b * b.b,
			a.a * b.a
			);
	}; 


	/**
	* color Equality, epsilon used due to numerical imprecision
	*/
	inline bool operator==(Color4f const &a, Color4f const &b)
	{
		if( IsZero(a.r-b.r) && IsZero(a.g-b.g) && IsZero(a.b-b.b) && IsZero(a.a-b.a) )
			return true;
		else
            return false;
	};


	const Color4f COLOR4f_BLACK	  =   Color4f( 0.f,  0.f,  0.f,  1.0f );
	const Color4f COLOR4f_GRAY	  =   Color4f( 0.5f, 0.5f, 0.5f, 1.0f );
	const Color4f COLOR4f_WHITE	  =   Color4f( 1.f,  1.f,  1.f,  1.0f );
	const Color4f COLOR4f_RED	  =	  Color4f( 1.f,  0.f,  0.f,  1.0f );
	const Color4f COLOR4f_GREEN	  =   Color4f( 0.f,  1.f,  0.f,  1.0f );
	const Color4f COLOR4f_BLUE	  =	  Color4f( 0.f,  0.f,  1.f,  1.0f );
	const Color4f COLOR4f_MAGENTA =	  Color4f( 1.f,  0.f,  1.f,  1.0f );
	const Color4f COLOR4f_CYAN	  =	  Color4f( 0.f,  1.f,  1.f,  1.0f );
	const Color4f COLOR4f_YELLOW  =   Color4f( 1.f,  1.f,  0.f,  1.0f );
}
using namespace Math;