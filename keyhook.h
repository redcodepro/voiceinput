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

struct key_state
{
	BYTE count : 5;
	BYTE flip : 1;
	BYTE pstate : 1;	/* previous state (1 down, 0 up) */
	BYTE consume : 1;	/* KEY_CONSUME(vkey) */
};

#define KEY_DOWN		keyhook_key_down
#define KEY_UP			keyhook_key_up
#define KEY_PRESSED		keyhook_key_pressed
#define KEY_RELEASED	keyhook_key_released
#define KEY_CONSUME		keyhook_key_consume
#define MOUSE_HOVERED	keyhook_mouse_hovered

LRESULT CALLBACK		wnd_proc(HWND wnd, UINT umsg, WPARAM wparam, LPARAM lparam);

void					keyhook_run();

int						keyhook_key_down ( int v );
int						keyhook_key_up ( int v );
int						keyhook_key_pressed ( int v );
int						keyhook_key_released ( int v );
void					keyhook_key_consume ( int v );
void					keyhook_clear_states();
bool					keyhook_mouse_hovered(int x, int y, int w, int h);
POINT					&keyhook_get_mouse_position();
int						key_lookup(const char* name);
std::string				key_delookup(int key);

extern struct key_state key_table[256];