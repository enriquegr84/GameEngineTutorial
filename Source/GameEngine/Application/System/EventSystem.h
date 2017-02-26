// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#ifndef EVENTSYSTEM_H
#define EVENTSYSTEM_H

static unsigned int LocaleIdToCodepage(unsigned int lcid)
{
	switch (lcid)
	{
	case 1098:  // Telugu
	case 1095:  // Gujarati
	case 1094:  // Punjabi
	case 1103:  // Sanskrit
	case 1111:  // Konkani
	case 1114:  // Syriac
	case 1099:  // Kannada
	case 1102:  // Marathi
	case 1125:  // Divehi
	case 1067:  // Armenian
	case 1081:  // Hindi
	case 1079:  // Georgian
	case 1097:  // Tamil
		return 0;
	case 1054:  // Thai
		return 874;
	case 1041:  // Japanese
		return 932;
	case 2052:  // Chinese (PRC)
	case 4100:  // Chinese (Singapore)
		return 936;
	case 1042:  // Korean
		return 949;
	case 5124:  // Chinese (Macau S.A.R.)
	case 3076:  // Chinese (Hong Kong S.A.R.)
	case 1028:  // Chinese (Taiwan)
		return 950;
	case 1048:  // Romanian
	case 1060:  // Slovenian
	case 1038:  // Hungarian
	case 1051:  // Slovak
	case 1045:  // Polish
	case 1052:  // Albanian
	case 2074:  // Serbian (Latin)
	case 1050:  // Croatian
	case 1029:  // Czech
		return 1250;
	case 1104:  // Mongolian (Cyrillic)
	case 1071:  // FYRO Macedonian
	case 2115:  // Uzbek (Cyrillic)
	case 1058:  // Ukrainian
	case 2092:  // Azeri (Cyrillic)
	case 1092:  // Tatar
	case 1087:  // Kazakh
	case 1059:  // Belarusian
	case 1088:  // Kyrgyz (Cyrillic)
	case 1026:  // Bulgarian
	case 3098:  // Serbian (Cyrillic)
	case 1049:  // Russian
		return 1251;
	case 8201:  // English (Jamaica)
	case 3084:  // French (Canada)
	case 1036:  // French (France)
	case 5132:  // French (Luxembourg)
	case 5129:  // English (New Zealand)
	case 6153:  // English (Ireland)
	case 1043:  // Dutch (Netherlands)
	case 9225:  // English (Caribbean)
	case 4108:  // French (Switzerland)
	case 4105:  // English (Canada)
	case 1110:  // Galician
	case 10249:  // English (Belize)
	case 3079:  // German (Austria)
	case 6156:  // French (Monaco)
	case 12297:  // English (Zimbabwe)
	case 1069:  // Basque
	case 2067:  // Dutch (Belgium)
	case 2060:  // French (Belgium)
	case 1035:  // Finnish
	case 1080:  // Faroese
	case 1031:  // German (Germany)
	case 3081:  // English (Australia)
	case 1033:  // English (United States)
	case 2057:  // English (United Kingdom)
	case 1027:  // Catalan
	case 11273:  // English (Trinidad)
	case 7177:  // English (South Africa)
	case 1030:  // Danish
	case 13321:  // English (Philippines)
	case 15370:  // Spanish (Paraguay)
	case 9226:  // Spanish (Colombia)
	case 5130:  // Spanish (Costa Rica)
	case 7178:  // Spanish (Dominican Republic)
	case 12298:  // Spanish (Ecuador)
	case 17418:  // Spanish (El Salvador)
	case 4106:  // Spanish (Guatemala)
	case 18442:  // Spanish (Honduras)
	case 3082:  // Spanish (International Sort)
	case 13322:  // Spanish (Chile)
	case 19466:  // Spanish (Nicaragua)
	case 2058:  // Spanish (Mexico)
	case 10250:  // Spanish (Peru)
	case 20490:  // Spanish (Puerto Rico)
	case 1034:  // Spanish (Traditional Sort)
	case 14346:  // Spanish (Uruguay)
	case 8202:  // Spanish (Venezuela)
	case 1089:  // Swahili
	case 1053:  // Swedish
	case 2077:  // Swedish (Finland)
	case 5127:  // German (Liechtenstein)
	case 1078:  // Afrikaans
	case 6154:  // Spanish (Panama)
	case 4103:  // German (Luxembourg)
	case 16394:  // Spanish (Bolivia)
	case 2055:  // German (Switzerland)
	case 1039:  // Icelandic
	case 1057:  // Indonesian
	case 1040:  // Italian (Italy)
	case 2064:  // Italian (Switzerland)
	case 2068:  // Norwegian (Nynorsk)
	case 11274:  // Spanish (Argentina)
	case 1046:  // Portuguese (Brazil)
	case 1044:  // Norwegian (Bokmal)
	case 1086:  // Malay (Malaysia)
	case 2110:  // Malay (Brunei Darussalam)
	case 2070:  // Portuguese (Portugal)
		return 1252;
	case 1032:  // Greek
		return 1253;
	case 1091:  // Uzbek (Latin)
	case 1068:  // Azeri (Latin)
	case 1055:  // Turkish
		return 1254;
	case 1037:  // Hebrew
		return 1255;
	case 5121:  // Arabic (Algeria)
	case 15361:  // Arabic (Bahrain)
	case 9217:  // Arabic (Yemen)
	case 3073:  // Arabic (Egypt)
	case 2049:  // Arabic (Iraq)
	case 11265:  // Arabic (Jordan)
	case 13313:  // Arabic (Kuwait)
	case 12289:  // Arabic (Lebanon)
	case 4097:  // Arabic (Libya)
	case 6145:  // Arabic (Morocco)
	case 8193:  // Arabic (Oman)
	case 16385:  // Arabic (Qatar)
	case 1025:  // Arabic (Saudi Arabia)
	case 10241:  // Arabic (Syria)
	case 14337:  // Arabic (U.A.E.)
	case 1065:  // Farsi
	case 1056:  // Urdu
	case 7169:  // Arabic (Tunisia)
		return 1256;
	case 1061:  // Estonian
	case 1062:  // Latvian
	case 1063:  // Lithuanian
		return 1257;
	case 1066:  // Vietnamese
		return 1258;
	}
	return 65001;   // utf-8
}

