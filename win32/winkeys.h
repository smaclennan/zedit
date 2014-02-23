static Byte virt[] = {
	0,
	0,		/* 0x01 VK_LBUTTON */
	0,		/* 0x02 VK_RBUTTON */
	0,		/* 0x03 VK_CANCEL */
	0,		/* 0x04 VK_MBUTTON */
	0,		/* 0x05 VK_XBUTTON1 */
	0,		/* 0x06 VK_XBUTTON2 */
	0,		/* 0x07 Undefined */
	127,		/* 0x08 VK_BACK */
	'\t',		/* 0x09 VK_TAB */
	0,		/* 0x0A-0B Reserved */
	0,		/* 0x0A-0B Reserved */
	0,		/* 0x0C VK_CLEAR */
	'\r',		/* 0x0D VK_RETURN */
	0,		/* 0x0E-0F Undefined */
	0,		/* 0x0E-0F Undefined */
	0,		/* 0x10 VK_SHIFT */
	0,		/* 0x11 VK_CONTROL */
	0,		/* 0x12 VK_MENU */
	0,		/* 0x13 VK_PAUSE */
	0,		/* 0x14 VK_CAPITAL */
	0,		/* 0x15 VK_KANA/VK_HANGUL */
	0,		/* 0x16 Undefined */
	0,		/* 0x17 VK_JUNJA */
	0,		/* 0x18 VK_FINAL */
	0,		/* 0x19 VK_HANJA/VK_KANJI */
	0,		/* 0x1A Undefined */
	'\033',		/* 0x1B VK_ESCAPE */
	0,		/* 0x1C VK_CONVERT */
	0,		/* 0x1D VK_NONCONVERT */
	0,		/* 0x1E VK_ACCEPT */
	0,		/* 0x1F VK_MODECHANGE */
	' ',		/* 0x20 VK_SPACE */
	TC_PGUP,		/* 0x21 VK_PRIOR */
	TC_PGDOWN,		/* 0x22 VK_NEXT */
	TC_END,		/* 0x23 VK_END */
	TC_HOME,	/* 0x24 VK_HOME */
	TC_LEFT,	/* 0x25 VK_LEFT */
	TC_UP,		/* 0x26 VK_UP */
	TC_RIGHT,	/* 0x27 VK_RIGHT */
	TC_DOWN,	/* 0x28 VK_DOWN */
	0,		/* 0x29 VK_SELECT */
	0,		/* 0x2A VK_PRINT */
	0,		/* 0x2B VK_EXECUTE */
	0,		/* 0x2C VK_SNAPSHOT */
	TC_INSERT,	/* 0x2D VK_INSERT */
	TC_DELETE,	/* 0x2E VK_DELETE */
	0,		/* 0x2F VK_HELP */
	'0',		/* 0x30 0 key */
	'1',		/* 0x31 1 key */
	'2',		/* 0x32 2 key */
	'3',		/* 0x33 3 key */
	'4',		/* 0x34 4 key */
	'5',		/* 0x35 5 key */
	'6',		/* 0x36 6 key */
	'7',		/* 0x37 7 key */
	'8',		/* 0x38 8 key */
	'9',		/* 0x39 9 key */
	0,		/* 0x3A-40 Undefined */
	0,		/* 0x3A-40 Undefined */
	0,		/* 0x3A-40 Undefined */
	0,		/* 0x3A-40 Undefined */
	0,		/* 0x3A-40 Undefined */
	0,		/* 0x3A-40 Undefined */
	0,		/* 0x3A-40 Undefined */
	'a',		/* 0x41 A key */
	'b',		/* 0x42 B key */
	'c',		/* 0x43 C key */
	'd',		/* 0x44 D key */
	'e',		/* 0x45 E key */
	'f',		/* 0x46 F key */
	'g',		/* 0x47 G key */
	'h',		/* 0x48 H key */
	'i',		/* 0x49 I key */
	'j',		/* 0x4A J key */
	'k',		/* 0x4B K key */
	'l',		/* 0x4C L key */
	'm',		/* 0x4D M key */
	'n',		/* 0x4E N key */
	'o',		/* 0x4F O key */
	'p',		/* 0x50 P key */
	'q',		/* 0x51 Q key */
	'r',		/* 0x52 R key */
	's',		/* 0x53 S key */
	't',		/* 0x54 T key */
	'u',		/* 0x55 U key */
	'v',		/* 0x56 V key */
	'w',		/* 0x57 W key */
	'x',		/* 0x58 X key */
	'y',		/* 0x59 Y key */
	'z',		/* 0x5A Z key */
	0,		/* 0x5B VK_LWIN */
	0,		/* 0x5C VK_RWIN */
	0,		/* 0x5D VK_APPS */
	0,		/* 0x5E Reserved */
	0,		/* 0x5F VK_SLEEP */
	0,		/* 0x60 VK_NUMPAD0 */
	0,		/* 0x61 VK_NUMPAD1 */
	0,		/* 0x62 VK_NUMPAD2 */
	0,		/* 0x63 VK_NUMPAD3 */
	0,		/* 0x64 VK_NUMPAD4 */
	0,		/* 0x65 VK_NUMPAD5 */
	0,		/* 0x66 VK_NUMPAD6 */
	0,		/* 0x67 VK_NUMPAD7 */
	0,		/* 0x68 VK_NUMPAD8 */
	0,		/* 0x69 VK_NUMPAD9 */
	0,		/* 0x6A VK_MULTIPLY */
	0,		/* 0x6B VK_ADD */
	0,		/* 0x6C VK_SEPARATOR */
	0,		/* 0x6D VK_SUBTRACT */
	0,		/* 0x6E VK_DECIMAL */
	0,		/* 0x6F VK_DIVIDE */
	TC_F1,		/* 0x70 VK_F1 */
	TC_F2,		/* 0x71 VK_F2 */
	TC_F3,		/* 0x72 VK_F3 */
	TC_F4,		/* 0x73 VK_F4 */
	TC_F5,		/* 0x74 VK_F5 */
	TC_F6,		/* 0x75 VK_F6 */
	TC_F7,		/* 0x76 VK_F7 */
	TC_F8,		/* 0x77 VK_F8 */
	TC_F9,		/* 0x78 VK_F9 */
	TC_F10,		/* 0x79 VK_F10 */
	TC_F11,		/* 0x7A VK_F11 */
	TC_F12,		/* 0x7B VK_F12 */
	0,		/* 0x7C VK_F13 */
	0,		/* 0x7D VK_F14 */
	0,		/* 0x7E VK_F15 */
	0,		/* 0x7F VK_F16 */
	0,		/* 0x80 VK_F17 */
	0,		/* 0x81 VK_F18 */
	0,		/* 0x82 VK_F19 */
	0,		/* 0x83 VK_F20 */
	0,		/* 0x84 VK_F21 */
	0,		/* 0x85 VK_F22 */
	0,		/* 0x86 VK_F23 */
	0,		/* 0x87 VK_F24 */
	0,		/* 0x88-8F Unassigned */
	0,		/* 0x88-8F Unassigned */
	0,		/* 0x88-8F Unassigned */
	0,		/* 0x88-8F Unassigned */
	0,		/* 0x88-8F Unassigned */
	0,		/* 0x88-8F Unassigned */
	0,		/* 0x88-8F Unassigned */
	0,		/* 0x88-8F Unassigned */
	0,		/* 0x90 VK_NUMLOCK */
	0,		/* 0x91 VK_SCROLL */
	0,		/* 0x92-96 OEM specific */
	0,		/* 0x92-96 OEM specific */
	0,		/* 0x92-96 OEM specific */
	0,		/* 0x92-96 OEM specific */
	0,		/* 0x92-96 OEM specific */
	0,		/* 0x97-9F Unassigned */
	0,		/* 0x97-9F Unassigned */
	0,		/* 0x97-9F Unassigned */
	0,		/* 0x97-9F Unassigned */
	0,		/* 0x97-9F Unassigned */
	0,		/* 0x97-9F Unassigned */
	0,		/* 0x97-9F Unassigned */
	0,		/* 0x97-9F Unassigned */
	0,		/* 0x97-9F Unassigned */
	0,		/* 0xA0 VK_LSHIFT */
	0,		/* 0xA1 VK_RSHIFT */
	0,		/* 0xA2 VK_LCONTROL */
	0,		/* 0xA3 VK_RCONTROL */
	0,		/* 0xA4 VK_LMENU */
	0,		/* 0xA5 VK_RMENU */
	0,		/* 0xA6 VK_BROWSER_BACK */
	0,		/* 0xA7 VK_BROWSER_FORWARD */
	0,		/* 0xA8 VK_BROWSER_REFRESH */
	0,		/* 0xA9 VK_BROWSER_STOP */
	0,		/* 0xAA VK_BROWSER_SEARCH */
	0,		/* 0xAB VK_BROWSER_FAVORITES */
	0,		/* 0xAC VK_BROWSER_HOME */
	0,		/* 0xAD VK_VOLUME_MUTE */
	0,		/* 0xAE VK_VOLUME_DOWN */
	0,		/* 0xAF VK_VOLUME_UP */
	0,		/* 0xB0 VK_MEDIA_NEXT_TRACK */
	0,		/* 0xB1 VK_MEDIA_PREV_TRACK */
	0,		/* 0xB2 VK_MEDIA_STOP */
	0,		/* 0xB3 VK_MEDIA_PLAY_PAUSE */
	0,		/* 0xB4 VK_LAUNCH_MAIL */
	0,		/* 0xB5 VK_LAUNCH_MEDIA_SELECT */
	0,		/* 0xB6 VK_LAUNCH_APP1 */
	0,		/* 0xB7 VK_LAUNCH_APP2 */
	0,		/* 0xB8-B9 Reserved */
	0,		/* 0xB8-B9 Reserved */
	';',		/* 0xBA VK_OEM_1';:' */
	'=',		/* 0xBB VK_OEM_PLUS */
	',',		/* 0xBC VK_OEM_COMMA */
	'-',		/* 0xBD VK_OEM_MINUS */
	'.',		/* 0xBE VK_OEM_PERIOD */
	'/',		/* 0xBF VK_OEM_2'/?' */
	'`',		/* 0xC0 VK_OEM_3'`~' */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xC1-D7 Reserved */
	0,		/* 0xD8-DA Unassigned */
	0,		/* 0xD8-DA Unassigned */
	0,		/* 0xD8-DA Unassigned */
	'[',		/* 0xDB VK_OEM_4'[{' */
	'\\',		/* 0xDC VK_OEM_5'\|' */
	']',		/* 0xDD VK_OEM_6']}' */
	'\'',		/* 0xDE VK_OEM_7''"' */
	0,		/* 0xDF VK_OEM_8 */
	0,		/* 0xE0 Reserved */
	0,		/* 0xE1 OEM specific */
	0,		/* 0xE2 VK_OEM_102 */
	0,		/* 0xE3-E4 OEM specific */
	0,		/* 0xE3-E4 OEM specific */
	0,		/* 0xE5 VK_PROCESSKEY */
	0,		/* 0xE6 OEM specific */
	0,		/* 0xE7 VK_PACKET */
	0,		/* 0xE8 Unassigned */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xE9-F5 OEM specific */
	0,		/* 0xF6 VK_ATTN */
	0,		/* 0xF7 VK_CRSEL */
	0,		/* 0xF8 VK_EXSEL */
	0,		/* 0xF9 VK_EREOF */
	0,		/* 0xFA VK_PLAY */
	0,		/* 0xFB VK_ZOOM */
	0,		/* 0xFC VK_NONAME */
	0,		/* 0xFD VK_PA1 */
	0,		/* 0xFE VK_OEM_CLEAR */
	0
};
