// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef COLOREFFECT_H
#define COLOREFFECT_H

#include "Mathematic/Algebra/Matrix4x4.h"

#include "Graphic/Effect/VisualEffect.h"

class GRAPHIC_ITEM ColorEffect : public VisualEffect
{
public:
    // Construction.
	ColorEffect(eastl::shared_ptr<ProgramFactory> const& factory, 
		eastl::vector<eastl::string> const& path);

	void SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix);

};

#endif