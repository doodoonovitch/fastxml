#ifndef __PX_FILE_BUFFER_H__
#define __PX_FILE_BUFFER_H__

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

#include "PxSimpleTypes.h"
#include "PxFileBuf.h"
#include "PsFile.h"
#include "PsUserAllocated.h"

namespace physx
{
class PxFileBuffer : public PxFileBuf, public UserAllocated
{
public:
	PxFileBuffer(const char *fileName,OpenMode mode)
	{
		mOpenMode = mode;
		mFph = NULL;
		mFileLength = 0;
		mSeekRead   = 0;
		mSeekWrite  = 0;
		mSeekCurrent = 0;
		switch ( mode )
		{
			case OPEN_READ_ONLY:
				physx::fopen_s(&mFph,fileName,"rb");
				break;
			case OPEN_WRITE_ONLY:
				physx::fopen_s(&mFph,fileName,"wb");
				break;
			case OPEN_READ_WRITE_NEW:
				physx::fopen_s(&mFph,fileName,"wb+");
				break;
			case OPEN_READ_WRITE_EXISTING:
				physx::fopen_s(&mFph,fileName,"rb+");
				break;
		}
		if ( mFph )
		{
			fseek(mFph,0L,SEEK_END);
			mFileLength = ftell(mFph);
			fseek(mFph,0L,SEEK_SET);
		}
		else
		{
			mOpenMode = OPEN_FILE_NOT_FOUND;
		}
    }

	virtual						~PxFileBuffer()
	{
		if ( mFph )
		{
			fclose(mFph);
		}
	}

	virtual SeekType isSeekable(void) const
	{
		return mSeekType;
	}

	virtual		PxU32			read(void* buffer, PxU32 size)	
	{
		PxU32 ret = 0;
		if ( mFph )
		{
			setSeekRead();
			ret = (PxU32)::fread(buffer,1,size,mFph);
			mSeekRead+=ret;
			mSeekCurrent+=ret;
		}
		return ret;
	}

	virtual		PxU32			peek(void* buffer, PxU32 size)
	{
		PxU32 ret = 0;
		if ( mFph )
		{
			PxU32 loc = tellRead();
			setSeekRead();
			ret = (PxU32)::fread(buffer,1,size,mFph);
			mSeekCurrent+=ret;
			seekRead(loc);
		}
		return ret;
	}

	virtual		PxU32		write(const void* buffer, PxU32 size)
	{
		PxU32 ret = 0;
		if ( mFph )
		{
			setSeekWrite();
			ret = (PxU32)::fwrite(buffer,1,size,mFph);
			mSeekWrite+=ret;
			mSeekCurrent+=ret;
			if ( mSeekWrite > mFileLength )
			{
				mFileLength = mSeekWrite;
			}
		}
		return ret;
	}

	virtual PxU32 tellRead(void) const
	{
		return mSeekRead;
	}

	virtual PxU32 tellWrite(void) const
	{
		return mSeekWrite;
	}

	virtual PxU32 seekRead(PxU32 loc) 
	{
		mSeekRead = loc;
		if ( mSeekRead > mFileLength )
		{
			mSeekRead = mFileLength;
		}
		return mSeekRead;
	}

	virtual PxU32 seekWrite(PxU32 loc)
	{
		mSeekWrite = loc;
		if ( mSeekWrite > mFileLength )
		{
			mSeekWrite = mFileLength;
		}
		return mSeekWrite;
	}

	virtual void flush(void)
	{
		if ( mFph )
		{
			::fflush(mFph);
		}
	}

	virtual OpenMode	getOpenMode(void) const
	{
		return mOpenMode;
	}

	virtual PxU32 getFileLength(void) const
	{
		return mFileLength;
	}

private:
	// Moves the actual file pointer to the current read location
	void setSeekRead(void) 
	{
		if ( mSeekRead != mSeekCurrent && mFph )
		{
			if ( mSeekRead >= mFileLength )
			{
				::fseek(mFph,0L,SEEK_END);
			}
			else
			{
				::fseek(mFph,mSeekRead,SEEK_SET);
			}
			mSeekCurrent = mSeekRead = ::ftell(mFph);
		}
	}
	// Moves the actual file pointer to the current write location
	void setSeekWrite(void)
	{
		if ( mSeekWrite != mSeekCurrent && mFph )
		{
			if ( mSeekWrite >= mFileLength )
			{
				::fseek(mFph,0L,SEEK_END);
			}
			else
			{
				::fseek(mFph,mSeekWrite,SEEK_SET);
			}
			mSeekCurrent = mSeekWrite = ::ftell(mFph);
		}
	}


	FILE		*mFph;
	PxU32		mSeekRead;
	PxU32		mSeekWrite;
	PxU32		mSeekCurrent;
	PxU32		mFileLength;
	SeekType	mSeekType;
	OpenMode	mOpenMode;
};

};

#endif
