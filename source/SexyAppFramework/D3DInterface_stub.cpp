#include "D3DInterface.h"

namespace Sexy {

std::string D3DInterface::mErrorString = "";

D3DInterface::D3DInterface() : mHWnd(0), mWidth(0), mHeight(0), mDD(0), mDDSDrawSurface(0), mZBuffer(0), mD3D(0), mD3DDevice(0), mSceneBegun(false), mIsWindowed(false) {}
D3DInterface::~D3DInterface() {}

void D3DInterface::Cleanup() {}
void D3DInterface::PushTransform(const SexyMatrix3 &theTransform, bool concatenate) {}
void D3DInterface::PopTransform() {}
bool D3DInterface::PreDraw() { return false; }
void D3DInterface::Flush() {}
void D3DInterface::RemoveMemoryImage(MemoryImage *theImage) {}
bool D3DInterface::CreateImageTexture(MemoryImage *theImage) { return false; }
bool D3DInterface::RecoverBits(MemoryImage* theImage) { return false; }
void D3DInterface::SetCurTexture(MemoryImage *theImage) {}
void D3DInterface::Blt(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter) {}
void D3DInterface::BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect *theClipRect, const Color& theColor, int theDrawMode) {}
void D3DInterface::BltMirror(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter) {}
void D3DInterface::StretchBlt(Image* theImage,  const Rect& theDestRect, const Rect& theSrcRect, const Rect* theClipRect, const Color &theColor, int theDrawMode, bool fastStretch, bool mirror) {}
void D3DInterface::BltRotated(Image* theImage, float theX, float theY, const Rect* theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY, const Rect& theSrcRect) {}
void D3DInterface::BltTransformed(Image* theImage, const Rect* theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX, float theY, bool center) {}
void D3DInterface::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode) {}
void D3DInterface::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode) {}
void D3DInterface::DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode) {}
void D3DInterface::DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode, Image *theTexture, bool blend) {}
void D3DInterface::DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend) {}
void D3DInterface::DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend) {}
void D3DInterface::FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty) {}
bool D3DInterface::InitFromDDInterface(DDInterface *theInterface) { return false; }
void D3DInterface::MakeDDPixelFormat(PixelFormat theFormatType, DDPIXELFORMAT* theFormat) {}
PixelFormat D3DInterface::GetDDPixelFormat(LPDDPIXELFORMAT theFormat) { return PixelFormat_Unknown; }
bool D3DInterface::CheckDXError(HRESULT theError, const char *theMsg) { return false; }

}
