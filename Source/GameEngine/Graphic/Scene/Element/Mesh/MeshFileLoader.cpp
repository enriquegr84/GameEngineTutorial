// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "GameEngineStd.h"

#include "MeshFileLoader.h"

#include "Graphic/Image/ImageResource.h"
#include "Graphic/Scene/Element/Mesh/MeshMD3.h"
#include "Graphic/Scene/Element/Mesh/SkinnedMesh.h"
#include "Graphic/Scene/Element/Mesh/StandardMesh.h"

#include "Core/Logger/Logger.h"
#include "Core/IO/FileSystem.h"
#include "Core/Utility/StringUtil.h"

#include "Graphic/3rdParty/stb/stb_image.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


//! Constructor
MeshFileLoader::MeshFileLoader( )
{
}


//! destructor
MeshFileLoader::~MeshFileLoader()
{
}


// CreateMeshResourceLoader
eastl::shared_ptr<BaseResourceLoader> CreateMeshResourceLoader()
{
	return eastl::shared_ptr<MeshFileLoader>(new MeshFileLoader());
}

//
// MeshFileLoader::LoadResource				- Chapter 14, page 492
//
bool MeshFileLoader::LoadResource(void *rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle)
{
	eastl::shared_ptr<MeshResourceExtraData> pExtraData(new MeshResourceExtraData());
	pExtraData->SetMesh( 0 );

	// try to load file based on file extension
	BaseReadFile* file = (BaseReadFile*)rawBuffer;
	if (IsALoadableFileExtension(file->GetFileName()))
	{
		pExtraData->SetMesh( CreateMesh(file) );
		if (pExtraData->GetMesh())
		{
			handle->SetExtra(eastl::shared_ptr<MeshResourceExtraData>(pExtraData));
			return true;
		}
	}

	return false;
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool MeshFileLoader::IsALoadableFileExtension(const eastl::wstring& fileName) const
{
	if (fileName.rfind('.') != eastl::string::npos)
	{
		eastl::wstring fileExtension = fileName.substr(fileName.rfind('.') + 1);
		return fileExtension.compare(L"3d") == 0 || fileExtension.compare(L"3ds") == 0 ||
			fileExtension.compare(L"3mf") == 0 || fileExtension.compare(L"ac") == 0 ||
			fileExtension.compare(L"ac3d") == 0 || fileExtension.compare(L"acc") == 0 ||
			fileExtension.compare(L"amj") == 0 || fileExtension.compare(L"ase") == 0 ||
			fileExtension.compare(L"b3d") == 0 || fileExtension.compare(L"blend") == 0 ||
			fileExtension.compare(L"bvh") == 0 || fileExtension.compare(L"cms") == 0 || 
			fileExtension.compare(L"cob") == 0 || fileExtension.compare(L"dae") == 0 || 
			fileExtension.compare(L"dxf") == 0 || fileExtension.compare(L"enff") == 0 || 
			fileExtension.compare(L"fbx") == 0 || fileExtension.compare(L"gltf") == 0 || 
			fileExtension.compare(L"hmb") == 0 || fileExtension.compare(L"ifc") == 0 || 
			fileExtension.compare(L"lwo") == 0 || fileExtension.compare(L"lws") == 0 || 
			fileExtension.compare(L"lxo") == 0 || fileExtension.compare(L"md2") == 0 || 
			fileExtension.compare(L"md3") == 0 || fileExtension.compare(L"md5") == 0 || 
			fileExtension.compare(L"mdc") == 0 || fileExtension.compare(L"mdl") == 0 || 
			fileExtension.compare(L"mesh") == 0 || fileExtension.compare(L"mot") == 0 || 
			fileExtension.compare(L"ms3d") == 0 || fileExtension.compare(L"ndo") == 0 || 
			fileExtension.compare(L"nff") == 0 || fileExtension.compare(L"obj") == 0 || 
			fileExtension.compare(L"off") == 0 || fileExtension.compare(L"ogex") == 0 || 
			fileExtension.compare(L"ply") == 0 || fileExtension.compare(L"pmx") == 0 || 
			fileExtension.compare(L"prj") == 0 || fileExtension.compare(L"q3s") == 0 || 
			fileExtension.compare(L"raw") == 0 || fileExtension.compare(L"scn") == 0 || 
			fileExtension.compare(L"sib") == 0 || fileExtension.compare(L"smd") == 0 || 
			fileExtension.compare(L"stp") == 0 || fileExtension.compare(L"stl") == 0 || 
			fileExtension.compare(L"ter") == 0 || fileExtension.compare(L"uc") == 0 ||
			fileExtension.compare(L"vta") == 0 || fileExtension.compare(L"x") == 0 || 
			fileExtension.compare(L"x3d") == 0 || fileExtension.compare(L"xgl") == 0 || 
			fileExtension.compare(L"pk3") == 0 || fileExtension.compare(L"zgl") == 0;
	}
	else return false;
}

bool LoadTexture(const aiScene* pScene, BaseMeshBuffer* meshBuffer, 
	bool textureMipmaps, TextureType textureType, aiString* textureName)
{
	//	If the texture is embedded, receives a '*' followed by the id of
	//	the texture(for the textures stored in the corresponding scene) which
	// can be converted to an int using a function like atoi.
	eastl::string texture = textureName->data;
	if (texture.substr(0, 1) == "*")
	{
		aiTexture* embeddedTexture =
			pScene->mTextures[atoi(texture.substr(1, texture.length() - 1).c_str())];

		if (embeddedTexture->mHeight == 0)
		{
			int width, height, components;

			unsigned char *imageData = stbi_load_from_memory(
				reinterpret_cast<unsigned char *>(embeddedTexture->pcData),
				embeddedTexture->mWidth, &width, &height, &components, STBI_rgb_alpha);
			if (imageData == nullptr)
			{
				LogError("load texture failed.");
				return false;
			}

			// R8G8B8A8 format with texels converted from the source format.
			DFType gtformat = DF_R8G8B8A8_UNORM;

			// Create the 2D texture and compute the stride and image size.
			eastl::shared_ptr<Texture2> meshTexture =
				eastl::make_shared<Texture2>(gtformat, width, height, textureMipmaps);
			meshTexture->SetName(ToWideString(texture.c_str()));
			UINT const stride = width * meshTexture->GetElementSize();
			UINT const imageSize = stride * height;

			// Copy the pixels from the decoder to the texture.
			std::memcpy(meshTexture->Get<BYTE>(), imageData, imageSize);
			stbi_image_free(imageData);

			if (textureMipmaps)
				meshTexture->AutogenerateMipmaps();
			meshBuffer->GetMaterial()->SetTexture(textureType, meshTexture);
		}
		else
		{
			// Create the 2D texture and compute the stride and image size.
			DFType gtformat = DF_R8G8B8A8_UNORM;
			eastl::shared_ptr<Texture2> meshTexture = eastl::make_shared<Texture2>(
				gtformat, embeddedTexture->mWidth, embeddedTexture->mHeight, textureMipmaps);
			meshTexture->SetName(ToWideString(texture.c_str()));

			// Copy the pixels from the decoder to the texture.
			//std::memcpy(meshTexture->Get<unsigned char>(), embeddedTexture->pcData, meshTexture->GetNumBytes());
			unsigned char* textureData = meshTexture->Get<unsigned char>();
			for (unsigned int i = 0; i <  embeddedTexture->mWidth * embeddedTexture->mHeight ; ++i)
			{
				aiTexel tx = embeddedTexture->pcData[i];
				*textureData++ = tx.r;
				*textureData++ = tx.g;
				*textureData++ = tx.b;
				*textureData++ = tx.a;
			}

			if (textureMipmaps)
				meshTexture->AutogenerateMipmaps();
			meshBuffer->GetMaterial()->SetTexture(textureType, meshTexture);
		}
		return true;
	}
	else if (!texture.empty())
	{
		int width, height, components;
		eastl::wstring directory = FileSystem::Get()->GetWorkingDirectory();
		eastl::string texturePath = textureName->C_Str();
		if (FileSystem::Get()->GetFileDir(ToWideString(textureName->C_Str())) == L".")
			texturePath = ToString(directory.c_str()) + "/" + texturePath;
		unsigned char *imageData = stbi_load(
			texturePath.c_str(), &width, &height, &components, STBI_rgb_alpha);
		if (imageData == nullptr)
		{
			LogError("load texture failed.");
			return false;
		}

		// R8G8B8A8 format with texels converted from the source format.
		DFType gtformat = DF_R8G8B8A8_UNORM;

		// Create the 2D texture and compute the stride and image size.
		eastl::shared_ptr<Texture2> meshTexture =
			eastl::make_shared<Texture2>(gtformat, width, height, textureMipmaps);
		meshTexture->SetName(ToWideString(texture.c_str()));
		UINT const stride = width * meshTexture->GetElementSize();
		UINT const imageSize = stride * height;

		// Copy the pixels from the decoder to the texture.
		std::memcpy(meshTexture->Get<BYTE>(), imageData, imageSize);
		stbi_image_free(imageData);

		if (textureMipmaps)
			meshTexture->AutogenerateMipmaps();
		meshBuffer->GetMaterial()->SetTexture(textureType, meshTexture);
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------
bool HasAlphaPixels(eastl::shared_ptr<Texture2> piTexture)
{
	for (unsigned int y = 0; y < piTexture->GetDimension(1); ++y)
	{
		for (unsigned int x = 0; x < piTexture->GetDimension(0); ++x)
		{
			char* tx = piTexture->GetData() + y * piTexture->GetDimension(0) + x;
			// compare alpha channel
			if (tx[3] != 0xFF)
				return true;

		}
	}
	return false;
}

//-------------------------------------------------------------------------------
void CreateMaterial(const aiScene* pScene, const aiMesh* pMesh, BaseMeshBuffer* meshBuffer, bool generateMipmaps)
{
	const aiMaterial* pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];

	// DIFFUSE COLOR
	aiColor4D diffuseColor;
	if (aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor) == aiReturn::aiReturn_SUCCESS)
		meshBuffer->GetMaterial()->mDiffuse = Vector4<float>{ diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a };

	// SPECULAR COLOR
	aiColor4D specularColor;
	if (aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_SPECULAR, &specularColor) == aiReturn::aiReturn_SUCCESS)
		meshBuffer->GetMaterial()->mSpecular = Vector4<float>{ specularColor.r, specularColor.g, specularColor.b, specularColor.a };

	// AMBIENT COLOR
	aiColor4D ambientColor;
	if (aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_AMBIENT, &ambientColor) == aiReturn::aiReturn_SUCCESS)
		meshBuffer->GetMaterial()->mAmbient = Vector4<float>{ ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a };

	// EMISSIVE COLOR
	aiColor4D emissiveColor;
	if (aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_EMISSIVE, &emissiveColor) == aiReturn::aiReturn_SUCCESS)
		meshBuffer->GetMaterial()->mEmissive = Vector4<float>{ emissiveColor.r, emissiveColor.g, emissiveColor.b, emissiveColor.a };

	// Opacity
	float opacity;
	if (aiGetMaterialFloat(pMaterial, AI_MATKEY_OPACITY, &opacity) == aiReturn::aiReturn_SUCCESS)
		meshBuffer->GetMaterial()->mType = (opacity > 0.5f) ? MT_SOLID : MT_TRANSPARENT;

	// Shading Model
	int shadingModel;
	bool defaultShadingModel = true;
	if (aiGetMaterialInteger(pMaterial, AI_MATKEY_SHADING_MODEL, &shadingModel) == aiReturn::aiReturn_SUCCESS)
	{
		defaultShadingModel = false;
		meshBuffer->GetMaterial()->mShadingModel = (ShadingModel)shadingModel;
	}

	// Shininess
	float shininess;
	if (aiGetMaterialFloat(pMaterial, AI_MATKEY_SHININESS, &shininess) == aiReturn::aiReturn_SUCCESS)
	{
		meshBuffer->GetMaterial()->mShininess = shininess;
		if (defaultShadingModel)
			meshBuffer->GetMaterial()->mShadingModel = (ShadingModel)aiShadingMode_Phong;
	}

	aiString texturePath;

	aiTextureMapMode mapU(aiTextureMapMode_Wrap), mapV(aiTextureMapMode_Wrap);

	bool bib = false;
	if (pMesh->mTextureCoords[0])
	{
		// DIFFUSE TEXTURE
		if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_TEXTURE_DIFFUSE(0), &texturePath))
		{
			LoadTexture(pScene, meshBuffer, generateMipmaps, TT_DIFFUSE, &texturePath);

			aiGetMaterialInteger(pMaterial, AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), (int*)&mapU);
			aiGetMaterialInteger(pMaterial, AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), (int*)&mapV);

			switch (mapU)
			{
				case aiTextureMapMode_Wrap:
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeU = SamplerState::WRAP;
					break;
				case aiTextureMapMode_Clamp:
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeU = SamplerState::CLAMP;
					break;
				case aiTextureMapMode_Decal:
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeU = SamplerState::BORDER;
					break;
				case aiTextureMapMode_Mirror:
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeU = SamplerState::MIRROR;
					break;
				default:
					break;
			}

			switch (mapV)
			{
				case aiTextureMapMode_Wrap:
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeV = SamplerState::WRAP;
					break;
				case aiTextureMapMode_Clamp:
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeV = SamplerState::CLAMP;
					break;
				case aiTextureMapMode_Decal:
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeV = SamplerState::BORDER;
					break;
				case aiTextureMapMode_Mirror:
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeV = SamplerState::MIRROR;
					break;
				default:
					break;
			}
		}

		// SPECULAR TEXTURE
		if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_TEXTURE_SPECULAR(0), &texturePath))
		{
			LoadTexture(pScene, meshBuffer, generateMipmaps, TT_SPECULAR, &texturePath);
		}

		// OPACITY TEXTURE
		if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_TEXTURE_OPACITY(0), &texturePath))
		{
			LoadTexture(pScene, meshBuffer, generateMipmaps, TT_OPACITY, &texturePath);
		}
		else
		{
			int flags = 0;
			aiGetMaterialInteger(pMaterial, AI_MATKEY_TEXFLAGS_DIFFUSE(0), &flags);

			// try to find out whether the diffuse texture has any
			// non-opaque pixels. If we find a few, use it as opacity texture
			eastl::shared_ptr<Texture2> diffuseTexture = meshBuffer->GetMaterial()->GetTexture(TT_DIFFUSE);
			if (diffuseTexture && !(flags & aiTextureFlags_IgnoreAlpha) && HasAlphaPixels(diffuseTexture))
			{
				int iVal;

				// NOTE: This special value is set by the tree view if the user
				// manually removes the alpha texture from the view ...
				if (AI_SUCCESS != aiGetMaterialInteger(pMaterial, "no_a_from_d", 0, 0, &iVal))
				{
					meshBuffer->GetMaterial()->SetTexture(TT_OPACITY, diffuseTexture);
				}
			}
		}

		// AMBIENT TEXTURE
		if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_TEXTURE_AMBIENT(0), &texturePath))
		{
			LoadTexture(pScene, meshBuffer, generateMipmaps, TT_AMBIENT, &texturePath);
		}

		// EMISSIVE TEXTURE
		if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_TEXTURE_EMISSIVE(0), &texturePath))
		{
			LoadTexture(pScene, meshBuffer, generateMipmaps, TT_EMISSIVE, &texturePath);
		}

		// SHININESS TEXTURE
		if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_TEXTURE_SHININESS(0), &texturePath))
		{
			LoadTexture(pScene, meshBuffer, generateMipmaps, TT_SHININESS, &texturePath);
		}

		// LIGHTMAP TEXTURE
		if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_TEXTURE_LIGHTMAP(0), &texturePath))
		{
			LoadTexture(pScene, meshBuffer, generateMipmaps, TT_LIGHTMAP, &texturePath);
		}

		// NORMAL/HEIGHT MAP
		bool bHM = false;
		if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_TEXTURE_NORMALS(0), &texturePath))
		{
			LoadTexture(pScene, meshBuffer, generateMipmaps, TT_NORMALS, &texturePath);
		}
		else
		{
			if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_TEXTURE_HEIGHT(0), &texturePath))
			{
				LoadTexture(pScene, meshBuffer, generateMipmaps, TT_NORMALS, &texturePath);
			}
			else bib = true;
			bHM = true;
		}

		// normal/height maps are sometimes mixed up. Try to detect the type
		// of the texture automatically
		/*
		eastl::shared_ptr<Texture2> normalTexture = meshBuffer->GetMaterial()->GetTexture(TT_NORMALS);
		if (normalTexture)
		{
			HMtoNMIfNecessary(pcMesh->piNormalTexture, &pcMesh->piNormalTexture, bHM);
		}
		*/
	}

	// check whether a global background texture is contained
	// in this material. Some loaders set this value ...
	/*
	if (AI_SUCCESS == aiGetMaterialString(pMaterial, AI_MATKEY_GLOBAL_BACKGROUND_IMAGE, &texturePath))
	{
		CBackgroundPainter::Instance().SetTextureBG(texturePath.data);
	}
	*/
	// BUGFIX: If the shininess is 0.0f disable phong lighting
	// This is a workaround for some meshes in the DX SDK (e.g. tiny.x)
	// FIX: Added this check to the x-loader, but the line remains to
	// catch other loader doing the same ...
	if (0.0f == shininess) 
	{
		meshBuffer->GetMaterial()->mShadingModel = (ShadingModel)aiShadingMode_Gouraud;
	}

	int culling;
	if (aiGetMaterialInteger(pMaterial, AI_MATKEY_TWOSIDED, &culling) == aiReturn::aiReturn_SUCCESS)
		meshBuffer->GetMaterial()->mCullMode = (culling != 0) ? RasterizerState::CULL_BACK : RasterizerState::CULL_NONE;
}


