#ifndef PX_SIMPLE_TYPES_H

#define PX_SIMPLE_TYPES_H

/*!
**
** Copyright (c) 2010 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

namespace physx
{
	typedef signed __int64		PxI64;
	typedef signed int			PxI32;
	typedef signed short		PxI16;
	typedef signed char			PxI8;

	typedef unsigned __int64	PxU64;
	typedef unsigned int		PxU32;
	typedef unsigned short		PxU16;
	typedef unsigned char		PxU8;

	typedef float				PxF32;
	typedef double				PxF64;


};

#define PX_INLINE inline

#endif
