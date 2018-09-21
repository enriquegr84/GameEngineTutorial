// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "GameEngineStd.h"

#include "MeshFileLoader.h"

#include "Graphic/Image/ImageResource.h"
#include "Graphic/Scene/Element/Mesh/AnimatedMesh.h"
#include "Graphic/Scene/Element/Mesh/StandardMesh.h"

#include "Core/Logger/Logger.h"
#include "Core/Utility/StringUtil.h"

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

// MD3MeshFileLoader::GetLoadedResourceSize
unsigned int MeshFileLoader::GetLoadedResourceSize(void *rawBuffer, unsigned int rawSize)
{
	// This will keep the resource cache from allocating memory for the texture, so DirectX can manage it on it's own.
	return rawSize;
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
		return !fileExtension.empty();
	}
	else return false;
}

AnimatedMesh* MeshFileLoader::CreateMesh(BaseReadFile* file)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* pScene = importer.ReadFile(
		ToString(file->GetFileName().c_str()).c_str(),
		aiProcess_Triangulate |
		aiProcess_OptimizeMeshes |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);
	// If the import failed, report it
	if (!pScene)
	{
		LogError(importer.GetErrorString());
		return false;
	}

	AnimatedMesh * aMesh = new AnimatedMesh();

	for (unsigned int m = 0; m<pScene->mNumMeshes; m++)
	{
		eastl::shared_ptr<StandardMesh> mesh(new StandardMesh());

		VertexFormat vformat;
		if (pScene->mMeshes[m]->HasPositions())
			vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
		if (pScene->mMeshes[m]->HasNormals())
			vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
		if (pScene->mMeshes[m]->HasTangentsAndBitangents())
			vformat.Bind(VA_TANGENT, DF_R32G32B32_FLOAT, 0);
		if (pScene->mMeshes[m]->HasTangentsAndBitangents())
			vformat.Bind(VA_BINORMAL, DF_R32G32B32_FLOAT, 0);
		for (unsigned int ch = 0; ch < pScene->mMeshes[m]->GetNumColorChannels(); ch++)
			if (pScene->mMeshes[m]->HasVertexColors(ch))
				vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, ch);
		for (unsigned int ch = 0; ch < pScene->mMeshes[m]->GetNumUVChannels(); ch++)
			if (pScene->mMeshes[m]->HasTextureCoords(ch))
				vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, ch);

		eastl::shared_ptr<MeshBuffer> meshBuffer(
			new MeshBuffer(vformat, pScene->mMeshes[m]->mNumVertices, 
			pScene->mMeshes[m]->mNumFaces, sizeof(unsigned int)));
		for (unsigned int v = 0; v < pScene->mMeshes[m]->mNumVertices; v++)
		{
			if (pScene->mMeshes[m]->HasPositions())
			{
				const aiVector3D& position = pScene->mMeshes[m]->mVertices[v];
				meshBuffer->Position(v) = Vector3<float>{ position.x, position.z, position.y };
			}
			if (pScene->mMeshes[m]->HasNormals())
			{
				const aiVector3D& normal = pScene->mMeshes[m]->mNormals[v];
				meshBuffer->Normal(v) = Vector3<float>{ normal.x, normal.z , normal.y };
			}
			if (pScene->mMeshes[m]->HasTangentsAndBitangents())
			{
				const aiVector3D& tangent = pScene->mMeshes[m]->mTangents[v];
				meshBuffer->Tangent(v) = Vector3<float>{ tangent.x, tangent.z , tangent.y };
			}
			if (pScene->mMeshes[m]->HasTangentsAndBitangents())
			{
				const aiVector3D& bitangent = pScene->mMeshes[m]->mBitangents[v];
				meshBuffer->Bitangent(v) = Vector3<float>{ bitangent.x, bitangent.z, bitangent.y };
			}	
			for (unsigned int ch = 0; ch < pScene->mMeshes[m]->GetNumColorChannels(); ch++)
			{
				if (pScene->mMeshes[m]->HasVertexColors(ch))
				{
					const aiColor4D& color = pScene->mMeshes[m]->mColors[ch][v];
					meshBuffer->Color(ch, v) = Vector4<float>{ color.r, color.g, color.b, color.a };
				}
			}
			for (unsigned int ch = 0; ch < pScene->mMeshes[m]->GetNumUVChannels(); ch++)
			{
				if (pScene->mMeshes[m]->HasTextureCoords(ch))
				{
					const aiVector3D& texCoord = pScene->mMeshes[m]->mTextureCoords[ch][v];
					meshBuffer->TCoord(ch, v) = Vector2<float>{ texCoord.x, 1-texCoord.y };
				}
			}
		}
		for (unsigned int f = 0; f < pScene->mMeshes[m]->mNumFaces; f++)
		{
			const aiFace& face = pScene->mMeshes[m]->mFaces[f];
			LogAssert(face.mNumIndices == 3, "Invalid number of indices");
			meshBuffer->GetIndice()->SetTriangle(f, face.mIndices[0], face.mIndices[1], face.mIndices[2]);
		}

		if (pScene->HasMaterials())
		{
			const aiMaterial* material = pScene->mMaterials[pScene->mMeshes[m]->mMaterialIndex];

			aiColor4D diffuseColor;
			aiColor4D ambientColor;
			aiColor4D specularColor;
			aiColor4D emissiveColor;
			aiColor4D transparentColor;
			int wireframe, culling, shadingModel;
			float opacity, shininess;

			if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mDiffuse = Vector4<float>{ diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a };
			if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambientColor) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mAmbient = Vector4<float>{ ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a };
			if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specularColor) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mSpecular = Vector4<float>{ specularColor.r, specularColor.g, specularColor.b, specularColor.a };
			if (aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emissiveColor) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mEmissive = Vector4<float>{ emissiveColor.r, emissiveColor.g, emissiveColor.b, emissiveColor.a };
			//if (aiGetMaterialColor(material, AI_MATKEY_COLOR_TRANSPARENT, &transparentColor) == aiReturn::aiReturn_SUCCESS)

			if (aiGetMaterialInteger(material, AI_MATKEY_TWOSIDED, &culling) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mRasterizerState->mCullMode = (culling != 0) ? RasterizerState::CULL_BACK : RasterizerState::CULL_NONE;
			if (aiGetMaterialInteger(material, AI_MATKEY_ENABLE_WIREFRAME, &wireframe) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mRasterizerState->mFillMode = (wireframe != 0) ? RasterizerState::FILL_WIREFRAME : RasterizerState::FILL_SOLID;
			if (aiGetMaterialInteger(material, AI_MATKEY_SHADING_MODEL, &shadingModel) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mShadingModel = (ShadingModel)shadingModel;
			if (aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mType = (opacity > 0.5f) ? MT_TRANSPARENT_ADD_COLOR : MT_SOLID;
			if (aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mShininess = shininess;

			int textureIdx = 0;
			aiTextureType textureTypes[] = { 
				aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_AMBIENT, 
				aiTextureType_EMISSIVE, aiTextureType_HEIGHT, aiTextureType_NORMALS, 
				aiTextureType_SHININESS, aiTextureType_OPACITY, aiTextureType_DISPLACEMENT, 
				aiTextureType_LIGHTMAP, aiTextureType_REFLECTION };
			for (auto textureType : textureTypes)
			{
				for (unsigned int idx = 0; idx < material->GetTextureCount(textureType); idx++)
				{
					aiString texturePath;
					aiTextureOp textureOperation;
					aiTextureMapMode textureMapMode;
					aiTextureMapping textureMapping;
					ai_real blend;
					unsigned int uvindex;

					material->GetTexture(textureType, idx, &texturePath, &textureMapping, &uvindex, &blend, &textureOperation, &textureMapMode);
					for (unsigned int propIdx = 0; propIdx < material->mNumProperties; propIdx++)
					{
						if (!eastl::string(material->mProperties[propIdx]->mKey.C_Str()).compare(_AI_MATKEY_TEXTURE_BASE))
						{
							eastl::shared_ptr<ResHandle>& resHandle =
								ResCache::Get()->GetHandle(&BaseResource(L"Art/" + ToWideString(texturePath.C_Str())));
							if (resHandle)
							{
								const eastl::shared_ptr<ImageResourceExtraData>& extra =
									eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
								extra->GetImage()->AutogenerateMipmaps();
								meshBuffer->GetMaterial()->SetTexture(textureIdx, extra->GetImage());
							}
						}
						else if (!eastl::string(material->mProperties[propIdx]->mKey.C_Str()).compare(_AI_MATKEY_UVWSRC_BASE))
						{

						}
						else if (!eastl::string(material->mProperties[propIdx]->mKey.C_Str()).compare(_AI_MATKEY_TEXOP_BASE))
						{

						}
						else if (!eastl::string(material->mProperties[propIdx]->mKey.C_Str()).compare(_AI_MATKEY_MAPPING_BASE))
						{

						}
						else if (!eastl::string(material->mProperties[propIdx]->mKey.C_Str()).compare(_AI_MATKEY_TEXBLEND_BASE))
						{

						}
						else if (!eastl::string(material->mProperties[propIdx]->mKey.C_Str()).compare(_AI_MATKEY_MAPPINGMODE_U_BASE))
						{
							switch (textureMapMode)
							{
								case aiTextureMapMode_Wrap:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mSamplerState->mMode[0] = SamplerState::WRAP;
									break;
								case aiTextureMapMode_Clamp:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mSamplerState->mMode[0] = SamplerState::CLAMP;
									break;
								case aiTextureMapMode_Decal:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mSamplerState->mMode[0] = SamplerState::BORDER;
									break;
								case aiTextureMapMode_Mirror:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mSamplerState->mMode[0] = SamplerState::MIRROR;
									break;
								default:
									break;
							}
						}
						else if (!eastl::string(material->mProperties[propIdx]->mKey.C_Str()).compare(_AI_MATKEY_MAPPINGMODE_V_BASE))
						{
							switch (textureMapMode)
							{
								case aiTextureMapMode_Wrap:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mSamplerState->mMode[1] = SamplerState::WRAP;
									break;
								case aiTextureMapMode_Clamp:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mSamplerState->mMode[1] = SamplerState::CLAMP;
									break;
								case aiTextureMapMode_Decal:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mSamplerState->mMode[1] = SamplerState::BORDER;
									break;
								case aiTextureMapMode_Mirror:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mSamplerState->mMode[1] = SamplerState::MIRROR;
									break;
								default:
									break;
							}
						}
					}

					textureIdx++;
				}
			}
		}

		mesh->mMeshBuffer = meshBuffer;
		aMesh->AddMesh(mesh);
	}

	return aMesh;
}