
#pragma once
#include "MathComm.h"

namespace Math
{
	struct Color3f
	{
		union 
		{
			struct 
			{
				float r, g, b; // Red, Green, and Blue color data
			};
			float c[3];
		};

		Color3f(){}

		Color3f( float inR, float inG, float inB ) :
		r( inR ), g( inG ), b( inB )
		{
		}

		void Assign( float inR, float inG, float inB )
		{
			r = inR;
			g = inG;
			b = inB;
		}

		uint32 MakeDWord()
		{
			unsigned long iR = (int)(r * 255.f ) << 16;
			unsigned long iG = (int)(g * 255.f ) << 8;
			unsigned long iB = (int)(b * 255.f );
			return 0xff000000 | iR | iG | iB;
		}

		uint32 MakeDWordSafe()
		{
			Color3f temp = *this;
			temp.Sat();
			return temp.MakeDWord();
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
			if( r < 0 ) 
				r = 0.f;
			if( g < 0 ) 
				g = 0.f;
			if( b < 0 ) 
				b = 0.f;
		}

		Color3f& operator += ( const Color3f& in );
		Color3f& operator -= ( const Color3f& in );
		Color3f& operator *= ( const float& in );
		Color3f& operator /= ( const float& in );
	};

	//==========--------------------------  Color3f Operators


	/**
	* Accumulative addition of two colors
	*/
	inline Color3f& Color3f::operator += ( const Color3f& in )
	{
		r += in.r;   
		g += in.g;   
		b += in.b;
		return *this;
	}


	/**
	* Accumulative subtraction of two colors
	*/
	inline Color3f& Color3f::operator -= ( const Color3f& in )
	{
		r -= in.r;   
		g -= in.g;   
		b -= in.b;
		return *this;
	}


	/**
	* Accumulative multiplication of a color by a scalar
	*/
	inline Color3f& Color3f::operator *= ( const float& in )
	{
		r *= in;   
		g *= in;   
		b *= in;
		return *this;
	}


	/**
	* Accumulative division of a color by a scalar
	*/
	inline Color3f& Color3f::operator /= ( const float& in )
	{
		float inv = 1.f / in;
		r *= inv;   
		g *= inv;   
		b *= inv;
		return *this;
	}


	/**
	* Adds two colors together: ret = a + b
	*/
	inline Color3f operator+(Color3f const &a, Color3f const &b)
	{
		return Color3f
			(
			a.r+b.r,
			a.g+b.g,
			a.b+b.b
			);
	}; 


	/**
	* Subtracts two colors : ret = a - b
	*/
	inline Color3f operator-(Color3f const &a, Color3f const &b)
	{
		return Color3f
			(
			a.r-b.r,
			a.g-b.g,
			a.b-b.b
			);
	}; 


	/**
	* Scales a color by a float : ret = a * b
	*/
	inline Color3f operator*(Color3f const &a, float const &b)
	{
		return Color3f
			(
			a.r*b,
			a.g*b,
			a.b*b
			);
	}; 


	/**
	* Scales a color by a float : ret = a * b
	*/
	inline Color3f operator*(float  const &a, Color3f const &b)
	{
		return Color3f
			(
			a*b.r,
			a*b.g,
			a*b.b
			);
	}; 


	/**
	* Divides a color by a float : ret = a / b
	*/
	inline Color3f operator/(Color3f const &a, float const &b)
	{
		float inv = 1.f / b;
		return Color3f
			(
			a.r*inv,
			a.g*inv,
			a.b*inv
			);
	}; 


	/**
	* color Equality, epsilon used due to numerical imprecision
	*/
	inline bool operator==(Color3f const &a, Color3f const &b)
	{
		if( IsZero(a.r-b.r) && IsZero(a.g-b.g) && IsZero(a.b-b.b) )
			return true;
		else
			return false;
	};


	const Color3f COLOR3f_BLACK	  =   Color3f( 0.f,  0.f,  0.f );
	const Color3f COLOR3f_GRAY	  =   Color3f( 0.5f, 0.5f, 0.5f );
	const Color3f COLOR3f_WHITE	  =   Color3f( 1.f,  1.f,  1.f );
	const Color3f COLOR3f_RED	  =	  Color3f( 1.f,  0.f,  0.f );
	const Color3f COLOR3f_GREEN	  =   Color3f( 0.f,  1.f,  0.f );
	const Color3f COLOR3f_BLUE	  =	  Color3f( 0.f,  0.f,  1.f );
	const Color3f COLOR3f_MAGENTA =	  Color3f( 1.f,  0.f,  1.f );
	const Color3f COLOR3f_CYAN	  =	  Color3f( 0.f,  1.f,  1.f );
	const Color3f COLOR3f_YELLOW  =   Color3f( 1.f,  1.f,  0.f );
}
using namespace Math;