const aiNodeAnim* FindNodeAnim(const aiScene* pScene, aiString nodeName)
{
	for (unsigned int a = 0; a < pScene->mNumAnimations; a++)
	{
		for (unsigned int ch = 0; ch < pScene->mAnimations[a]->mNumChannels; ch++)
		{
			const aiNodeAnim* pNodeAnim = pScene->mAnimations[a]->mChannels[ch];

			if (pNodeAnim->mNodeName == nodeName)
				return pNodeAnim;
		}
	}

	return NULL;
}

void ReadNodeSkinMesh(const aiScene* pScene,
	const aiNode* pNode, const aiNode* pParentNode, SkinnedMesh* skinnedMesh)
{
	int jointNr = skinnedMesh->GetJointNumber(pNode->mName.C_Str());
	BaseSkinnedMesh::Joint* joint = jointNr != -1 ?
		skinnedMesh->GetAllJoints()[jointNr] : skinnedMesh->AddJoint();
	if (jointNr == -1)
	{
		joint->mName = pNode->mName.C_Str();

		// copy rotation matrix
		Matrix4x4<float> transformMatrix = Matrix4x4<float>::Identity();
		for (int row = 0; row < 3; ++row)
			for (int column = 0; column < 3; ++column)
				transformMatrix(row, column) = pNode->mTransformation[column][row];

		// copy position
		Vector3<float> translationVector = Vector3<float>::Zero();
		for (int row = 0; row < 3; ++row)
			translationVector[row] = pNode->mTransformation[row][3];

		// transforms the mesh vertices to the space of the node
		joint->mLocalTransform.SetRotation(transformMatrix);
		joint->mLocalTransform.SetTranslation(translationVector);
	}

	if (pParentNode != nullptr)
	{
		BaseSkinnedMesh::Joint* parentJoint = skinnedMesh->GetAllJoints()
			[skinnedMesh->GetJointNumber(pParentNode->mName.C_Str())];

		joint->mParent = parentJoint;
		parentJoint->mChildren.push_back(joint);
	}

	const aiNodeAnim* pNodeAnim = FindNodeAnim(pScene, pNode->mName);
	if (pNodeAnim)
	{
		for (unsigned p = 0; p < pNodeAnim->mNumPositionKeys; p++)
		{
			SkinnedMesh::PositionKey* positionKey = skinnedMesh->AddPositionKey(joint);
			positionKey->mFrame = (float)pNodeAnim->mPositionKeys[p].mTime;
			positionKey->mPosition = Vector3<float>{
				pNodeAnim->mPositionKeys[p].mValue[0],
				pNodeAnim->mPositionKeys[p].mValue[1],
				pNodeAnim->mPositionKeys[p].mValue[2]
			};
		}

		for (unsigned int s = 0; s < pNodeAnim->mNumScalingKeys; s++)
		{
			SkinnedMesh::ScaleKey* scaleKey = skinnedMesh->AddScaleKey(joint);
			scaleKey->mFrame = (float)pNodeAnim->mScalingKeys[s].mTime;
			scaleKey->mScale = Vector3<float>{
				pNodeAnim->mScalingKeys[s].mValue[0],
				pNodeAnim->mScalingKeys[s].mValue[1],
				pNodeAnim->mScalingKeys[s].mValue[2]
			};
		}

		for (unsigned int r = 0; r < pNodeAnim->mNumRotationKeys; r++)
		{
			SkinnedMesh::RotationKey* rotationKey = skinnedMesh->AddRotationKey(joint);
			rotationKey->mFrame = (float)pNodeAnim->mRotationKeys[r].mTime;
			rotationKey->mRotation.Set(
				-pNodeAnim->mRotationKeys[r].mValue.x,
				-pNodeAnim->mRotationKeys[r].mValue.y,
				-pNodeAnim->mRotationKeys[r].mValue.z,
				pNodeAnim->mRotationKeys[r].mValue.w);
			Normalize(rotationKey->mRotation);
		}
	}

	for (unsigned int n = 0; n < pNode->mNumChildren; n++)
		ReadNodeSkinMesh(pScene, pNode->mChildren[n], pNode, skinnedMesh);
}

