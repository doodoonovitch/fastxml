#include "FastXml.h"
#include "PxFileBuf.h"
#include "PsUserAllocated.h"
#include <string.h>
#include <malloc.h>

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

namespace FAST_XML
{

class MyFastXml : public FastXml, public physx::UserAllocated
{
public:
	enum CharType
	{
		CT_DATA,
		CT_EOF,
		CT_SOFT,
		CT_END_OF_ELEMENT, // either a forward slash or a greater than symbol
		CT_END_OF_LINE,
	};

	MyFastXml()
	{
		memset(mTypes, CT_DATA, sizeof(mTypes));
		mTypes[0] = CT_EOF;
		mTypes[' '] = mTypes['\t'] = CT_SOFT;
		mTypes['/'] = mTypes['>'] = mTypes['?'] = CT_END_OF_ELEMENT;
		mTypes['\n'] = mTypes['\r'] = CT_END_OF_LINE;
		mError = 0;
		mStackIndex = 0;
		mFileBuf = NULL;
		mReadBuffer = NULL;
		mReadBufferSize = 1024*1024; // process data one megabyte at a time by default...
	}

	virtual ~MyFastXml(void)
	{
		releaseMemory();
	}

	char *processClose(char c, const char *element, char *scan, physx::PxI32 argc, const char **argv, FastXml::Callback *iface)
	{
		if ( c == '/' || c == '?' )
		{
			char *slash = (char *)strchr(element, c);
			if( slash )
				*slash = 0;

			if( c == '?' && strcmp(element, "xml") == 0 )
			{
				if( !iface->processXmlDeclaration(argc/2, argv, 0, mLineNo) )
					return 0;
			}
			else
			{
				if ( !iface->processElement(element, argc, argv, 0, mLineNo) )
				{
					mError = "User aborted the parsing process";
					return 0;
				}

				pushElement(element);

				const char *close = popElement();
				if( !iface->processClose(close) )
				{
					mError = "User aborted the parsing process";
					return 0;
				}
			}

			if ( !slash )
				++scan;
		}
		else
		{
			scan = skipNextData(scan);
			char *data = scan; // this is the data portion of the element, only copies memory if we encounter line feeds
			char *dest_data = 0;
			while ( *scan && *scan != '<' )
			{
				if ( mTypes[*scan] == CT_END_OF_LINE )
				{
					if ( *scan == '\r' ) mLineNo++;
					dest_data = scan;
					*dest_data++ = ' '; // replace the linefeed with a space...
					scan = skipNextData(scan);
					while ( *scan && *scan != '<' )
					{
						if ( mTypes[*scan] == CT_END_OF_LINE )
						{
							if ( *scan == '\r' ) mLineNo++;
							*dest_data++ = ' '; // replace the linefeed with a space...
							scan = skipNextData(scan);
						}
						else
						{
							*dest_data++ = *scan++;
						}
					}
					break;
				}
				else
					++scan;
			}

			if ( *scan == '<' )
			{
				if ( dest_data )
				{
					*dest_data = 0;
				}
				else
				{
					*scan = 0;
				}

				scan++; // skip it..

				if ( *data == 0 ) data = 0;

				if ( !iface->processElement(element, argc/2, argv, data, mLineNo) )
				{
					mError = "User aborted the parsing process";
					return 0;
				}

				pushElement(element);

				// check for the comment use case...
				if ( scan[0] == '!' && scan[1] == '-' && scan[2] == '-' )
				{
					scan+=3;
					while ( *scan && *scan == ' ' )
						++scan;

					char *comment = scan;
					char *comment_end = strstr(scan, "-->");
					if ( comment_end )
					{
						*comment_end = 0;
						scan = comment_end+3;
						if( !iface->processComment(comment) )
						{
							mError = "User aborted the parsing process";
							return 0;
						}
					}
				}
				else if ( *scan == '/' )
				{
					scan = processClose(scan, iface);
					if( !scan ) return 0;
				}
			}
			else
			{
				mError = "Data portion of an element wasn't terminated properly";
				return 0;
			}
		}

		return scan;
	}

	char *processClose(char *scan, FastXml::Callback *iface)
	{
		const char *start = popElement(), *close = start;
		if( scan[1] != '>')
		{
			scan++;
			close = scan;
			while ( *scan && *scan != '>' ) scan++;
			*scan = 0;
		}

		if( 0 != strcmp(start, close) )
		{
			mError = "Open and closing tags do not match";
			return 0;
		}

		if( !iface->processClose(close) )
		{
			mError = "User aborted the parsing process";
			return 0;
		}

		return ++scan;
	}

	virtual bool processXml(physx::PxFileBuf &fileBuf, FastXml::Callback *iface)
	{
		releaseMemory();
		mFileBuf = &fileBuf;
		return processXml(iface);
	}

	// if we have finished processing the data we had pending..
	char * readData(void)
	{
		char *ret = NULL;

		if ( mReadBuffer == NULL )
		{
			mReadBuffer = (char *)::malloc(mReadBufferSize+1);
		}
		physx::PxU32 readCount = mFileBuf->read(mReadBuffer,mReadBufferSize);
		if ( readCount > 0 )
		{
			ret = mReadBuffer;
			mReadBuffer[readCount] = 0; // end of string terminator...
		}
		return ret;
	}

