#ifndef __DDRAW_INCLUDED__
#define __DDRAW_INCLUDED__

#include "WindowsCompat.h"

struct IDirectDrawSurface;
struct IDirectDrawSurface7;
struct IDirectDraw;
struct IDirectDraw7;
struct IDirectDrawPalette;
struct IDirect3D7;
typedef struct IDirect3D7* LPDIRECT3D7;
struct IDirect3DDevice7;
typedef struct IDirect3DDevice7* LPDIRECT3DDEVICE7;
typedef struct IDirectDrawPalette* LPDIRECTDRAWPALETTE;

typedef struct IDirectDraw* LPDIRECTDRAW;
typedef struct IDirectDraw7* LPDIRECTDRAW7;
typedef struct IDirectDrawSurface* LPDIRECTDRAWSURFACE;
typedef struct IDirectDrawSurface7* LPDIRECTDRAWSURFACE7;

typedef struct _DDCOLORKEY {
    DWORD dwColorSpaceLowValue;
    DWORD dwColorSpaceHighValue;
} DDCOLORKEY;

typedef struct _DDPIXELFORMAT {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    union {
        DWORD dwRGBBitCount;
        DWORD dwYUVBitCount;
        DWORD dwZBufferBitDepth;
        DWORD dwAlphaBitDepth;
    };
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    union {
        DWORD dwRGBAlphaBitMask;
        DWORD dwYUVAlphaBitMask;
    };
} DDPIXELFORMAT, *LPDDPIXELFORMAT;

typedef struct _DDSCAPS2 {
    DWORD dwCaps;
    DWORD dwCaps2;
    DWORD dwCaps3;
    DWORD dwCaps4;
} DDSCAPS2;

typedef struct _DDSURFACEDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    union {
        LONG lPitch;
        DWORD dwLinearSize;
    };
    DWORD dwBackBufferCount;
    union {
        DWORD dwMipMapCount;
        DWORD dwZBufferBitDepth;
        DWORD dwRefreshRate;
    };
    DWORD dwAlphaBitDepth;
    DWORD dwReserved;
    void* lpSurface;
    union {
        DDCOLORKEY ddckCKDestOverlay;
        DWORD dwEmptyFaceColor;
    };
    DDCOLORKEY ddckCKDestBlt;
    DDCOLORKEY ddckCKSrcOverlay;
    DDCOLORKEY ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS2 ddsCaps;
} DDSURFACEDESC, DDSURFACEDESC2;

typedef struct _DDBLTFX {
    DWORD dwSize;
    DWORD dwDDFX;
    DWORD dwROP;
    DWORD dwDDROP;
    DWORD dwRotationAngle;
    DWORD dwZBufferOpCode;
    DWORD dwZBufferLow;
    DWORD dwZBufferHigh;
    DWORD dwZBufferBaseDest;
    DWORD dwZFilterFlags;
    DWORD dwAlphaEdgeBlendBitDepth;
    DWORD dwAlphaEdgeBlend;
    DWORD dwReserved;
    DWORD dwAlphaDestConstBitDepth;
    union {
        DWORD dwAlphaDestConst;
        void* lpDDSAlphaDestConst;
    };
    DWORD dwAlphaSrcConstBitDepth;
    union {
        DWORD dwAlphaSrcConst;
        void* lpDDSSrcConst;
    };
    union {
        DWORD dwFillColor;
        DWORD dwFillDepth;
        DWORD dwFillPixel;
        void* lpDDSPattern;
    };
    DDCOLORKEY ddckDestColorkey;
    DDCOLORKEY ddckSrcColorkey;
} DDBLTFX;

struct IDirectDrawSurface {
    virtual HRESULT Release() { return S_OK; }
    virtual HRESULT AddRef() { return S_OK; }
    virtual HRESULT Blt(LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, DDBLTFX*) { return S_OK; }
    virtual HRESULT Lock(LPRECT, DDSURFACEDESC*, DWORD, HANDLE) { return S_OK; }
    virtual HRESULT Unlock(void*) { return S_OK; }
    virtual HRESULT Flip(LPDIRECTDRAWSURFACE, DWORD) { return S_OK; }
    virtual HRESULT SetColorKey(DWORD, DDCOLORKEY*) { return S_OK; }
    virtual HRESULT GetSurfaceDesc(DDSURFACEDESC*) { return S_OK; }
};

struct IDirectDrawSurface7 : public IDirectDrawSurface {
};

struct IDirectDraw {
    virtual HRESULT Release() { return S_OK; }
    virtual HRESULT AddRef() { return S_OK; }
    virtual HRESULT FlipToGDISurface() { return S_OK; }
    virtual HRESULT RestoreDisplayMode() { return S_OK; }
};

struct IDirectDraw7 : public IDirectDraw {
    virtual HRESULT GetAvailableVidMem(DDSCAPS2*, DWORD*, DWORD*) { return S_OK; }
};

#define DD_OK                   0
#define DDERR_SURFACELOST       0x887601C2
#define DDERR_WASSTILLDRAWING   0x8876021C
#define DDBLT_WAIT              0x01000000
#define DDBLT_KEYSRC            0x00008000
#define DDBLT_COLORFILL         0x00000400
#define DDLOCK_WAIT             0x00000001
#define DDLOCK_SURFACEMEMORYPTR 0x00000000
#define DDFLIP_WAIT             0x00000001
#define DDCKEY_SRCCLOBRG        0x00000001
#define DDCKEY_SRCBLT           0x00000008


#define DDSD_CAPS               0x00000001
#define DDSD_HEIGHT             0x00000002
#define DDSD_WIDTH              0x00000004
#define DDSD_PITCH              0x00000008
#define DDSD_PIXELFORMAT        0x00001000

#define DDSCAPS_SYSTEMMEMORY    0x00000800
#define DDSCAPS_VIDEOMEMORY     0x00004000

#endif