void ReadNodeMesh(const aiScene* pScene,
	const aiNode* pNode, BaseMesh* mesh, eastl::wstring fileExtension)
{
	if (fileExtension == L"md3")
	{
		AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mesh);

		eastl::shared_ptr<MD3Mesh> meshMD3;
		if (pScene->mRootNode == pNode)
		{
			meshMD3 = eastl::make_shared<MD3Mesh>(pNode->mName.C_Str());
			animMeshMD3->SetMD3Mesh(meshMD3);
		}
		else
		{
			meshMD3 = animMeshMD3->GetMD3Mesh()->CreateMesh(
				pNode->mParent->mName.C_Str(), pNode->mName.C_Str());
		}

		aiVector3t<float> position;
		aiQuaterniont<float> rotation;
		pNode->mTransformation.DecomposeNoScaling(rotation, position);
		meshMD3->GetTagInterpolation().mPosition =
			Vector3<float>{ position.x, -position.z, position.y };
		meshMD3->GetTagInterpolation().mRotation =
			Quaternion<float>{ rotation.x, rotation.y, rotation.z, rotation.w };

		if (pNode->mMetaData != NULL)
		{
			aiString path, numAnimations;
			if (pNode->mMetaData->Get("path", path) == true &&
				pNode->mMetaData->Get("numanimations", numAnimations) == true)
			{
				meshMD3->LoadModel(ToWideString(path.C_Str()));

				unsigned int index = 2;
				for (int anim = 0; anim < atoi(numAnimations.C_Str()); anim++)
				{
					aiString type, start, end, loop, fps;
					pNode->mMetaData->Get(index++, type);
					pNode->mMetaData->Get(index++, start);
					pNode->mMetaData->Get(index++, end);
					pNode->mMetaData->Get(index++, loop);
					pNode->mMetaData->Get(index++, fps);

					AnimationData md3Animation;
					md3Animation.mBeginFrame = atoi(start.C_Str());
					md3Animation.mEndFrame = atoi(end.C_Str());
					md3Animation.mLoopFrame = atoi(loop.C_Str());
					md3Animation.mFramesPerSecond = (float)atoi(fps.C_Str());
					md3Animation.mAnimationType = atoi(type.C_Str());
					meshMD3->AddAnimation(md3Animation);
				}

				if (pScene->HasMaterials())
				{
					for (unsigned int m = 0; m < pNode->mNumMeshes; m++)
					{
						unsigned int* meshIndex = &(pNode->mMeshes[m]);
						aiMesh* mesh = pScene->mMeshes[*meshIndex];

						for (unsigned int mb = 0; mb < meshMD3->GetMeshBufferCount(); mb++)
						{
							eastl::shared_ptr<BaseMeshBuffer> meshBuffer = meshMD3->GetMeshBuffer(mb);
							if (meshBuffer->GetName() == ToWideString(mesh->mName.C_Str()))
							{
								CreateMaterial(pScene, mesh, meshBuffer.get(), true);
								break;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (unsigned int m = 0; m < pNode->mNumMeshes; m++)
		{
			unsigned int* meshIndex = &(pNode->mMeshes[m]);

			VertexFormat vformat;
			if (pScene->mMeshes[*meshIndex]->HasPositions())
				vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
			for (unsigned int ch = 0; ch < pScene->mMeshes[*meshIndex]->GetNumColorChannels(); ch++)
				if (pScene->mMeshes[*meshIndex]->HasVertexColors(ch))
					vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, ch);
			for (unsigned int ch = 0; ch < pScene->mMeshes[*meshIndex]->GetNumUVChannels(); ch++)
				if (pScene->mMeshes[*meshIndex]->HasTextureCoords(ch))
					vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, ch);
			if (pScene->mMeshes[*meshIndex]->HasNormals())
				vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
			if (pScene->mMeshes[*meshIndex]->HasTangentsAndBitangents())
				vformat.Bind(VA_TANGENT, DF_R32G32B32_FLOAT, 0);
			if (pScene->mMeshes[*meshIndex]->HasTangentsAndBitangents())
				vformat.Bind(VA_BINORMAL, DF_R32G32B32_FLOAT, 0);

			BaseMeshBuffer* meshBuffer = NULL;
			if (pScene->HasAnimations())
			{
				meshBuffer = new SkinMeshBuffer(
					vformat, pScene->mMeshes[*meshIndex]->mNumVertices,
					pScene->mMeshes[*meshIndex]->mNumFaces, sizeof(unsigned int));
			}
			else
			{
				meshBuffer = new MeshBuffer(
					vformat, pScene->mMeshes[*meshIndex]->mNumVertices,
					pScene->mMeshes[*meshIndex]->mNumFaces, sizeof(unsigned int));
			}
			meshBuffer->SetName(ToWideString(pScene->mMeshes[*meshIndex]->mName.C_Str()));

			for (unsigned int v = 0; v < pScene->mMeshes[*meshIndex]->mNumVertices; v++)
			{
				if (pScene->mMeshes[*meshIndex]->HasPositions())
				{
					const aiVector3D& position = pScene->mMeshes[*meshIndex]->mVertices[v];
					if (fileExtension == L"pk3")
						meshBuffer->Position(v) = Vector3<float>{ position.x, position.y, -position.z };
					else
						meshBuffer->Position(v) = Vector3<float>{ position.x, position.z, position.y };
				}
				if (pScene->mMeshes[*meshIndex]->HasNormals())
				{
					const aiVector3D& normal = pScene->mMeshes[*meshIndex]->mNormals[v];
					if (fileExtension == L"pk3")
						meshBuffer->Normal(v) = Vector3<float>{ normal.x, normal.y , -normal.z };
					else
						meshBuffer->Normal(v) = Vector3<float>{ normal.x, normal.z , normal.y };
				}
				if (pScene->mMeshes[*meshIndex]->HasTangentsAndBitangents())
				{
					const aiVector3D& tangent = pScene->mMeshes[*meshIndex]->mTangents[v];
					if (fileExtension == L"pk3")
						meshBuffer->Tangent(v) = Vector3<float>{ tangent.x, tangent.y , -tangent.z };
					else
						meshBuffer->Tangent(v) = Vector3<float>{ tangent.x, tangent.z , tangent.y };
				}
				if (pScene->mMeshes[*meshIndex]->HasTangentsAndBitangents())
				{
					const aiVector3D& bitangent = pScene->mMeshes[*meshIndex]->mBitangents[v];
					if (fileExtension == L"pk3")
						meshBuffer->Bitangent(v) = Vector3<float>{ bitangent.x, bitangent.y, -bitangent.z };
					else
						meshBuffer->Bitangent(v) = Vector3<float>{ bitangent.x, bitangent.z, bitangent.y };
				}
				for (unsigned int ch = 0; ch < pScene->mMeshes[*meshIndex]->GetNumColorChannels(); ch++)
				{
					if (pScene->mMeshes[*meshIndex]->HasVertexColors(ch))
					{
						const aiColor4D& color = pScene->mMeshes[*meshIndex]->mColors[ch][v];
						meshBuffer->Color(ch, v) = Vector4<float>{ color.r, color.g, color.b, color.a };
					}
				}
				for (unsigned int ch = 0; ch < pScene->mMeshes[*meshIndex]->GetNumUVChannels(); ch++)
				{
					if (pScene->mMeshes[*meshIndex]->HasTextureCoords(ch))
					{
						const aiVector3D& texCoord = pScene->mMeshes[*meshIndex]->mTextureCoords[ch][v];
						meshBuffer->TCoord(ch, v) = Vector2<float>{ texCoord.x, texCoord.y };
					}
				}
			}

			for (unsigned int f = 0; f < pScene->mMeshes[*meshIndex]->mNumFaces; f++)
			{
				const aiFace& face = pScene->mMeshes[*meshIndex]->mFaces[f];
				LogAssert(face.mNumIndices == 3, "Invalid number of indices");
				meshBuffer->GetIndice()->SetTriangle(f, face.mIndices[0], face.mIndices[1], face.mIndices[2]);
			}

			if (pScene->mMeshes[*meshIndex]->HasBones())
			{
				SkinnedMesh* skinnedMesh = dynamic_cast<SkinnedMesh*>(mesh);
				for (unsigned int b = 0; b < pScene->mMeshes[*meshIndex]->mNumBones; b++)
				{
					aiBone* bone = pScene->mMeshes[*meshIndex]->mBones[b];
					aiNode* node = pScene->mRootNode->FindNode(bone->mName);

					BaseSkinnedMesh::Joint* joint = skinnedMesh->AddJoint();
					joint->mName = bone->mName.C_Str();

					// copy rotation matrix
					Matrix4x4<float> transformMatrix = Matrix4x4<float>::Identity();
					for (int row = 0; row < 3; ++row)
						for (int column = 0; column < 3; ++column)
							transformMatrix(row, column) = node->mTransformation[column][row];

					// copy position
					Vector3<float> translationVector = Vector3<float>::Zero();
					for (int row = 0; row < 3; ++row)
						translationVector[row] = node->mTransformation[row][3];

					// transforms the mesh vertices to the space of the node
					joint->mLocalTransform.SetRotation(transformMatrix);
					joint->mLocalTransform.SetTranslation(translationVector);

					if (!bone->mOffsetMatrix.IsIdentity())
					{
						transformMatrix = Matrix4x4<float>::Identity();
						for (int row = 0; row < 3; ++row)
							for (int column = 0; column < 3; ++column)
								transformMatrix(row, column) = bone->mOffsetMatrix[column][row];

						translationVector = Vector3<float>::Zero();
						for (int row = 0; row < 3; ++row)
							translationVector[row] = bone->mOffsetMatrix[row][3];

						joint->mGlobalInversedTransform.SetRotation(transformMatrix);
						joint->mGlobalInversedTransform.SetTranslation(translationVector);
					}

					joint->mAttachedMeshes.push_back(mesh->GetMeshBufferCount());
					for (unsigned int w = 0; w < bone->mNumWeights; w++)
					{
						BaseSkinnedMesh::Weight weight;
						weight.mBufferId = mesh->GetMeshBufferCount();
						weight.mVertexId = bone->mWeights[w].mVertexId;
						weight.mStrength = bone->mWeights[w].mWeight;
						joint->mWeights.push_back(weight);
					}
				}
			}

			if (pScene->HasMaterials())
			{
				bool generateMipmaps = dynamic_cast<StandardMesh*>(mesh) == NULL;
				CreateMaterial(pScene, pScene->mMeshes[*meshIndex], meshBuffer, generateMipmaps);
			}

			mesh->AddMeshBuffer(meshBuffer);
		}
	}
	
	for (unsigned int n = 0; n < pNode->mNumChildren; n++)
		ReadNodeMesh(pScene, pNode->mChildren[n], mesh, fileExtension);
}

// -------------------------------------------------------------------------------
BaseMesh* MeshFileLoader::CreateMesh(BaseReadFile* file)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably want to request more postprocessing than we do in this example.
	const aiScene* pScene = importer.ReadFile(
		ToString(file->GetFileName().c_str()).c_str(),
		aiProcess_ConvertToLeftHanded | //this is necessary for Direct3D rendering
		aiProcessPreset_TargetRealtime_Fast );
	// If the import failed, report it
	if (!pScene)
	{
		LogError(importer.GetErrorString());
		return false;
	}
	
	eastl::wstring saveDir = FileSystem::Get()->GetWorkingDirectory();
	FileSystem::Get()->ChangeWorkingDirectoryTo(
		FileSystem::Get()->GetFileDir(file->GetFileName()));
	eastl::wstring fileExtension = 
		file->GetFileName().substr(file->GetFileName().rfind('.') + 1);

	BaseMesh * mesh = NULL;
	if (pScene->HasAnimations())
		mesh = new SkinnedMesh();
	else if (fileExtension == L"md3")
		mesh = new AnimateMeshMD3();
	else
		mesh = new StandardMesh();

	ReadNodeMesh(pScene, pScene->mRootNode, mesh, fileExtension);

	if (fileExtension == L"pk3")
	{
		for (unsigned int i = 0; i<mesh->GetMeshBufferCount(); ++i)
		{
			const eastl::shared_ptr<BaseMeshBuffer>& meshBuffer = mesh->GetMeshBuffer(i);
			if (meshBuffer)
			{
				eastl::shared_ptr<Texture2> textureDiffuse = meshBuffer->GetMaterial()->GetTexture(TT_DIFFUSE);
				if (textureDiffuse)
				{
					if (!meshBuffer->GetMaterial()->GetTexture(TT_LIGHTMAP))
					{
						meshBuffer->GetMaterial()->mBlendTarget.enable = true;
						meshBuffer->GetMaterial()->mBlendTarget.srcColor = BlendState::BM_ONE;
						meshBuffer->GetMaterial()->mBlendTarget.dstColor = BlendState::BM_INV_SRC_COLOR;
						meshBuffer->GetMaterial()->mBlendTarget.srcAlpha = BlendState::BM_SRC_ALPHA;
						meshBuffer->GetMaterial()->mBlendTarget.dstAlpha = BlendState::BM_INV_SRC_ALPHA;

						meshBuffer->GetMaterial()->mDepthBuffer = true;
						meshBuffer->GetMaterial()->mDepthMask = DepthStencilState::MASK_ZERO;
						meshBuffer->GetMaterial()->mType = MT_TRANSPARENT;
					}
				}
				else
				{
					meshBuffer->GetMaterial()->mBlendTarget.enable = true;
					meshBuffer->GetMaterial()->mBlendTarget.srcColor = BlendState::BM_ONE;
					meshBuffer->GetMaterial()->mBlendTarget.dstColor = BlendState::BM_INV_SRC_COLOR;
					meshBuffer->GetMaterial()->mBlendTarget.srcAlpha = BlendState::BM_SRC_ALPHA;
					meshBuffer->GetMaterial()->mBlendTarget.dstAlpha = BlendState::BM_INV_SRC_ALPHA;

					meshBuffer->GetMaterial()->mDepthBuffer = true;
					meshBuffer->GetMaterial()->mDepthMask = DepthStencilState::MASK_ZERO;
					meshBuffer->GetMaterial()->mType = MT_TRANSPARENT;
				}
			}
		}
	}

	if (mesh->GetMeshType() == MT_SKINNED)
	{
		SkinnedMesh* skinnedMesh = dynamic_cast<SkinnedMesh*>(mesh);

		ReadNodeSkinMesh(pScene, pScene->mRootNode, nullptr, skinnedMesh);
		skinnedMesh->Finalize();
	}

	FileSystem::Get()->ChangeWorkingDirectoryTo(saveDir);
	return mesh;
}