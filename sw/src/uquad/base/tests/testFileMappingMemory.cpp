#include <boost/test/unit_test.hpp>
#include <uquad/base/FileHandle.h>
#include <uquad/base/FileMappingMemory.h>
#include <fstream>

using namespace uquad;


BOOST_AUTO_TEST_CASE(FileMappingMemory)
{
    const std::string filename = "test.map";
    const std::size_t filesize = 9999;
    
    {
        //Create file with given size
        std::ofstream file(filename, std::ios::binary | std::ios::trunc);
        file.seekp(static_cast<std::streamoff>(filesize-1));
        file.write("", 1);
    }
    
    intrusive_ptr<base::FileHandle> fh;
    {
        interprocess::file_mapping filemapping(filename.c_str(), interprocess::read_write);
        fh.reset(new base::FileHandle(filemapping.get_mapping_handle().handle));
    }
    
    BOOST_TEST(fh->isValid());
    
    intrusive_ptr<base::FileMappingMemory> fmm(new base::FileMappingMemory(fh));
    BOOST_TEST(fmm->isValid());
    BOOST_TEST(fmm->data());
    BOOST_TEST(fmm->size() == filesize);
    
    fh.reset();
    BOOST_TEST(fmm->isValid());
    
    fmm->release();
    interprocess::file_mapping::remove(filename.c_str());
    
    BOOST_TEST(!fmm->isValid());
    fmm.reset();
}
