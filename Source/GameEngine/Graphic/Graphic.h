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
#include "Effect/AmbientLightEffect.h"
#include "Effect/ColorEffect.h"
#include "Effect/ConstantColorEffect.h"
#include "Effect/DirectionalLightEffect.h"
#include "Effect/DirectionalLightTextureEffect.h"
#include "Effect/Font.h"
#include "Effect/FontArialW400H18.h"
#include "Effect/Lighting.h"
#include "Effect/LightingEffect.h"
#include "Effect/LightCameraGeometry.h"
#include "Effect/MaterialLayer.h"
#include "Effect/Material.h"
#include "Effect/Particle.h"
#include "Effect/PointLightEffect.h"
#include "Effect/PointLightTextureEffect.h"
#include "Effect/SpotLightEffect.h"
#include "Effect/SpotLightTextureEffect.h"
#include "Effect/Texture2ColorEffect.h"
#include "Effect/MultiTexture2Effect.h"
#include "Effect/Texture2Effect.h"
#include "Effect/TextEffect.h"
#include "Effect/ColorEffect.h"
#include "Effect/VisualEffect.h"

// Image
#include "Image/ImageResource.h"

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
#include "Resource/Buffer/SkinMeshBuffer.h"
#include "Resource/Buffer/MeshBuffer.h"
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
#include "Scene/Scene.h"
#include "Scene/MeshFactory.h"
#include "Scene/LightManager.h"

#include "Scene/Controller/Controller.h"
#include "Scene/Controller/ControlledObject.h"
#include "Scene/Controller/BlendTransformController.h"
#include "Scene/Controller/TransformController.h"
#include "Scene/Controller/KeyframeController.h"
#include "Scene/Controller/SkinController.h"

#include "Scene/Element/Animator/NodeAnimatorDelete.h"
#include "Scene/Element/Animator/NodeAnimatorFlyCircle.h"
#include "Scene/Element/Animator/NodeAnimatorFlyStraight.h"
#include "Scene/Element/Animator/NodeAnimatorFollowSpline.h"
#include "Scene/Element/Animator/NodeAnimatorFollowCamera.h"
#include "Scene/Element/Animator/NodeAnimatorRotation.h"
#include "Scene/Element/Animator/NodeAnimatorTexture.h"

#include "Scene/Element/Mesh/Mesh.h"
#include "Scene/Element/Mesh/MeshMD3.h"
#include "Scene/Element/Mesh/SkinnedMesh.h"
#include "Scene/Element/Mesh/StandardMesh.h"
#include "Scene/Element/Mesh/MeshCache.h"
#include "Scene/Element/Mesh/MeshLoader.h"
#include "Scene/Element/Mesh/MeshFileLoader.h"

#include "Scene/Element/Particle/ParticleEmitter.h"
#include "Scene/Element/Particle/ParticleAffector.h"
#include "Scene/Element/Particle/ParticleAttractionAffector.h"
#include "Scene/Element/Particle/ParticleBoxEmitter.h"
#include "Scene/Element/Particle/ParticleCylinderEmitter.h"
#include "Scene/Element/Particle/ParticleFadeOutAffector.h"
#include "Scene/Element/Particle/ParticleGravityAffector.h"
#include "Scene/Element/Particle/ParticleMeshEmitter.h"
#include "Scene/Element/Particle/ParticlePointEmitter.h"
#include "Scene/Element/Particle/ParticleRingEmitter.h"
#include "Scene/Element/Particle/ParticleRotationAffector.h"
#include "Scene/Element/Particle/ParticleScaleAffector.h"
#include "Scene/Element/Particle/ParticleSphereEmitter.h"

#include "Scene/Element/AnimatedMeshNode.h"
#include "Scene/Element/BillboardNode.h"
#include "Scene/Element/BoneNode.h"
#include "Scene/Element/CameraNode.h"
#include "Scene/Element/RectangleNode.h"
#include "Scene/Element/CubeNode.h"
#include "Scene/Element/EmptyNode.h"
#include "Scene/Element/LightNode.h"
#include "Scene/Element/MeshNode.h"
#include "Scene/Element/ParticleAnimatedMeshNodeEmitter.h"
#include "Scene/Element/ParticleSystemNode.h"
#include "Scene/Element/RootNode.h"
#include "Scene/Element/ShadowVolumeNode.h"
#include "Scene/Element/SkyDomeNode.h"
#include "Scene/Element/SphereNode.h"
#include "Scene/Element/VolumeLightNode.h"
#include "Scene/Element/ViewVolumeNode.h"

#include "Scene/Hierarchy/Node.h"
#include "Scene/Hierarchy/Light.h"
#include "Scene/Hierarchy/Visual.h"
#include "Scene/Hierarchy/Camera.h"
#include "Scene/Hierarchy/Spatial.h"
#include "Scene/Hierarchy/PVWUpdater.h"
#include "Scene/Hierarchy/ViewVolume.h"
#include "Scene/Hierarchy/BoundingSphere.h"

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

// User Interfaces
#include "UI/UserInterface.h"

#endif
