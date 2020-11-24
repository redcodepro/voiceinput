/*

	PROJECT:		mod_sa
	LICENSE:		See LICENSE in the top level directory
	COPYRIGHT:		Copyright we_sux, BlastHack

	mod_sa is available from https://github.com/BlastHackNet/mod_s0beit_sa/

	mod_sa is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	mod_sa is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with mod_sa.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "main.h"

struct key_state	key_table[256];
int					keys_cleared;
POINT				mouse_position;

static void process_key(int down, int vkey)
{
	if ( down && KEY_DOWN(vkey) )
		return; /* ignore key repeat, bad states */
	if ( !down && KEY_UP(vkey) )
		return; /* ignore bad states */

	key_table[vkey].count++;
}

LRESULT CALLBACK wnd_proc(HWND wnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	if (!G.isMenuOpened)
	{
		switch (umsg)
		{
		case WM_KILLFOCUS:
			keys_cleared = 0;
			keyhook_clear_states();
			break;

		/* :D */
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			unsigned long	p = (unsigned long)lparam;
			int				down = (umsg == WM_KEYDOWN || umsg == WM_SYSKEYDOWN);
			int				vkey = (int)wparam;

			unsigned int	scancode = (p >> 16) & 0x00FF;
			unsigned int	extended = (p >> 24) & 0x0001;

			/* :D :D :D :D :D */
			switch (vkey)
			{
			case VK_SHIFT:
				if (scancode == MapVirtualKey(VK_LSHIFT, 0))
					vkey = VK_LSHIFT;
				else if (scancode == MapVirtualKey(VK_RSHIFT, 0))
					vkey = VK_RSHIFT;
				break;

			case VK_CONTROL:
				vkey = extended ? VK_RCONTROL : VK_LCONTROL;
				break;
			case VK_MENU:
				vkey = extended ? VK_RMENU : VK_LMENU;
				break;
			}

			/* :D */
			if (KEY_DOWN(VK_LMENU) && vkey == VK_LMENU && down)
				break;
			if (KEY_UP(VK_LMENU) && vkey == VK_LMENU && !down)
				break;

			process_key(down, vkey);
		}
		break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			process_key((umsg == WM_LBUTTONDOWN), VK_LBUTTON);
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			process_key((umsg == WM_RBUTTONDOWN), VK_RBUTTON);
			break;

		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			process_key((umsg == WM_MBUTTONDOWN), VK_MBUTTON);
			break;

		case WM_MOUSEMOVE:
			mouse_position.x = ((int)(short)LOWORD(lparam));
			mouse_position.y = ((int)(short)HIWORD(lparam));
			break;
		}
	}
	return CallWindowProc(G.orig_wndproc, wnd, umsg, wparam, lparam);
}

void keyhook_run ( void )
{
	keys_cleared = 0;
	for ( int i = 0; i < 256; i++ )
	{
		key_table[i].consume = 0;

		if ( i == VK_PRIOR || i == VK_NEXT || i == VK_TAB )
			key_table[i].pstate = ( key_table[i].count & 1 );
		else
			key_table[i].pstate = KEY_DOWN( i );

		if ( key_table[i].count > 0 )
		{
			key_table[i].flip ^= 1;
			key_table[i].count--;
		}
	}
}

int keyhook_key_down ( int v )
{
	if ( key_table[v].consume )
		return 0;
	else
		return ( key_table[v].count & 1 ) ^ key_table[v].flip;
}

int keyhook_key_up ( int v )
{
	return !KEY_DOWN( v );
}

int keyhook_key_pressed ( int v )
{
	if ( key_table[v].consume )
		return 0;
	else if ( v == VK_PRIOR || v == VK_NEXT || v == VK_TAB )
		return KEY_DOWN( v ) && !( key_table[v].pstate ^ key_table[v].flip );
	else
		return KEY_DOWN( v ) && !key_table[v].pstate;
}

int keyhook_key_released ( int v )
{
	return KEY_UP( v ) && key_table[v].pstate;
}

void keyhook_key_consume ( int v )
{
	key_table[v].consume = 1;
}

void keyhook_clear_states ( void )
{
	if ( !keys_cleared )
	{
		keys_cleared = 1;
		for ( int i = 0; i < 256; i++ )
		{
			key_table[i].pstate = 0;
			key_table[i].count = 0;
			key_table[i].flip = 0;
		}
	}
}

bool keyhook_mouse_hovered(int x, int y, int w, int h)
{
	if (mouse_position.x > x && mouse_position.x < x + w && mouse_position.y > y && mouse_position.y < y + h)
		return true;
	return false;
}

