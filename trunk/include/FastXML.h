#ifndef FAST_XML_H

#define FAST_XML_H

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

#include "PxSimpleTypes.h"	 // defines basic data types; modify for your platform as needed.
#include "PxFileBuf.h"		// defines the basic file stream interface.

namespace FAST_XML
{

class FastXml
{
public:
	/***
	* Callbacks to the user with the contents of the XML file properly digested.
	*/
	class Callback
	{
	public:

		virtual bool processComment(const char *comment) = 0; // encountered a comment in the XML
		virtual bool processClose(const char *element) = 0;	  // process the 'close' indicator for a previously encountered element

		// return true to continue processing the XML document, false to skip.
		virtual bool processElement(
			const char *elementName,   // name of the element
			physx::PxI32 argc,                // number of attributes pairs
			const char **argv,         // list of attributes.
			const char  *elementData,  // element data, null if none
			physx::PxI32 lineno) = 0;         // line number in the source XML file

		// process the XML declaration header
		virtual bool processXmlDeclaration(
			physx::PxI32 argc,
			const char **argv,
			const char  *elementData,
			physx::PxI32 lineno) = 0;
	};

	virtual bool processXml(physx::PxFileBuf &buff, Callback *iface) = 0;

	virtual const char *getError(physx::PxI32 &lineno) = 0; // report the reason for a parsing error, and the line number where it occurred.

	virtual void release(void) = 0;
};

FastXml * createFastXml(void);

}; // end of namespace FAST_XML

#endif