enum KeyCode
{
	KEY_LBUTTON = 0x01,  // Left mouse button
	KEY_RBUTTON = 0x02,  // Right mouse button
	KEY_CANCEL = 0x03,  // Control-break processing
	KEY_MBUTTON = 0x04,  // Middle mouse button (three-button mouse)
	KEY_XBUTTON1 = 0x05,  // Windows 2000/XP: X1 mouse button
	KEY_XBUTTON2 = 0x06,  // Windows 2000/XP: X2 mouse button
	KEY_BACK = 0x08,  // BACKSPACE key
	KEY_TAB = 0x09,  // TAB key
	KEY_CLEAR = 0x0C,  // CLEAR key
	KEY_RETURN = 0x0D,  // ENTER key
	KEY_SHIFT = 0x10,  // SHIFT key
	KEY_CONTROL = 0x11,  // CTRL key
	KEY_MENU = 0x12,  // ALT key
	KEY_PAUSE = 0x13,  // PAUSE key
	KEY_CAPITAL = 0x14,  // CAPS LOCK key
	KEY_KANA = 0x15,  // IME Kana mode
	KEY_HANGUEL = 0x15,  // IME Hanguel mode (maintained for compatibility use KEY_HANGUL)
	KEY_HANGUL = 0x15,  // IME Hangul mode
	KEY_JUNJA = 0x17,  // IME Junja mode
	KEY_FINAL = 0x18,  // IME final mode
	KEY_HANJA = 0x19,  // IME Hanja mode
	KEY_KANJI = 0x19,  // IME Kanji mode
	KEY_ESCAPE = 0x1B,  // ESC key
	KEY_CONVERT = 0x1C,  // IME convert
	KEY_NONCONVERT = 0x1D,  // IME nonconvert
	KEY_ACCEPT = 0x1E,  // IME accept
	KEY_MODECHANGE = 0x1F,  // IME mode change request
	KEY_SPACE = 0x20,  // SPACEBAR
	KEY_PRIOR = 0x21,  // PAGE UP key
	KEY_NEXT = 0x22,  // PAGE DOWN key
	KEY_END = 0x23,  // END key
	KEY_HOME = 0x24,  // HOME key
	KEY_LEFT = 0x25,  // LEFT ARROW key
	KEY_UP = 0x26,  // UP ARROW key
	KEY_RIGHT = 0x27,  // RIGHT ARROW key
	KEY_DOWN = 0x28,  // DOWN ARROW key
	KEY_SELECT = 0x29,  // SELECT key
	KEY_PRINT = 0x2A,  // PRINT key
	KEY_EXECUT = 0x2B,  // EXECUTE key
	KEY_SNAPSHOT = 0x2C,  // PRINT SCREEN key
	KEY_INSERT = 0x2D,  // INS key
	KEY_DELETE = 0x2E,  // DEL key
	KEY_HELP = 0x2F,  // HELP key
	KEY_KEY_0 = 0x30,  // 0 key
	KEY_KEY_1 = 0x31,  // 1 key
	KEY_KEY_2 = 0x32,  // 2 key
	KEY_KEY_3 = 0x33,  // 3 key
	KEY_KEY_4 = 0x34,  // 4 key
	KEY_KEY_5 = 0x35,  // 5 key
	KEY_KEY_6 = 0x36,  // 6 key
	KEY_KEY_7 = 0x37,  // 7 key
	KEY_KEY_8 = 0x38,  // 8 key
	KEY_KEY_9 = 0x39,  // 9 key
	KEY_KEY_A = 0x41,  // A key
	KEY_KEY_B = 0x42,  // B key
	KEY_KEY_C = 0x43,  // C key
	KEY_KEY_D = 0x44,  // D key
	KEY_KEY_E = 0x45,  // E key
	KEY_KEY_F = 0x46,  // F key
	KEY_KEY_G = 0x47,  // G key
	KEY_KEY_H = 0x48,  // H key
	KEY_KEY_I = 0x49,  // I key
	KEY_KEY_J = 0x4A,  // J key
	KEY_KEY_K = 0x4B,  // K key
	KEY_KEY_L = 0x4C,  // L key
	KEY_KEY_M = 0x4D,  // M key
	KEY_KEY_N = 0x4E,  // N key
	KEY_KEY_O = 0x4F,  // O key
	KEY_KEY_P = 0x50,  // P key
	KEY_KEY_Q = 0x51,  // Q key
	KEY_KEY_R = 0x52,  // R key
	KEY_KEY_S = 0x53,  // S key
	KEY_KEY_T = 0x54,  // T key
	KEY_KEY_U = 0x55,  // U key
	KEY_KEY_V = 0x56,  // V key
	KEY_KEY_W = 0x57,  // W key
	KEY_KEY_X = 0x58,  // X key
	KEY_KEY_Y = 0x59,  // Y key
	KEY_KEY_Z = 0x5A,  // Z key
	KEY_LWIN = 0x5B,  // Left Windows key (Microsoft® Natural® keyboard)
	KEY_RWIN = 0x5C,  // Right Windows key (Natural keyboard)
	KEY_APPS = 0x5D,  // Applications key (Natural keyboard)
	KEY_SLEEP = 0x5F,  // Computer Sleep key
	KEY_NUMPAD0 = 0x60,  // Numeric keypad 0 key
	KEY_NUMPAD1 = 0x61,  // Numeric keypad 1 key
	KEY_NUMPAD2 = 0x62,  // Numeric keypad 2 key
	KEY_NUMPAD3 = 0x63,  // Numeric keypad 3 key
	KEY_NUMPAD4 = 0x64,  // Numeric keypad 4 key
	KEY_NUMPAD5 = 0x65,  // Numeric keypad 5 key
	KEY_NUMPAD6 = 0x66,  // Numeric keypad 6 key
	KEY_NUMPAD7 = 0x67,  // Numeric keypad 7 key
	KEY_NUMPAD8 = 0x68,  // Numeric keypad 8 key
	KEY_NUMPAD9 = 0x69,  // Numeric keypad 9 key
	KEY_MULTIPLY = 0x6A,  // Multiply key
	KEY_ADD = 0x6B,  // Add key
	KEY_SEPARATOR = 0x6C,  // Separator key
	KEY_SUBTRACT = 0x6D,  // Subtract key
	KEY_DECIMAL = 0x6E,  // Decimal key
	KEY_DIVIDE = 0x6F,  // Divide key
	KEY_F1 = 0x70,  // F1 key
	KEY_F2 = 0x71,  // F2 key
	KEY_F3 = 0x72,  // F3 key
	KEY_F4 = 0x73,  // F4 key
	KEY_F5 = 0x74,  // F5 key
	KEY_F6 = 0x75,  // F6 key
	KEY_F7 = 0x76,  // F7 key
	KEY_F8 = 0x77,  // F8 key
	KEY_F9 = 0x78,  // F9 key
	KEY_F10 = 0x79,  // F10 key
	KEY_F11 = 0x7A,  // F11 key
	KEY_F12 = 0x7B,  // F12 key
	KEY_F13 = 0x7C,  // F13 key
	KEY_F14 = 0x7D,  // F14 key
	KEY_F15 = 0x7E,  // F15 key
	KEY_F16 = 0x7F,  // F16 key
	KEY_F17 = 0x80,  // F17 key
	KEY_F18 = 0x81,  // F18 key
	KEY_F19 = 0x82,  // F19 key
	KEY_F20 = 0x83,  // F20 key
	KEY_F21 = 0x84,  // F21 key
	KEY_F22 = 0x85,  // F22 key
	KEY_F23 = 0x86,  // F23 key
	KEY_F24 = 0x87,  // F24 key
	KEY_NUMLOCK = 0x90,  // NUM LOCK key
	KEY_SCROLL = 0x91,  // SCROLL LOCK key
	KEY_LSHIFT = 0xA0,  // Left SHIFT key
	KEY_RSHIFT = 0xA1,  // Right SHIFT key
	KEY_LCONTROL = 0xA2,  // Left CONTROL key
	KEY_RCONTROL = 0xA3,  // Right CONTROL key
	KEY_LMENU = 0xA4,  // Left MENU key
	KEY_RMENU = 0xA5,  // Right MENU key
	KEY_OEM_1 = 0xBA,  // for US    ";:"
	KEY_PLUS = 0xBB,  // Plus Key   "+"
	KEY_COMMA = 0xBC,  // Comma Key  ","
	KEY_MINUS = 0xBD,  // Minus Key  "-"
	KEY_PERIOD = 0xBE,  // Period Key "."
	KEY_OEM_2 = 0xBF,  // for US    "/?"
	KEY_OEM_3 = 0xC0,  // for US    "`~"
	KEY_OEM_4 = 0xDB,  // for US    "[{"
	KEY_OEM_5 = 0xDC,  // for US    "\|"
	KEY_OEM_6 = 0xDD,  // for US    "]}"
	KEY_OEM_7 = 0xDE,  // for US    "'""
	KEY_OEM_8 = 0xDF,  // None
	KEY_OEM_AX = 0xE1,  // for Japan "AX"
	KEY_OEM_102 = 0xE2,  // "<>" or "\|"
	KEY_ATTN = 0xF6,  // Attn key
	KEY_CRSEL = 0xF7,  // CrSel key
	KEY_EXSEL = 0xF8,  // ExSel key
	KEY_EREOF = 0xF9,  // Erase EOF key
	KEY_PLAY = 0xFA,  // Play key
	KEY_ZOOM = 0xFB,  // Zoom key
	KEY_PA1 = 0xFD,  // PA1 key
	KEY_OEM_CLEAR = 0xFE,   // Clear key

