// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2012/07/01)

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "GraphicStd.h"

//InputLayout
#include "InputLayout/InputLayoutManager.h"

// Effects
#include "Effect/Font.h"
#include "Effect/FontArialW400H18.h"
#include "Effect/TextEffect.h"
#include "Effect/BasicEffect.h"
#include "Effect/VisualEffect.h"

// Resources
#include "Resource/DataFormat.h"
#include "Resource/GraphicObject.h"
#include "Resource/MemberLayout.h"
#include "Resource/Resource.h"

// Resources/Buffers
#include "Resource/Buffer/Buffer.h"
#include "Resource/Buffer/ConstantBuffer.h"
#include "Resource/Buffer/IndexBuffer.h"
#include "Resource/Buffer/IndexFormat.h"
#include "Resource/Buffer/RawBuffer.h"
#include "Resource/Buffer/TextureBuffer.h"
#include "Resource/Buffer/StructuredBuffer.h"
#include "Resource/Buffer/VertexBuffer.h"
#include "Resource/Buffer/VertexFormat.h"

// Resources/Textures
#include "Resource/Texture/Texture.h"
#include "Resource/Texture/Texture1.h"
#include "Resource/Texture/Texture1Array.h"
#include "Resource/Texture/Texture2.h"
#include "Resource/Texture/Texture2Array.h"
#include "Resource/Texture/Texture3.h"
#include "Resource/Texture/TextureArray.h"
#include "Resource/Texture/TextureCube.h"
#include "Resource/Texture/TextureCubeArray.h"
#include "Resource/Texture/TextureSingle.h"
#include "Resource/Texture/DrawTarget.h"
#include "Resource/Texture/TextureDS.h"
#include "Resource/Texture/TextureRT.h"

// Scene
#include "Scene/MeshFactory.h"

#include "Scene/Controller/Controller.h"
#include "Scene/Controller/ControlledObject.h"

#include "Scene/Element/Node.h"
#include "Scene/Element/Visual.h"
#include "Scene/Element/Camera.h"
#include "Scene/Element/Spatial.h"
#include "Scene/Element/ViewVolume.h"
#include "Scene/Element/BoundingSphere.h"

#include "Scene/Visibility/Culler.h"
#include "Scene/Visibility/CullingPlane.h"

// Shader
#include "Shader/ComputeProgram.h"
#include "Shader/ComputeShader.h"
#include "Shader/GeometryShader.h"
#include "Shader/PixelShader.h"
#include "Shader/ProgramDefines.h"
#include "Shader/ProgramFactory.h"
#include "Shader/Shader.h"
#include "Shader/VertexShader.h"
#include "Shader/VisualProgram.h"

// State
#include "State/BlendState.h"
#include "State/DepthStencilState.h"
#include "State/DrawingState.h"
#include "State/RasterizerState.h"
#include "State/SamplerState.h"

// Renderers
#include "Renderer/Renderer.h"

#endif
