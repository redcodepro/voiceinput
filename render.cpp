#include "main.h"

hookedPresent_t	orig_Present;
hookedReset_t	orig_Reset;

IDirect3DTexture9* pTexture[3];

#define ID_MICRO 0
#define ID_BACK  1
#define ID_ROUND 2

CD3DRender *render = new CD3DRender(128);

HRESULT CALLBACK hooked_Present(IDirect3DDevice9* pDevice, const RECT* r1, const RECT* r2, HWND hwnd, const RGNDATA* rngd)
{
	static DWORD prev_ticks = 0;
	float frame_time = (float)(GetTickCount() - prev_ticks) / 1000.0f;
	prev_ticks = GetTickCount();

	static bool init = false;
	if (!init)
	{
		render->Initialize(pDevice);
		renderInitResources(pDevice);

		init = true;
	}

	if (!G.isMenuOpened && ps.dwPluginState != STATE_NOTAVAIL)
	{
		static bool move = false;
		static int offset[2] = {0, 0};

		POINT &curPos = keyhook_get_mouse_position();

		if (move)
		{
			if (KEY_DOWN(VK_LBUTTON))
			{
				ps.pos_x = curPos.x - offset[0];
				ps.pos_y = curPos.y - offset[1];

				if (ps.pos_x < 0) ps.pos_x = 0;
				if (ps.pos_y < 0) ps.pos_y = 0;
				if (ps.pos_x + 32 > *(int*)0xC9C040) ps.pos_x = ((*(int*)0xC9C040) - 32);
				if (ps.pos_y + 32 > *(int*)0xC9C044) ps.pos_y = ((*(int*)0xC9C044) - 32);
			}
			else move = false;
		}
		else
		{
			if (SAMP::pInputBox->m_bEnabled && KEY_PRESSED(VK_LBUTTON) && MOUSE_HOVERED(ps.pos_x, ps.pos_y, 32, 32))
			{
				offset[0] = curPos.x - ps.pos_x;
				offset[1] = curPos.y - ps.pos_y;
				move = true;
			}
		}
		
		if (SUCCEEDED(render->BeginRender()))
		{
			static float requesting_alpha = 0.56f;
			static float microphone_alpha = 0.56f;
			static bool  alpha_orient = false;

			switch (ps.dwPluginState)
			{
			case STATE_ALREADY:
				{
					if (requesting_alpha > 0.56f)
					{
						requesting_alpha -= 3.0f * frame_time;
						if (requesting_alpha < 0.56f)
							requesting_alpha = 0.56f;
					}

					if (microphone_alpha > 0.56f)
					{
						microphone_alpha -= 3.0f * frame_time;
						if (microphone_alpha > 0.56f)
							microphone_alpha = 0.56f;
					}
				}
				break;
			case STATE_RECORDING:
				{
					if (requesting_alpha < 1.0f)
					{
						requesting_alpha += 3.0f * frame_time;
						if (requesting_alpha > 1.0f)
							requesting_alpha = 1.0f;
					}

					if (microphone_alpha < 1.0f)
					{
						microphone_alpha += 3.0f * frame_time;
						if (microphone_alpha > 1.0f)
							microphone_alpha = 1.0f;
					}
				}
				break;
			case STATE_REQUESTING:
				{
					if (microphone_alpha > 0.56f)
					{
						microphone_alpha -= 3.0f * frame_time;
						if (microphone_alpha > 0.56f)
							microphone_alpha = 0.56f;
					}

					float pulse_step = 2.1f * frame_time;

					if (requesting_alpha + pulse_step >= 1.0f)
						alpha_orient = false;
					if (requesting_alpha - pulse_step <= 0.5f)
						alpha_orient = true;

					requesting_alpha += alpha_orient ? pulse_step : -pulse_step;
				}
				break;
			}

			render->DrawTexture(ps.pos_x, ps.pos_y, 32, 32, pTexture[ID_BACK]);
			render->DrawTexture(ps.pos_x, ps.pos_y, 32, 32, pTexture[ID_MICRO], D3DCOLOR_ARGB((byte)(microphone_alpha * 255), 255, 255, 255));
			render->DrawTexture(ps.pos_x, ps.pos_y, 32, 32, pTexture[ID_ROUND], D3DCOLOR_ARGB((byte)(requesting_alpha * 255), 255, 255, 255));

			render->EndRender();
		}
	}
	return orig_Present(pDevice, r1, r2, hwnd, rngd);
}

HRESULT CALLBACK hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	render->Invalidate();
	for (int i = 0; i < _countof(pTexture); ++i)
		SAFE_RELEASE(pTexture[i]);

	HRESULT hr = orig_Reset(pDevice, pPresentationParameters);
	if (SUCCEEDED(hr))
	{
		render->Initialize(pDevice);
		renderInitResources(pDevice);
	}
	return hr;
}

void renderInit()
{
	MH_CreateHook(VT_FUNC(G.pD3DDevice, 17), &hooked_Present, reinterpret_cast<void**>(&orig_Present));
	MH_EnableHook(VT_FUNC(G.pD3DDevice, 17));

	MH_CreateHook(VT_FUNC(G.pD3DDevice, 16), &hooked_Reset, reinterpret_cast<void**>(&orig_Reset));
	MH_EnableHook(VT_FUNC(G.pD3DDevice, 16));
}

void renderFree()
{
	MH_RemoveHook(VT_FUNC(G.pD3DDevice, 17));
	MH_RemoveHook(VT_FUNC(G.pD3DDevice, 16));

	for (int i = 0; i < _countof(pTexture); ++i)
		SAFE_RELEASE(pTexture[i]);

	delete render;
}

void renderInitResources(IDirect3DDevice9* pDevice)
{
	load_texture_from_resource(pDevice, &pTexture[ID_MICRO], MAKEINTRESOURCEA(IDB_PNG1), 32, 32);
	load_texture_from_resource(pDevice, &pTexture[ID_BACK], MAKEINTRESOURCEA(IDB_PNG2), 32, 32);
	load_texture_from_resource(pDevice, &pTexture[ID_ROUND], MAKEINTRESOURCEA(IDB_PNG3), 32, 32);
}

HRESULT load_texture_from_resource(LPDIRECT3DDEVICE9 device, LPDIRECT3DTEXTURE9 *texture, LPSTR IDB_ID, UINT Width, UINT Height)
{
	HGLOBAL resource;
	void *buffer;
	DWORD size;

	HRSRC resinfo = FindResourceA(G.selfModule, IDB_ID, "PNG");

	if (resinfo)
	{
		size = SizeofResource(G.selfModule, resinfo);
		if (size == 0) return HRESULT_FROM_WIN32(GetLastError());

		resource = LoadResource(G.selfModule, resinfo);
		if (!resource) return HRESULT_FROM_WIN32(GetLastError());

		buffer = LockResource(resource);
		if (buffer == NULL) return HRESULT_FROM_WIN32(GetLastError());

		return D3DXCreateTextureFromFileInMemoryEx(device, buffer, size, Width, Height,
			D3DX_DEFAULT, // MIP LEVELS
			NULL, // USAGE
			D3DFMT_UNKNOWN, // FORMAT
			D3DPOOL_MANAGED, // POOL
			D3DX_DEFAULT, // FILTER (D3DX_DEFAULT)
			D3DX_DEFAULT, // MIP FILTER (D3DX_DEFAULT)
			0x00000000, // COLOR KEY
			NULL, // D3DXIMAGE_INFO
			NULL, // pPallete
			texture
		);
	}
	return HRESULT_FROM_WIN32(GetLastError());
}