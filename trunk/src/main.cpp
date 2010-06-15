#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PxAssert.h"

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

#include "FastXml.h"
#include "PxFileBuffer.h"

#pragma warning(disable:4996)

class TestFastXML : public FAST_XML::FastXml::Callback
{
public:
	TestFastXML(physx::PxFileBuf &stream)
	{
		mDepth = 0;
		FAST_XML::FastXml *xml = FAST_XML::createFastXml(this);
		PX_ASSERT(xml);
		if ( xml )
		{
			printf("Processing XML file once\r\n");
			xml->processXml(stream);
			xml->release();
		}
	}
	virtual bool processComment(const char *comment) 
	{
		indent();
		printf("XML Comment: \"%s\"\r\n", comment );
		return true;
	}

	virtual bool processClose(const char *element,physx::PxU32 depth) 
	{
#if 1
		indent();
		printf("CLOSE TAG: %s\r\n", element );
		mDepth--;
		PX_ASSERT(mDepth>=0);
#endif
		return depth == 0 ? false : true;
	}

	// return true to continue processing the XML document, false to skip.
	virtual bool processElement(
		const char *elementName,   // name of the element
		physx::PxI32 argc,                // number of attributes
		const char **argv,         // list of attributes.
		const char  *elementData,  // element data, null if none
		physx::PxI32 lineno)         // line number in the source XML file
	{
#if 1
		mDepth++;
		indent();
		printf("Element: %s with %d attributes.\r\n", elementName, argc );
		for (physx::PxI32 i=0; i<(argc/2); i++)
		{
			indent();
			printf("Attribute[%d] %s=%s\r\n", i+1, argv[i*2], argv[i*2+1] );
		}
		if ( elementData )
		{
			indent();
			char scratch[96];
			strncpy(scratch,elementData,95);
			scratch[95] = 0;
			printf("ElementData: %s\r\n", scratch );
		}
#endif
		return true;
	}

	void indent(void)
	{
		for (physx::PxI32 i=0; i<mDepth; i++)
		{
			printf("  ");
		}
	}

	virtual bool processXmlDeclaration(
		physx::PxI32 argc,
		const char **argv,
		const char  *elementData,
		physx::PxI32 lineno) 
	{
		return true;
	}

	virtual void *  fastxml_malloc(physx::PxU32 size) 
	{
		return ::malloc(size);
	}

	virtual void	fastxml_free(void *mem) 
	{
		::free(mem);
	}

	physx::PxI32	mDepth;
};

void main(int argc,const char **argv)
{
	if ( argc == 2 )
	{
		physx::PxFileBuffer fb(argv[1],physx::PxFileBuf::OPEN_READ_ONLY);
		if ( fb.getOpenMode() != physx::PxFileBuf::OPEN_FILE_NOT_FOUND )
		{
			TestFastXML t(fb);
		}
		else
		{
			printf("Input file '%s' not found.\r\n", argv[1] );
		}
	}
	else
	{
		printf("This is a demonstration application which shows how to use the FastXML parser.\r\n");
		printf("Usage: FastXML <filename>\r\n");
	}
}
