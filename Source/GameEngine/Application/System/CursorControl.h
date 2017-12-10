// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#ifndef CURSORCONTROL_H
#define CURSORCONTROL_H

#include "Mathematic/Algebra/Vector2.h"
#include "Mathematic/Geometric/Rectangle.h"

class BaseUISpriteBank;

//! Default icons for cursors
enum CursorIcon
{
	// Following cursors might be system specific, or might use an GameEngine icon-set. No guarantees so far.
	CI_NORMAL,	// arrow
	CI_CROSS,	// Crosshair
	CI_HAND,	// Hand
	CI_HELP,	// Arrow and question mark
	CI_IBEAM,	// typical text-selection cursor
	CI_NO,		// should not click icon
	CI_WAIT,	// hourclass
	CI_SIZEALL,	// arrow in all directions
	CI_SIZENESW,	// resizes in direction north-east or south-west
	CI_SIZENWSE,	// resizes in direction north-west or south-east
	CI_SIZENS,	// resizes in direction north or south
	CI_SIZEWE,	// resizes in direction west or east
	CI_UP,		// up-arrow

	// Implementer note: Should we add system specific cursors, which use guaranteed the system icons,
	// then I would recommend using a naming scheme like CI_W32_CROSS, CI_X11_CROSSHAIR and adding those
	// additionally.
	CI_COUNT		// maximal of defined cursors. Note that higher values can be created at runtime
};

//! platform specific behavior flags for the cursor
enum CursorPlatformBehavior
{
	//! default - no platform specific behavior
	CPB_NONE = 0,

	//! On X11 try caching cursor updates as XQueryPointer calls can be expensive.
	/** Update cursor positions only when the GameEngine timer has been updated or the timer is stopped.
	This means you usually get one cursor update per device->run() which will be fine in most cases.
	*/
	CPB_X11_CACHE_UPDATES = 1
};

//! structure used to set sprites as cursors.
struct CursorSprite
{
	CursorSprite()
		: mSpriteBank(0), mSpriteId(-1)
	{
	}

	CursorSprite(BaseUISpriteBank * spriteBank, int spriteId, const Vector2<int> &hotspot = (Vector2<int>{0, 0}))
		: mSpriteBank(spriteBank), mSpriteId(spriteId), mHotSpot(hotspot)
	{
	}

	int mSpriteId;
	Vector2<int> mHotSpot;
	BaseUISpriteBank* mSpriteBank;
};

//! Interface to manipulate the mouse cursor.
class BaseCursorControl : public eastl::enable_shared_from_this<BaseCursorControl>
{
public:

	//! Changes the visible state of the mouse cursor.
	/** \param visible: The new visible state. If true, the cursor will be visible,
	if false, it will be invisible. */
	virtual void SetVisible(bool visible) = 0;

	//! Returns if the cursor is currently visible.
	/** \return True if the cursor is visible, false if not. */
	virtual bool IsVisible() const = 0;

	//! Sets the new position of the cursor.
	/** The position must be
	between (0.0f, 0.0f) and (1.0f, 1.0f), where (0.0f, 0.0f) is
	the top left corner and (1.0f, 1.0f) is the bottom right corner of the
	render window.
	\param pos New position of the cursor. */
	virtual void SetPosition(const Vector2<float> &pos) = 0;

	//! Sets the new position of the cursor.
	/** \param pos: New position of the cursor. The coordinates are pixel units. */
	virtual void SetPosition(const Vector2<int> &pos) = 0;

	//! Sets the new position of the cursor.
	/** The position must be
	between (0.0f, 0.0f) and (1.0f, 1.0f), where (0.0f, 0.0f) is
	the top left corner and (1.0f, 1.0f) is the bottom right corner of the
	render window.
	\param x New x-coord of the cursor.
	\param y New x-coord of the cursor. */
	virtual void SetPosition(float x, float y) = 0;

	//! Sets the new position of the cursor.
	/** \param x New x-coord of the cursor. The coordinates are pixel units.
	\param y New y-coord of the cursor. The coordinates are pixel units. */
	virtual void SetPosition(int x, int y) = 0;

	//! Returns the current position of the mouse cursor.
	/** \return Returns the current position of the cursor. The returned position
	is the position of the mouse cursor in pixel units. */
	virtual const Vector2<unsigned int>& GetPosition() = 0;

	//! Returns the current position of the mouse cursor.
	/** \return Returns the current position of the cursor. The returned position
	is a value between (0.0f, 0.0f) and (1.0f, 1.0f), where (0.0f, 0.0f) is
	the top left corner and (1.0f, 1.0f) is the bottom right corner of the
	render window. */
	virtual Vector2<float> GetRelativePosition() = 0;

	//! Sets an absolute reference rect for setting and retrieving the cursor position.
	/** If this rect is set, the cursor position is not being calculated relative to
	the rendering window but to this rect. You can set the rect pointer to 0 to disable
	this feature again. This feature is useful when rendering into parts of foreign windows
	for example in an editor.
	\param rect: A pointer to an reference rectangle or 0 to disable the reference rectangle.*/
	virtual void SetReferenceRect(const RectangleShape<2, int>* rect = 0) = 0;

	//! Sets the active cursor icon
	/** Setting cursor icons is so far only supported on Win32 and Linux */
	virtual void SetActiveIcon(CursorIcon iconId) {}

	//! Gets the currently active icon
	virtual CursorIcon GetActiveIcon() const { return CI_NORMAL; }

	//! Add a custom sprite as cursor icon.
	/** \return Identification for the icon */
	virtual CursorIcon AddIcon(const CursorSprite& icon) { return CI_NORMAL; }

	//! replace a cursor icon.
	/** Changing cursor icons is so far only supported on Win32 and Linux
	Note that this only changes the icons within your application, system cursors outside your
	application will not be affected.*/
	virtual void ChangeIcon(CursorIcon iconId, const CursorSprite& sprite) {}

	//! Return a system-specific size which is supported for cursors. Larger icons will fail, smaller icons might work.
	virtual Vector2<int> GetSupportedIconSize() const { return Vector2<int>{0, 0}; }

	//! Set platform specific behavior flags.
	virtual void SetPlatformBehavior(CursorPlatformBehavior behavior) {}

	//! Return platform specific behavior.
	/** \return Behavior set by setPlatformBehavior or ECPB_NONE for platforms not implementing specific behaviors.*/
	virtual CursorPlatformBehavior GetPlatformBehavior() const { return CPB_NONE; }
};

#endif //CURSORCONTROL_H