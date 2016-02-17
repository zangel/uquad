#ifndef UQUAD_BASE_FILE_HANDLE_H
#define UQUAD_BASE_FILE_HANDLE_H

#include "Config.h"
#include "RefCounted.h"

namespace uquad
{
namespace base
{
	class FileHandle
    	: public RefCounted
    {
    public:
    
        typedef enum
        {
            kReadOnly = interprocess::read_only,
            kReadWrite = interprocess::read_write,
            kCopyOnWrite = interprocess::copy_on_write,
            kReadPrivate = interprocess::read_private,
            kInvalid = interprocess::invalid_mode
        } eMode;
        
        
    	static interprocess::file_handle_t s_InvalidNativeHandle;

    	FileHandle(interprocess::file_handle_t fh, bool duplicated = false);
    	~FileHandle();
        
        inline interprocess::file_handle_t getNativeHandle() const { return m_Handle; }
        void setNativeHandle(interprocess::file_handle_t h);

    	inline bool isValid() const { return m_Handle != s_InvalidNativeHandle; }
    	system::error_code close();

    private:
    	interprocess::file_handle_t m_Handle;
    };

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_FILE_HANDLE_H