	KEY_KEY_CODES_COUNT = 0xFF // this is not a key, but the amount of keycodes there are.
};

//! Enumeration for all event types there are.
enum CORE_ITEM EventType
{
	/*
	GUI events are created by the GUI environment or the GUI elements in response to mouse
	or keyboard events. When a GUI element receives an event it will either process it and
	return true, or pass the event to its parent. If an event is not absorbed before it
	reaches the root element then it will then be passed to the user receiver.
	*/
	//! An event of the graphical user interface.
	ET_GUI_EVENT = 0,

	/*
	Mouse events are created by the device and passed to Device::PostEvent in response to
	mouse input received from the operating system. Mouse events are first passed to the user
	receiver, then to the GUI environment and its elements, then finally the input receiving
	scene manager where it is passed to the active camera.
	*/
	//! A mouse input event.
	ET_MOUSE_INPUT_EVENT,


	/*
	Like mouse events, keyboard events are created by the device and passed to Device::PostEvent.
	They take the same path as mouse events.
	*/
	//! A key input event.
	ET_KEY_INPUT_EVENT,

	/*
	Joystick events are created by polling all connected joysticks once per device run() and
	then passing the events to Device::PostEvent. They take the same path as mouse events.
	*/
	//! A joystick (joypad, gamepad) input event.
	ET_JOYSTICK_INPUT_EVENT,

