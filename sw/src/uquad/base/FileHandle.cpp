#include "FileHandle.h"
#include "Error.h"

namespace uquad
{
namespace base
{
	interprocess::file_handle_t FileHandle::s_InvalidNativeHandle = interprocess::ipcdetail::invalid_file();

	FileHandle::FileHandle(interprocess::file_handle_t fh, bool duplicated)
		: m_Handle(duplicated?
				fh:(fh != s_InvalidNativeHandle ? ::dup(fh) : s_InvalidNativeHandle)
			)
	{

	}

	FileHandle::~FileHandle()
	{
		close();
	}
    
    void FileHandle::setNativeHandle(interprocess::file_handle_t h)
    {
        close();
        m_Handle = h;
    }

	system::error_code FileHandle::close()
	{
		if(m_Handle == s_InvalidNativeHandle)
		{
			return makeErrorCode(kENotOpened);
		}

		interprocess::ipcdetail::close_file(m_Handle);
		m_Handle = s_InvalidNativeHandle;
		return makeErrorCode(kENoError);

	}

} //namespace base
} //namespace uquad

