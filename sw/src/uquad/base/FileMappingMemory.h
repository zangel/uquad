#ifndef UQUAD_BASE_FILE_MAPPING_MEMORY_H
#define UQUAD_BASE_FILE_MAPPING_MEMORY_H

#include "Config.h"
#include "Memory.h"
#include "FileHandle.h"

namespace uquad
{
namespace base
{
	class FileMappingMemory
    	: public Memory
    {
    public:
        FileMappingMemory(intrusive_ptr<FileHandle> const &fh, FileHandle::eMode mode = FileHandle::kReadWrite, std::size_t size = 0);
    	~FileMappingMemory();

        bool isValid() const;
    	void* data();
    	std::size_t size() const;

    	system::error_code release();

    private:
    	intrusive_ptr<FileHandle> m_FileHandle;
    	interprocess::mapped_region m_MappedRegion;
    };

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_FILE_MAPPING_MEMORY_H
