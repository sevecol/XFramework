
#pragma once
#include "MathComm.h"

namespace Math
{
	// projects and transforms to viewport space
	inline void ProjectViewportTransform( Vector4f& v, float width, float height)
	{	
		const float oow = 1.0f / v.w;
		v.x *= oow;
		v.y *= oow;

		// Map [-1, 1] to [0, width] and [0, height].
		v.x = (v.x + 1.0f) * width  * 0.5f;
		v.y = (v.y + 1.0f) * height * 0.5f;
	}

	// Clip with near Z plane
	static bool ClipLine( Vector4f& a, Vector4f& b)
	{	
		// -w <= z <= w
		bool aClip = (a.z < 0.0f);//对于OpenGL才是(a.z < -a.w);
		bool bClip = (b.z < 0.0f);//对于OpenGL(b.z < -b.w);
		if(!(aClip  ||  bClip))	  return true;
		else if(aClip  &&  bClip) return false;

		// near plane
		if(aClip)
		{	
			const float dx = b.x - a.x;
			const float dy = b.y - a.y;
			const float dz = b.z - a.z;
			const float dw = b.w - a.w;

			const float t = -(a.z + a.w) / (dw + dz);
			a.x += dx * t;
			a.y += dy * t;
			a.z += dz * t;
			a.w += dw * t;
		}	
		else if(bClip)
		{	
			const float dx = a.x - b.x;
			const float dy = a.y - b.y;
			const float dz = a.z - b.z;
			const float dw = a.w - b.w;

			const float t = -(b.z + b.w) / (dw + dz);
			b.x += dx * t;
			b.y += dy * t;
			b.z += dz * t;
			b.w += dw * t;
		}

		return true;
	}

	class HorizonOC
	{
	public:
		HorizonOC() : _Heights(0) {};
		~HorizonOC()
		{
			SAFE_DELGRP( _Heights );
		}

	public: 
		void SetData( uint32 vw, uint32 vh )
		{
			SAFE_DELGRP( _Heights );

			_viewportWidth = vw;  _viewportHeight = vh;
			_Heights = new float[ _viewportWidth ];
			/*for( int i = 0; i < _viewportWidth; i++ )
				_Heights[i] = -1e37f; */

			memset( _Heights, 0, _viewportWidth*sizeof(float) );
		}

		void Init()
		{
			memset( _Heights, 0, _viewportWidth*sizeof(float) );
		}

		// test if a line is above or below the horizon
		// True if above, false if below.
		bool IsLineVisible(float x1, float y1, float x2, float y2)
		{	
			float t;
			if(x1 > x2)
			{
				// ensure x1 <= x2
				t = x1; x1 = x2; x2 = t;
				t = y1; y1 = y2; y2 = t;
			}

			// check and clip line into [0, _viewportWidth]
			const float right = (float)_viewportWidth - 1.0f;
			if(x2 < 0  ||  x1 > right) return false;

			const float dydx = (y2 - y1) / (x2 - x1);
			if(x1 < 0)
			{
				y1 += -x1 * dydx;
				x1 = 0;
			}

			if(x2 > right)
			{
				y2 -= (right - x2) * dydx;
				x2 = right;
			}

			// Check integer x along the line!
			float y = y1;
			const int ix1 = int(x1+0.5f);//)ceil(x1 - 0.5f);
			const int ix2 = int(x2+0.5f);//floor(x2 + 0.5f);
			for(int x = ix1; x <= ix2; ++x)
			{	
				if(_Heights[x] < y)
				{
					return true;
				}
				y += dydx;
			}

			return false;
		}


		// insert a line into the horizon
		void MergeLine(float x1, float y1, float x2, float y2)
		{	
			float t;
			if(x1 > x2)
			{		
				// ensure x1 <= x2
				t = x1;	x1 = x2; x2 = t;
				t = y1;	y1 = y2; y2 = t;
			}

			// check and clip line into [0, _viewportWidth-1]
			const float right = (float)_viewportWidth - 1.0f;
			if(x2 < 0  ||  x1 > right) return;

			const float dydx = (y2 - y1) / (x2 - x1);
			if(x1 < 0)
			{	
				y1 += -x1 * dydx;
				x1 = 0;
			}
			if(x2 > right)
			{	
				y2 -= (right - x2) * dydx;
				x2 = right;
			}

			// Check integer x along the line!
			float y = y1;
			const int ix1 = int(x1+0.5f);//)ceil(x1 - 0.5f);
			const int ix2 = int(x2+0.5f);//floor(x2 + 0.5f);

			for(int x = ix1; x <= ix2; ++x)
			{	
				if(_Heights[x] < y)
					_Heights[x] = y;
				/*else
				{
					int i = 0; 
					i++;
				}*/
				y += dydx;
			}
		}

		bool IsLineAboveHorizon( Vector4f& a, Vector4f& b )
		{	
			if(ClipLine(a, b))
			{	
				ProjectViewportTransform( a, (float)_viewportWidth, (float)_viewportHeight );
				ProjectViewportTransform( b, (float)_viewportWidth, (float)_viewportHeight );
				return IsLineVisible(a.x, a.y, b.x, b.y);
			}
			else return false;
		}

		void  ClipAndMergeLine( Vector4f& a, Vector4f& b )
		{	
			if(ClipLine(a, b))
			{	
				ProjectViewportTransform( a, (float)_viewportWidth, (float)_viewportHeight );
				ProjectViewportTransform( b, (float)_viewportWidth, (float)_viewportHeight );
				MergeLine(a.x, a.y, b.x, b.y);
			}
		}

	private:
		uint32 _viewportWidth;
		uint32 _viewportHeight;
		float* _Heights;
	};
}
using namespace Math;