	/*
	Log events are only passed to the user receiver if there is one. If they are absorbed by
	the user receiver then no text will be sent to the console.
	*/
	//! A log event
	ET_LOG_TEXT_EVENT,

	/*
	This is not used by GameEngine and can be used to send user specific data though the system.
	The usage and behavior depends on the operating system:

	Windows: send a WM_USER message to the GameEngine Window; the wParam and lParam will be used
	to populate the UserData1 and UserData2 members of the struct UserEvent.

	Linux: send a ClientMessage via XSendEvent to the GameEngine window; the data.l[0] and
	data.l[1] members will be casted to signed int and used as UserData1 and UserData2.

	MacOS: Not yet implemented
	*/
	//! A user event with user data.
	ET_USER_EVENT

};

//! Enumeration for all mouse input events
enum MouseInputEvent
{
	//! Left mouse button was pressed down.
	MIE_LMOUSE_PRESSED_DOWN = 0,

	//! Right mouse button was pressed down.
	MIE_RMOUSE_PRESSED_DOWN,

	//! Middle mouse button was pressed down.
	MIE_MMOUSE_PRESSED_DOWN,

	//! Left mouse button was left up.
	MIE_LMOUSE_LEFT_UP,

	//! Right mouse button was left up.
	MIE_RMOUSE_LEFT_UP,

