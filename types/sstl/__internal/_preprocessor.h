/*
Copyright © 2015 Kean Mariotti <kean.mariotti@gmail.com>
This work is free. You can redistribute it and/or modify it under the
terms of the Do What The Fuck You Want To Public License, Version 2,
as published by Sam Hocevar. See http://www.wtfpl.net/ for more details.
*/

#ifndef _SSTL_PREPROCESSOR__

#if _MSC_VER && !__INTEL_COMPILER
   /*
   MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
   MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
   MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
   MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
   MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
   MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
   MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
   MSVC++ 7.0  _MSC_VER == 1300
   MSVC++ 6.0  _MSC_VER == 1200
   MSVC++ 5.0  _MSC_VER == 1100
   */
   #define _is_msvc() 1
#else
   #define _is_msvc() 0
#endif

#if defined(__GNUC__)
   #define _sstl_is_gcc() 1
#else
   #define _sstl_is_gcc() 0
#endif

#endif // _SSTL_PREPROCESSOR__
