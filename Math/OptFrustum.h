
#pragma  once
#include "Frustum.h"
#include "OptPlane.h"

namespace Math
{
	class OptFrustum : public Frustum
	{
		_declspec (align(64)) OptPlane __planes[6];
	public:
		OptFrustum() 
		{
			_planes = new Plane* [Plane_Count];
			for( int i = 0; i < 6; i++ )
				_planes[i] = new(&__planes[i]) OptPlane;

		};
		~OptFrustum()
		{
			SAFE_DELGRP( _planes );
		}

		void InitData( const Plane* pPlane, bool bNeedNormalize = false )
		{
			SetData( pPlane, bNeedNormalize );

			OptPlane* sp = NULL;
			for( int i = 0; i < Plane_Count; i++ )
			{
				sp = static_cast<OptPlane*>(_planes[i]);
				sp->CalNPVertex();
			}
		}

		void InitData( const Matrix4& comboMatrix, bool bNeedNormalize = false )
		{
			SetData( comboMatrix, bNeedNormalize );

			OptPlane* sp = NULL;
			for( int i = 0; i < Plane_Count; i++ )
			{
				sp = static_cast<OptPlane*>(_planes[i]);
				sp->CalNPVertex();
			}
		}
	};
}
using namespace Math;
