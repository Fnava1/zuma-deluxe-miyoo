#ifndef __D3D_H__
#define __D3D_H__
#include "WindowsCompat.h"
#include "ddraw.h"

typedef struct _D3DVIEWPORT7 {
    DWORD dwX;
    DWORD dwY;
    DWORD dwWidth;
    DWORD dwHeight;
    float dvMinZ;
    float dvMaxZ;
} D3DVIEWPORT7, *LPD3DVIEWPORT7;

#endif