	//! Middle mouse button was left up.
	MIE_MMOUSE_LEFT_UP,

	//! The mouse cursor changed its position.
	MIE_MOUSE_MOVED,

	//! The mouse wheel was moved. Use Wheel value in event data to find out
	//! in what direction and how fast.
	MIE_MOUSE_WHEEL,

	//! Left mouse button double click.
	//! This event is generated after the second MIE_LMOUSE_PRESSED_DOWN event.
	MIE_LMOUSE_DOUBLE_CLICK,

	//! Right mouse button double click.
	//! This event is generated after the second MIE_RMOUSE_PRESSED_DOWN event.
	MIE_RMOUSE_DOUBLE_CLICK,

	//! Middle mouse button double click.
	//! This event is generated after the second MIE_MMOUSE_PRESSED_DOWN event.
	MIE_MMOUSE_DOUBLE_CLICK,

	//! Left mouse button triple click.
	//! This event is generated after the third MIE_LMOUSE_PRESSED_DOWN event.
	MIE_LMOUSE_TRIPLE_CLICK,

	//! Right mouse button triple click.
	//! This event is generated after the third MIE_RMOUSE_PRESSED_DOWN event.
	MIE_RMOUSE_TRIPLE_CLICK,

	//! Middle mouse button triple click.
	//! This event is generated after the third MIE_MMOUSE_PRESSED_DOWN event.
	MIE_MMOUSE_TRIPLE_CLICK,

