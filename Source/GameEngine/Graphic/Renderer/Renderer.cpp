// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#include "Graphic/Graphic.h"

#include "Renderer.h"

Renderer* Renderer::mRenderer = NULL;

Renderer* Renderer::Get(void)
{
	LogAssert(Renderer::mRenderer, "Renderer doesn't exist");
	return Renderer::mRenderer;
}

//----------------------------------------------------------------------------
Renderer::Renderer()
	:
	mScreenSize(),
	mClearDepth(1.0f),
	mClearStencil(0),
	mCreateDrawTarget(nullptr),
	mGraphicObjectCreator(nullptr)
{
	mClearColor.fill(1.0f);
	mCreateGraphicObject.fill(nullptr);

	mGOListener = eastl::make_shared<GOListener>(this);
	GraphicObject::SubscribeForDestruction(mGOListener);

	mDTListener = eastl::make_shared<DTListener>(this);
	DrawTarget::SubscribeForDestruction(mDTListener);

	if (Renderer::mRenderer)
	{
		LogError("Attempting to create two global renderer! \
					The old one will be destroyed and overwritten with this one.");
		delete Renderer::mRenderer;
	}

	Renderer::mRenderer = this;
}
//----------------------------------------------------------------------------
Renderer::~Renderer()
{
	if (Renderer::mRenderer == this)
		Renderer::mRenderer = nullptr;
}
//----------------------------------------------------------------------------
void Renderer::SetFont(eastl::shared_ptr<Font> const& font)
{
	if (font)
	{
		if (font != mActiveFont)
		{
			// Destroy font resources in GPU memory. The mActiveFont should
			// be null once, only when the mDefaultFont is created.
			if (mActiveFont)
			{
				Unbind(mActiveFont->GetVertexBuffer());
				Unbind(mActiveFont->GetIndexBuffer());
				Unbind(mActiveFont->GetTextEffect()->GetTranslate());
				Unbind(mActiveFont->GetTextEffect()->GetColor());
				Unbind(mActiveFont->GetTextEffect()->GetVertexShader());
				Unbind(mActiveFont->GetTextEffect()->GetPixelShader());
			}

			mActiveFont = font;

			// Create font resources in GPU memory.
			Bind(mActiveFont->GetVertexBuffer());
			Bind(mActiveFont->GetIndexBuffer());
			Bind(mActiveFont->GetTextEffect()->GetTranslate());
			Bind(mActiveFont->GetTextEffect()->GetColor());
			Bind(mActiveFont->GetTextEffect()->GetVertexShader());
			Bind(mActiveFont->GetTextEffect()->GetPixelShader());
		}
	}
	else
	{
		LogError("Input font is null.");
	}
}


void Renderer::CreateDefaultGlobalState()
{
	mDefaultBlendState = eastl::make_shared<BlendState>();
	mDefaultDepthStencilState = eastl::make_shared<DepthStencilState>();
	mDefaultRasterizerState = eastl::make_shared<RasterizerState>();

	SetDefaultBlendState();
	SetDefaultDepthStencilState();
	SetDefaultRasterizerState();
}

void Renderer::DestroyDefaultGlobalState()
{
	if (mDefaultBlendState)
	{
		Unbind(mDefaultBlendState);
	}

	if (mDefaultDepthStencilState)
	{
		Unbind(mDefaultDepthStencilState);
	}

	if (mDefaultRasterizerState)
	{
		Unbind(mDefaultRasterizerState);
	}

	mDefaultBlendState = nullptr;
	mActiveBlendState = nullptr;
	mDefaultDepthStencilState = nullptr;
	mActiveDepthStencilState = nullptr;
	mDefaultRasterizerState = nullptr;
	mActiveRasterizerState = nullptr;
}

uint64_t Renderer::Draw(Visual* visual)
{
	if (visual)
	{
		auto const& vbuffer = visual->GetVertexBuffer();
		auto const& ibuffer = visual->GetIndexBuffer();
		auto const& effect = visual->GetEffect();
		if (vbuffer && ibuffer && effect)
		{
			return DrawPrimitive(vbuffer, ibuffer, effect);
		}
	}

	LogError("Null input to Draw.");
	return 0;
}

uint64_t Renderer::Draw(eastl::vector<Visual*> const& visuals)
{
	uint64_t numPixelsDrawn = 0;
	for (auto const& visual : visuals)
	{
		numPixelsDrawn += Draw(visual);
	}
	return numPixelsDrawn;
}

uint64_t Renderer::Draw(eastl::shared_ptr<Visual> const& visual)
{
	return Draw(visual.get());
}

uint64_t Renderer::Draw(eastl::vector<eastl::shared_ptr<Visual>> const& visuals)
{
	uint64_t numPixelsDrawn = 0;
	for (auto const& visual : visuals)
	{
		numPixelsDrawn += Draw(visual);
	}
	return numPixelsDrawn;
}