POINT &keyhook_get_mouse_position()
{
	return mouse_position;
}

struct key_alias
{
	int		key;
	char	*name;
};

static struct key_alias key_alias[] =
{
	{ VK_LBUTTON, "lbutton" },
	{ VK_RBUTTON, "rbutton" },
	{ VK_MBUTTON, "mbutton" },
	{ VK_XBUTTON1, "mouse_4" },
	{ VK_XBUTTON2, "mouse_5" },
	{ VK_BACK, "backspace" },
	{ VK_TAB, "tab" },
	{ VK_RETURN, "return" },
	{ VK_LSHIFT, "lshift" },
	{ VK_RSHIFT, "rshift" },
	{ VK_LCONTROL, "lctrl" },
	{ VK_RCONTROL, "rctrl" },
	{ VK_LMENU, "lalt" },
	{ VK_RMENU, "ralt" },
	{ VK_SPACE, "space" },
	{ VK_PRIOR, "pageup" },
	{ VK_NEXT, "pagedn" },
	{ VK_END, "end" },
	{ VK_HOME, "home" },
	{ VK_LEFT, "left" },
	{ VK_UP, "up" },
	{ VK_RIGHT, "right" },
	{ VK_DOWN, "down" },
	{ VK_INSERT, "insert" },
	{ VK_DELETE, "delete" },
	{ VK_PAUSE, "pause" },
	{ VK_NUMPAD0, "np0" },
	{ VK_NUMPAD1, "np1" },
	{ VK_NUMPAD2, "np2" },
	{ VK_NUMPAD3, "np3" },
	{ VK_NUMPAD4, "np4" },
	{ VK_NUMPAD5, "np5" },
	{ VK_NUMPAD6, "np6" },
	{ VK_NUMPAD7, "np7" },
	{ VK_NUMPAD8, "np8" },
	{ VK_NUMPAD9, "np9" },
	{ VK_MULTIPLY, "multiply" },
	{ VK_ADD, "add" },
	{ VK_SEPARATOR, "separator" },
	{ VK_SUBTRACT, "subtract" },
	{ VK_DECIMAL, "decimal" },
	{ VK_DIVIDE, "divide" },
	{ VK_F1, "f1" },
	{ VK_F2, "f2" },
	{ VK_F3, "f3" },
	{ VK_F4, "f4" },
	{ VK_F5, "f5" },
	{ VK_F6, "f6" },
	{ VK_F7, "f7" },
	{ VK_F8, "f8" },
	{ VK_F9, "f9" },
	{ VK_F10, "f10" },
	{ VK_F11, "f11" },
	{ VK_F12, "f12" },
	{ VK_F13, "f13" },
	{ VK_F14, "f14" },
	{ VK_F15, "f15" },
	{ VK_F16, "f16" },
	{ VK_F17, "f17" },
	{ VK_F18, "f18" },
	{ VK_F19, "f19" },
	{ VK_F20, "f20" },
	{ VK_F21, "f21" },
	{ VK_F22, "f22" },
	{ VK_F23, "f23" },
	{ VK_F24, "f24" },
	{ VK_OEM_PLUS, "oem_plus" },
	{ VK_OEM_COMMA, "oem_comma" },
	{ VK_OEM_MINUS, "oem_minus" },
	{ VK_OEM_PERIOD, "oem_period" },
	{ VK_OEM_1, "oem_1" },
	{ VK_OEM_2, "oem_2" },
	{ VK_OEM_3, "oem_3" },
	{ VK_OEM_4, "oem_4" },
	{ VK_OEM_5, "oem_5" },
	{ VK_OEM_6, "oem_6" },
	{ VK_OEM_7, "oem_7" },
	{ VK_OEM_8, "oem_8" }
};

int key_lookup(const char* name)
{
	if (name[0] && !name[1])
	{
		if (name[0] >= '0' && name[0] <= '9')
			return name[0];

		if (name[0] >= 'A' && name[0] <= 'Z')
			return name[0];

		if (name[0] >= 'a' && name[0] <= 'z')
			return 'A' + (name[0] - 'a');
	}

	for (int i = 0; i < _countof(key_alias); ++i)
	{
		if (!strcmp(key_alias[i].name, name))
		{
			return key_alias[i].key;
		}
	}

	return 0;
}

std::string key_delookup(int key)
{
	if (key >= 'A' && key <= 'Z')
		return std::string(1, key);

	if (key >= '0' && key <= '9')
		return std::string(1, key);

	for (int i = 0; i < _countof(key_alias); ++i)
	{
		if (key_alias[i].key == key)
			return std::string(key_alias[i].name);
	}
	return std::string();
}