	//! No real event. Just for convenience to get number of events
	MIE_COUNT
};

//! Masks for mouse button states
enum MouseButtonStateMask
{
	MBSM_LEFT = 0x01,
	MBSM_RIGHT = 0x02,
	MBSM_MIDDLE = 0x04,

	//! currently only on windows
	MBSM_EXTRA1 = 0x08,

	//! currently only on windows
	MBSM_EXTRA2 = 0x10,

	MBSM_FORCE_32_BIT = 0x7fffffff
};

class GUIElement;

//! Enumeration for all events which are sendable by the gui system
enum GuiEventType
{
	//! No real event. Just for convenience to get number of events
	GET_COUNT
};

//! Events hold information about an event. See EventReceiver for details on event handling.
struct Event
{
	//! Any kind of GUI event.
	struct GUIEvent
	{
		//! IGUIElement who called the event
		GUIElement* mCaller;

		//! If the event has something to do with another element, it will be held here.
		GUIElement* mElement;

		//! Type of GUI Event
		GuiEventType mEventType;

	};

	//! Any kind of mouse event.
	struct MouseInput
	{
		//! X position of mouse cursor
		signed int X;

		//! Y position of mouse cursor
		signed int Y;

		/* Only valid if event was MIE_MOUSE_WHEEL */
		//! mouse wheel delta, often 1.0 or -1.0, but can have other values < 0.f or > 0.f;
		float mWheel;

		//! True if shift was also pressed
		bool mShift : 1;

		//! True if ctrl was also pressed
		bool mControl : 1;

		//! A bitmap of button states. You can use isButtonPressed() to determine
		//! if a button is pressed or not.
		//! Currently only valid if the event was MIE_MOUSE_MOVED
		unsigned int mButtonStates;

		//! Is the left button pressed down?
		bool IsLeftPressed() const { return 0 != (mButtonStates & MBSM_LEFT); }

		//! Is the right button pressed down?
		bool IsRightPressed() const { return 0 != (mButtonStates & MBSM_RIGHT); }

		//! Is the middle button pressed down?
		bool IsMiddlePressed() const { return 0 != (mButtonStates & MBSM_MIDDLE); }

		//! Type of mouse event
		MouseInputEvent mEvent;
	};

	//! Any kind of keyboard event.
	struct KeyInput
	{
		//! Character corresponding to the key (0, if not a character)
		wchar_t mChar;

		//! Key which has been pressed or released
		KeyCode mKey;

		//! If not true, then the key was left up
		bool mPressedDown : 1;

		//! True if shift was also pressed
		bool mShift : 1;

		//! True if ctrl was also pressed
		bool mControl : 1;
	};

	//! Any kind of user event.
	struct UserEvent
	{
		//! Some user specified data as int
		signed int mUserData1;

		//! Another user specified data as int
		signed int mUserData2;
	};

	EventType mEventType;
	union
	{
		struct GUIEvent		mGUIEvent;
		struct MouseInput	mMouseInput;
		struct KeyInput		mKeyInput;
		struct UserEvent	mUserEvent;
	};

};

/*
Supporting system for interactive application posses the capability to accept events. They can be
from different kind, such device input (mouse and keyboard) graphic user interface (gui) or user
events. The GameEngine is a event-driven system in which any subsystem can listen to declared
events and process them on their convenience. Events usually start at a PostEvent function and are
passed down through a chain of event receivers until OnEvent returns true. See EEVENT_TYPE for a
description of where each type of event starts, and the path it takes through the system.
*/
//! Interface of an object which can listen events.
class EventListener
{
public:

	//! Destructor
	virtual ~EventListener() {}

	/*
	Please take care that the method only returns 'true' inr order to prevent the GameEngine
	from processing the event any further. So 'true' does mean that an event is completely done.
	Therefore your return value for all unprocessed events should be 'false'.
	\return True if the event was processed.
	*/
	//! Called if an event happened.
	virtual bool OnEvent(const Event& ev) = 0;
};

#endif //EVENTSYSTEM_H