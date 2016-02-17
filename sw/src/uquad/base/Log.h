#ifndef UQUAD_BASE_LOG_H
#define UQUAD_BASE_LOG_H

#include "Config.h"
#include "RefCounted.h"
#include "../utility/Singleton.h"

#define UQUAD_BASE_LOG_ENTRY_MSG_SIZE    512

#define UQUAD_BASE_LOG_LEVEL_MAKE_EXPANDED(level) ::uquad::base::Log::k##level
#define UQUAD_BASE_LOG_LEVEL_MAKE(level) UQUAD_BASE_LOG_LEVEL_MAKE_EXPANDED(level)

namespace uquad
{
namespace base
{
    class Log
    {
    public:
        
        typedef enum
        {
            kVerbose,
            kInfo,
            kDebug,
            kWarning,
            kError,
            kFatal,
            kSilent
        } eLevel;
        
    protected:
        Log();

    public:
        ~Log();
        
        class Entry
            : public RefCounted
        {
        public:
            Entry(eLevel l, char const *t);
            
            chrono::system_clock::time_point time;
            thread::id threadId;
            eLevel level;
            char const *tag;
            char msg[UQUAD_BASE_LOG_ENTRY_MSG_SIZE];
        };
        
        class Sink
            : public RefCounted
        {
        public:
            Sink();
            virtual ~Sink();
            
            virtual void write(intrusive_ptr<Entry> entry) = 0;
        };
        
        class Stream
        {
        public:
            Stream(eLevel level, char const *tag)
                : m_Entry(SI(Log).makeEntry(level, tag))
                , m_Stream(&m_Entry->msg[0], &m_Entry->msg[0] + UQUAD_BASE_LOG_ENTRY_MSG_SIZE)
            {
            }
            
            ~Stream()
            {
                m_Stream << '\0' << std::flush;
                
                SI(Log).writeEntry(m_Entry);
            }
            
            template <typename T>
            inline Stream& operator <<(T const &arg)
            {
                m_Stream << arg;
                return *this;
            }
            
        private:
            intrusive_ptr<Entry> m_Entry;
            iostreams::stream<iostreams::array_sink> m_Stream;

        };
        
        intrusive_ptr<Sink> createSystemSink();
        
        intrusive_ptr<Entry> makeEntry(eLevel level, char const *tag);
        void freeEntry(Entry *entry);
        void writeEntry(intrusive_ptr<Entry> entry);
        
        static char const* levelName(eLevel level);
        
        void setLevel(eLevel level);
        void setSink(intrusive_ptr<Sink> sink);
        
        
    private:
        eLevel m_Level;
        intrusive_ptr<Sink> m_Sink;
        object_pool<Entry> m_EntryPool;
    };
    
    static inline void intrusive_ptr_release(const Log::Entry *entry)
    {
        if(thread_safe_counter::decrement(entry->m_RefCounter) == 0)
            SI(Log).freeEntry(const_cast<Log::Entry *>(entry));
    }

} //namespace base
} //namespace uquad

#define UQUAD_BASE_LOG_MODULE(level, tag) uquad::base::Log::Stream(uquad::base::Log::k##level, tag)
#define UQUAD_BASE_LOG(level) UQUAD_BASE_LOG_MODULE(level, "base")

#endif //UQUAD_BASE_LOG_H
