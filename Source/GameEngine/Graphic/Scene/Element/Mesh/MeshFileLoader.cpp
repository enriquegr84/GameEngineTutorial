// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "GameEngineStd.h"

#include "MeshFileLoader.h"

#include "Graphic/Image/ImageResource.h"
#include "Graphic/Scene/Element/Mesh/SkinnedMesh.h"
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

void ReadNodeHeirarchy(const aiScene* pScene, 
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
		ReadNodeHeirarchy(pScene, pNode->mChildren[n], pNode, skinnedMesh);
}

// -------------------------------------------------------------------------------
BaseMesh* MeshFileLoader::CreateMesh(BaseReadFile* file)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
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

	BaseMesh * mesh = NULL;
	if (pScene->HasAnimations())
		mesh = new SkinnedMesh();
	else
		mesh = new StandardMesh();

	for (unsigned int m = 0; m<pScene->mNumMeshes; m++)
	{
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

		BaseMeshBuffer* meshBuffer = NULL;
		if (pScene->HasAnimations())
		{
			meshBuffer = new SkinMeshBuffer(
				vformat, pScene->mMeshes[m]->mNumVertices,
				pScene->mMeshes[m]->mNumFaces, sizeof(unsigned int));
		}
		else
		{
			meshBuffer = new MeshBuffer(
				vformat, pScene->mMeshes[m]->mNumVertices,
				pScene->mMeshes[m]->mNumFaces, sizeof(unsigned int));
		}

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
				meshBuffer->GetMaterial()->mCullMode = (culling != 0) ? RasterizerState::CULL_BACK : RasterizerState::CULL_NONE;
			if (aiGetMaterialInteger(material, AI_MATKEY_ENABLE_WIREFRAME, &wireframe) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mFillMode = (wireframe != 0) ? RasterizerState::FILL_WIREFRAME : RasterizerState::FILL_SOLID;
			if (aiGetMaterialInteger(material, AI_MATKEY_SHADING_MODEL, &shadingModel) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mShadingModel = (ShadingModel)shadingModel;
			if (aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity) == aiReturn::aiReturn_SUCCESS)
				meshBuffer->GetMaterial()->mType = (opacity > 0.5f) ? MT_SOLID : MT_TRANSPARENT;
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
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mModeU = SamplerState::WRAP;
									break;
								case aiTextureMapMode_Clamp:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mModeU = SamplerState::CLAMP;
									break;
								case aiTextureMapMode_Decal:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mModeU = SamplerState::BORDER;
									break;
								case aiTextureMapMode_Mirror:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mModeU = SamplerState::MIRROR;
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
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mModeV = SamplerState::WRAP;
									break;
								case aiTextureMapMode_Clamp:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mModeV = SamplerState::CLAMP;
									break;
								case aiTextureMapMode_Decal:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mModeV = SamplerState::BORDER;
									break;
								case aiTextureMapMode_Mirror:
									meshBuffer->GetMaterial()->mTextureLayer[textureIdx].mModeV = SamplerState::MIRROR;
									break;
								default:
									break;
							}
						}
					}

					textureIdx++;
				}
			}

			if (pScene->mMeshes[m]->HasBones())
			{
				SkinnedMesh* skinnedMesh = dynamic_cast<SkinnedMesh*>(mesh);
				for (unsigned int b = 0; b < pScene->mMeshes[m]->mNumBones; b++)
				{
					aiBone* bone = pScene->mMeshes[m]->mBones[b];
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
		}

		mesh->AddMeshBuffer(meshBuffer);
	}
	
	if (mesh->GetMeshType() == MT_SKINNED)
	{
		SkinnedMesh* skinnedMesh = dynamic_cast<SkinnedMesh*>(mesh);

		ReadNodeHeirarchy(pScene, pScene->mRootNode, nullptr, skinnedMesh);
		skinnedMesh->Finalize();
	}

	return mesh;
}