	bool processXml(FastXml::Callback *iface)
	{
		bool ret = true;

		const int MAX_ATTRIBUTE = 2048; // can't imagine having more than 2,048 attributes in a single element right?

		mLineNo = 1;

		char *element, *scan = readData();

		while( *scan )
		{
			scan = skipNextData(scan);
			if( *scan == 0 ) break;
			if( *scan == '<' )
			{
				scan++;
				if( *scan == '?' ) //Allow xml declarations
				{
					scan++;
				}
				else if ( scan[0] == '!' && scan[1] == '-' && scan[2] == '-' )
				{
					scan+=3;
					while ( *scan && *scan == ' ' )
						scan++;
					char *comment = scan, *comment_end = strstr(scan, "-->");
					if ( comment_end )
					{
						*comment_end = 0;
						scan = comment_end+3;
						if( !iface->processComment(comment) )
						{
							mError = "User aborted the parsing process";
							return false;
						}
					}
					continue;
				}
			}

			if( *scan == '/' )
			{
				scan = processClose(scan, iface);
				if( !scan )
					return false;
			}
			else
			{
				if( *scan == '?' )
					scan++;
				element = scan;
				physx::PxI32 argc = 0;
				const char *argv[MAX_ATTRIBUTE];
				bool close;
				scan = nextSoftOrClose(scan, close);
				if( close )
				{
					char c = *(scan-1);
					if ( c != '?' && c != '/' )
					{
						c = '>';
					}
					*scan++ = 0;
					scan = processClose(c, element, scan, argc, argv, iface);
					if ( !scan )
						return false;
				}
				else
				{
					if ( *scan == 0 ) return ret;
					*scan = 0; // place a zero byte to indicate the end of the element name...
					scan++;

					while ( *scan )
					{
						scan = skipNextData(scan); // advance past any soft seperators (tab or space)

						if ( mTypes[*scan] == CT_END_OF_ELEMENT )
						{
							char c = *scan++;
							if( '?' == c )
							{
								if( '>' != *scan ) //?>
									return false;

								scan++;
							}
							scan = processClose(c, element, scan, argc, argv, iface);
							if ( !scan ) 
								return false;
							break;
						}
						else
						{
							if( argc >= MAX_ATTRIBUTE )
							{
								mError = "encountered too many attributes";
								return false;
							}
							argv[argc] = scan;
							scan = nextSep(scan);  // scan up to a space, or an equal
							if( *scan )
							{
								if( *scan != '=' )
								{
									*scan = 0;
									scan++;
									while ( *scan && *scan != '=' ) scan++;
									if ( *scan == '=' ) scan++;
								}
								else
								{
									*scan=0;
									scan++;
								}

								if( *scan ) // if not eof...
								{
									scan = skipNextData(scan);
									if( *scan == '"' )
									{
										scan++;
										argc++;
										argv[argc] = scan;
										argc++;
										while ( *scan && *scan != 34 ) scan++;
										if( *scan == '"' )
										{
											*scan = 0;
											scan++;
										}
										else
										{
											mError = "Failed to find closing quote for attribute";
											return false;
										}
									}
									else
									{
										//mError = "Expected quote to begin attribute";
										//return false;
										// PH: let's try to have a more graceful fallback
										argc--;
										while(*scan != '/' && *scan != '>' && *scan != 0)
											scan++;
									}
								}
							} //if( *scan )
						} //if ( mTypes[*scan]
					} //if( close )
				} //if( *scan == '/'
			} //while( *scan )
		}

		if( mStackIndex )
		{
			mError = "Invalid file format";
			return false;
		}

		return ret;
	}

	const char *getError(physx::PxI32 &lineno)
	{
		const char *ret = mError;
		lineno = mLineNo;
		mError = 0;
		return ret;
	}

	virtual void release(void)
	{
		delete this;
	}

private:
	PX_INLINE void releaseMemory(void)
	{
		mError = 0;
		mFileBuf = NULL;
		::free(mReadBuffer);
		mReadBuffer = NULL;
	}

	PX_INLINE char *nextSoft(char *scan)
	{
		while ( *scan && mTypes[*scan] != CT_SOFT ) scan++;
		return scan;
	}

	PX_INLINE char *nextSoftOrClose(char *scan, bool &close)
	{
		while ( *scan && mTypes[*scan] != CT_SOFT && *scan != '>' ) scan++;
		close = *scan == '>';
		return scan;
	}

	PX_INLINE char *nextSep(char *scan)
	{
		while ( *scan && mTypes[*scan] != CT_SOFT && *scan != '=' ) scan++;
		return scan;
	}

	PX_INLINE char *skipNextData(char *scan)
	{
		// while we have data, and we encounter soft seperators or line feeds...
		while ( *scan && mTypes[*scan] == CT_SOFT || mTypes[*scan] == CT_END_OF_LINE )
		{
			if ( *scan == '\n' ) mLineNo++;
			scan++;
		}
		return scan;
	}

	void pushElement(const char *element)
	{
		PX_ASSERT( mStackIndex < MAX_STACK );
		if( mStackIndex < MAX_STACK )
			mStack[mStackIndex++] = element;
	}

	const char *popElement(void)
	{
		return mStackIndex ? mStack[--mStackIndex] : 0;
	}

	static const int MAX_STACK = 2048;

	char mTypes[256];

	physx::PxFileBuf *mFileBuf;
	char			*mReadBuffer;
	physx::PxU32	mReadBufferSize;

	physx::PxI32 mLineNo;
	const char *mError;
	physx::PxU32 mStackIndex;
	const char *mStack[MAX_STACK];
};

FastXml * createFastXml(void)
{
	MyFastXml *m = PX_NEW(MyFastXml);
	return static_cast< FastXml *>(m);
}

}; // end of namespace
