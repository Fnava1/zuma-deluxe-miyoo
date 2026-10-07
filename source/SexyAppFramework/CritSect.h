#ifndef _H_CritSect
#define _H_CritSect

#include "Common.h"
#if defined(_WIN32)
#include <windows.h>
#elif defined(_POSIX)
#include <pthread.h>
#endif

class CritSync;

namespace Sexy
{

class CritSect 
{
private:
	CRITICAL_SECTION mCriticalSection;
	friend class AutoCrit;

public:
	CritSect(void);
	~CritSect(void);
};

}

#endif // _H_CritSect
