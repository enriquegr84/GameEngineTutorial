// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "GameEngineStd.h"

#include "MeshFileLoader.h"

#include "Graphic/Scene/Element/Mesh/AnimatedMesh.h"
#include "Graphic/Scene/Element/Mesh/StandardMesh.h"

#include "Core/Logger/Logger.h"
#include "Core/Utility/StringUtil.h"

#include <Importer.hpp>      // C++ importer interface
#include <scene.h>           // Output data structure
#include <postprocess.h>     // Post processing flags

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
				meshBuffer->Position(v) = Vector3<float>{ position.x, position.y, position.z };
			}
			if (pScene->mMeshes[m]->HasNormals())
			{
				const aiVector3D& normal = pScene->mMeshes[m]->mNormals[v];
				meshBuffer->Normal(v) = Vector3<float>{ normal.x, normal.y, normal.z };
			}
			if (pScene->mMeshes[m]->HasTangentsAndBitangents())
			{
				const aiVector3D& tangent = pScene->mMeshes[m]->mTangents[v];
				meshBuffer->Tangent(v) = Vector3<float>{ tangent.x, tangent.y, tangent.z };
			}
			if (pScene->mMeshes[m]->HasTangentsAndBitangents())
			{
				const aiVector3D& bitangent = pScene->mMeshes[m]->mBitangents[v];
				meshBuffer->Bitangent(v) = Vector3<float>{ bitangent.x, bitangent.y, bitangent.z };
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
					meshBuffer->TCoord(ch, v) = Vector2<float>{ texCoord.x, texCoord.y };
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
			float opacity;

			aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambientColor);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specularColor);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emissiveColor);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_TRANSPARENT, &transparentColor);
			aiGetMaterialInteger(material, AI_MATKEY_TWOSIDED, &culling);
			aiGetMaterialInteger(material, AI_MATKEY_ENABLE_WIREFRAME, &wireframe);
			aiGetMaterialInteger(material, AI_MATKEY_SHADING_MODEL, &shadingModel);
			aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity);
			aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &meshBuffer->GetMaterial()->mShininess);

			meshBuffer->GetMaterial()->mType = (opacity > 0.5f) ? MT_TRANSPARENT_ADD_COLOR : MT_SOLID;
			meshBuffer->GetMaterial()->mRasterizerState->mCullMode =
				(culling != 0) ? RasterizerState::CULL_BACK : RasterizerState::CULL_NONE;
			meshBuffer->GetMaterial()->mRasterizerState->mFillMode = 
				(wireframe != 0) ? RasterizerState::FILL_WIREFRAME : RasterizerState::FILL_SOLID;
			meshBuffer->GetMaterial()->mShadingModel = (ShadingModel)shadingModel;
			meshBuffer->GetMaterial()->mDiffuse = Vector4<float>{ diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a };
			meshBuffer->GetMaterial()->mAmbient = Vector4<float>{ ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a };
			meshBuffer->GetMaterial()->mSpecular = Vector4<float>{ specularColor.r, specularColor.g, specularColor.b, specularColor.a };
			meshBuffer->GetMaterial()->mEmissive = Vector4<float>{ emissiveColor.r, emissiveColor.g, emissiveColor.b, emissiveColor.a };
		}

		mesh->mMeshBuffer = meshBuffer;
		aMesh->AddMesh(mesh);
	}

	return aMesh;
}