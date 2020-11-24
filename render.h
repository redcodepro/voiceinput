#pragma once

#include "main.h"

typedef HRESULT(__stdcall* hookedPresent_t)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
typedef HRESULT(__stdcall* hookedReset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

void renderInit();
void renderFree();
void renderInitResources(IDirect3DDevice9* pDevice);
HRESULT load_texture_from_resource(LPDIRECT3DDEVICE9 device, LPDIRECT3DTEXTURE9 *texture, LPSTR IDB_ID, UINT Width = D3DX_DEFAULT, UINT Height = D3DX_DEFAULT);
