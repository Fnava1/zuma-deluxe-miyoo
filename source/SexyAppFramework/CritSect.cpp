#pragma warning(disable:4786)

#include "CritSect.h"
#include <cassert>

using namespace Sexy;

////////////////////////////////////////////////////////////////////////////////

CritSect::CritSect(void)
{
	InitializeCriticalSection(&mCriticalSection);
}

////////////////////////////////////////////////////////////////////////////////

CritSect::~CritSect(void)
{
	DeleteCriticalSection(&mCriticalSection);
}