uint64_t Renderer::Draw(int x, int y, eastl::array<float, 4> const& color, eastl::wstring const& message)
{
	uint64_t numPixelsDrawn;

	if (message.length() > 0)
	{
		int vx, vy, vw, vh;
		GetViewport(vx, vy, vw, vh);
		mActiveFont->Typeset(vw, vh, x, y, color, message);

		Update(mActiveFont->GetTextEffect()->GetTranslate());
		Update(mActiveFont->GetTextEffect()->GetColor());
		Update(mActiveFont->GetVertexBuffer());

		// We need to restore default state for text drawing.  Remember the
		// current state so that we can reset it after drawing.
		eastl::shared_ptr<BlendState> bState = GetBlendState();
		eastl::shared_ptr<DepthStencilState> dState = GetDepthStencilState();
		eastl::shared_ptr<RasterizerState> rState = GetRasterizerState();
		SetDefaultBlendState();
		SetDefaultDepthStencilState();
		SetDefaultRasterizerState();

		numPixelsDrawn = DrawPrimitive(mActiveFont->GetVertexBuffer(),
			mActiveFont->GetIndexBuffer(), mActiveFont->GetTextEffect());

		SetBlendState(bState);
		SetDepthStencilState(dState);
		SetRasterizerState(rState);
	}
	else
	{
		numPixelsDrawn = 0;
	}

	return numPixelsDrawn;
}

GraphicObject* Renderer::Bind(eastl::shared_ptr<GraphicObject> const& object)
{
	if (!object)
	{
		LogError("Attempt to bind a null object.");
		return nullptr;
	}

	GraphicObject const* gObject = object.get();
	eastl::shared_ptr<GraphicObject> geObject;
	if (!mGraphicObjects.Get(gObject, geObject))
	{
		// The 'create' function is not null with the current engine design.
		// If the assertion is triggered, someone changed the hierarchy of
		// GraphicsObjectType but did not change msCreateFunctions[] to match.
		CreateGraphicObject create = mCreateGraphicObject[object->GetType()];
		if (!create)
		{
			// No logger message is generated here because GL4 does not
			// have shader creation functions.
			return nullptr;
		}

		geObject = create(mGraphicObjectCreator, gObject);
		LogAssert(gObject, "Null object.  Out of memory?");

		mGraphicObjects.Insert(gObject, geObject);
	}
	return geObject.get();
}

DrawTarget* Renderer::Bind(eastl::shared_ptr<DrawTarget> const& target)
{
	DrawTarget const* gTarget = target.get();
	eastl::shared_ptr<DrawTarget> geTarget;
	if (!mDrawTargets.Get(gTarget, geTarget))
	{
		unsigned int const numTargets = target->GetNumTargets();
		eastl::vector<GraphicObject*> rtTextures(numTargets);
		for (unsigned int i = 0; i < numTargets; ++i)
		{
			rtTextures[i] = static_cast<GraphicObject*>(Bind(target->GetRTTexture(i)));
		}

		eastl::shared_ptr<TextureDS> object = target->GetDSTexture();
		GraphicObject* dsTexture;
		if (object)
		{
			dsTexture = static_cast<GraphicObject*>(Bind(object));
		}
		else
		{
			dsTexture = nullptr;
		}

		geTarget = mCreateDrawTarget(gTarget, rtTextures, dsTexture);
		mDrawTargets.Insert(gTarget, geTarget);
	}
	return geTarget.get();
}

GraphicObject* Renderer::Get(eastl::shared_ptr<GraphicObject> const& object) const
{
	GraphicObject const* gObject = object.get();
	eastl::shared_ptr<GraphicObject> geObject;
	if (mGraphicObjects.Get(gObject, geObject))
	{
		return geObject.get();
	}
	return nullptr;
}

DrawTarget* Renderer::Get(eastl::shared_ptr<DrawTarget> const& target) const
{
	DrawTarget const* gTarget = target.get();
	eastl::shared_ptr<DrawTarget> geTarget;
	if (!mDrawTargets.Get(gTarget, geTarget))
	{
		LogWarning("Cannot find draw target.");
	}
	return geTarget.get();
}

bool Renderer::Unbind(GraphicObject const* object)
{
	eastl::shared_ptr<GraphicObject> gxObject;
	if (mGraphicObjects.Get(object, gxObject))
	{
		GraphicObjectType type = object->GetType();
		if (type == GE_VERTEX_BUFFER)
		{
			mInputLayouts->Unbind(static_cast<VertexBuffer const*>(object));
		}
		else if (type == GE_VERTEX_SHADER)
		{
			mInputLayouts->Unbind(static_cast<Shader const*>(object));
		}

		if (mGraphicObjects.Remove(object, gxObject))
		{
			return true;
		}
	}
	return false;
}

bool Renderer::Unbind(DrawTarget const* target)
{
	eastl::shared_ptr<DrawTarget> dxTarget = nullptr;
	if (mDrawTargets.Remove(target, dxTarget))
	{
		return true;
	}

	return false;
}

Renderer::GOListener::~GOListener()
{
}

Renderer::GOListener::GOListener(Renderer* renderer)
	:	
	mRenderer(renderer)
{
}

void Renderer::GOListener::OnDestroy(GraphicObject const* object)
{
	if (mRenderer)
	{
		mRenderer->Unbind(object);
	}
}

Renderer::DTListener::~DTListener()
{
}

Renderer::DTListener::DTListener(Renderer* renderer)
	:	
	mRenderer(renderer)
{
}

void Renderer::DTListener::OnDestroy(DrawTarget const* target)
{
	if (mRenderer)
	{
		mRenderer->Unbind(target);
	}
}