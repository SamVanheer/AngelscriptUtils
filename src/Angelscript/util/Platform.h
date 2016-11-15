#ifndef ANGELSCRIPT_UTIL_PLATFORM_H
#define ANGELSCRIPT_UTIL_PLATFORM_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if _MSC_VER < 1900
#define snprintf _snprintf
#endif

#define mkdir _mkdir

#ifndef ALLOW_VOID_DEF
#undef VOID
#endif

#else
#include <linux/limits.h>

#define MAX_PATH PATH_MAX
#endif

#endif //ANGELSCRIPT_UTIL_PLATFORM_H
