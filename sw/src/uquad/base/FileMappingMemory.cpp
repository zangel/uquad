#include "FileMappingMemory.h"
#include "Error.h"

namespace uquad
{
namespace base
{
    namespace
    {
        struct file_handle_mappable
        {
            file_handle_mappable(intrusive_ptr<FileHandle> const &fh)
                : m_Handle(fh)
            {
            }
            
            inline interprocess::mapping_handle_t get_mapping_handle() const
            {
                return interprocess::ipcdetail::mapping_handle_from_file_handle(m_Handle->getNativeHandle());
            }
            
            intrusive_ptr<FileHandle> m_Handle;
        };
        
    }
    
	FileMappingMemory::FileMappingMemory(intrusive_ptr<FileHandle> const &fh, FileHandle::eMode mode, std::size_t size)
		: Memory()
		, m_FileHandle()
        , m_MappedRegion()
	{
        try
        {
            m_MappedRegion = interprocess::mapped_region(file_handle_mappable(fh), interprocess::mode_t(mode), 0, size);
            m_FileHandle = fh;
        }
        catch(...)
        {
        }
	}

	FileMappingMemory::~FileMappingMemory()
	{
        release();
	}
    
    bool FileMappingMemory::isValid() const
    {
        return m_FileHandle.get();
    }

	void* FileMappingMemory::data()
	{
		return m_MappedRegion.get_address();
	}

	std::size_t FileMappingMemory::size() const
	{
		return m_MappedRegion.get_size();
	}
    
    system::error_code FileMappingMemory::release()
    {
        if(!isValid())
        {
            return makeErrorCode(kENotOpened);
        }
        
        try
        {
            m_MappedRegion = interprocess::mapped_region();
        }
        catch(...)
        {
            return makeErrorCode(kENotApplicable);
        }
        
        m_FileHandle->close();
        m_FileHandle.reset();
        return makeErrorCode(kENoError);
    }

} //namespace base
} //namespace uquad
