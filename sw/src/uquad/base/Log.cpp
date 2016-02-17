#include "Log.h"


#if defined(UQUAD_PLATFORM_Android)
#include <android/log.h>
#else
#endif

namespace uquad
{
namespace base
{
    Log::Entry::Entry(Log::eLevel l, char const *t)
        : time(chrono::system_clock::now())
        , threadId(this_thread::get_id())
        , level(l)
        , tag(t)
    {
        msg[0] = '\0';
    }
    
    Log::Sink::Sink()
    {
    }
    
    Log::Sink::~Sink()
    {
    }
    
#if defined(UQUAD_PLATFORM_Android)
    class SystemLogSink
            : public Log::Sink
        {
        public:
            SystemLogSink()
            {
            }

            ~SystemLogSink()
            {
            }

            static inline android_LogPriority androidLogPriorityFromLevel(Log::eLevel level)
            {
                switch(level)
                {
                case Log::kVerbose: return ANDROID_LOG_VERBOSE; break;
                case Log::kDebug:   return ANDROID_LOG_DEBUG;   break;
                case Log::kInfo:    return ANDROID_LOG_INFO;    break;
                case Log::kWarning: return ANDROID_LOG_WARN;    break;
                case Log::kError:   return ANDROID_LOG_ERROR;   break;
                case Log::kFatal:   return ANDROID_LOG_FATAL;   break;
                case Log::kSilent:  return ANDROID_LOG_SILENT;  break;
                default:            return ANDROID_LOG_DEFAULT; break;
                }
            }

            void write(intrusive_ptr<Log::Entry> entry)
            {
                __android_log_write(
                        androidLogPriorityFromLevel(entry->level),
                        entry->tag,
                        entry->msg
                );
            }
        };
#else
    class SystemLogSink
        : public Log::Sink
    {
    public:
        SystemLogSink()
        {
        }
        
        ~SystemLogSink()
        {
        }

        void write(intrusive_ptr<Log::Entry> entry)
        {
            auto minutes = chrono::duration_cast<chrono::minutes>(entry->time - chrono::system_clock::time_point());
            
            chrono::duration<chrono::fractional_seconds> d = entry->time - (chrono::system_clock::time_point() + minutes);
            
            std::clog << '|' << entry->tag << '|';
            std::clog << entry->threadId << '|';
            std::clog << Log::levelName(entry->level) << '|';
            
            std::clog << chrono::time_fmt(chrono::timezone::local, "%H:%M:") << entry->time;
            
            std::ios::fmtflags flgs = std::clog.flags();
            std::clog << std::fixed << std::setw(6) << std::setprecision(3) << std::setfill('0') << d.count();
            std::clog.flags(flgs);
            std::clog << '|';
            std::clog << (const char *)entry->msg << std::endl << std::flush;
        }
        
    };
    
#endif
    
    Log::Log()
        : m_Level(kDebug)
        , m_Sink(createSystemSink())
        , m_EntryPool()
    {
        
    }
    
    Log::~Log()
    {
    }
    
    intrusive_ptr<Log::Sink> Log::createSystemSink()
    {
        return intrusive_ptr<Log::Sink>(new SystemLogSink());
    }
    
    intrusive_ptr<Log::Entry> Log::makeEntry(eLevel level, char const *tag)
    {
        return intrusive_ptr<Entry>(m_EntryPool.construct(level, tag));
    }
    
    void Log::freeEntry(Entry *entry)
    {
        m_EntryPool.destroy(entry);
    }
    
    void Log::writeEntry(intrusive_ptr<Entry> entry)
    {
        if(m_Sink && entry->level >= m_Level) m_Sink->write(entry);
    }
    
    char const* Log::levelName(eLevel level)
    {
        switch(level)
        {
        case kVerbose:  return "VERBOSE";   break;
        case kDebug:    return "DEBUG";     break;
        case kInfo:     return "INFO";      break;
        case kWarning:  return "WARNING";   break;
        case kError:    return "ERROR";     break;
        case kFatal:    return "FATAL";     break;
        case kSilent:   return "SILENT";    break;
        default:        return "UNKNOWN";   break;
        }
    }
    
    void Log::setLevel(eLevel level)
    {
        m_Level = level;
    }
    
    void Log::setSink(intrusive_ptr<Sink> sink)
    {
        m_Sink = sink;
    }

} //namespace base
} //namespace uquad
