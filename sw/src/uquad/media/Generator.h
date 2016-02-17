#ifndef UQUAD_MEDIA_GENERATOR_H
#define UQUAD_MEDIA_GENERATOR_H

#include "Config.h"
#include "Source.h"
#include "../base/Delegate.h"

namespace uquad
{
namespace media
{
    class GeneratorDelegate
	{
	public:
		GeneratorDelegate() = default;
		virtual ~GeneratorDelegate() = default;
        virtual void onSampleGenerated() {}
	};
    
    class Generator
    	: public Source
    {
    public:
        typedef base::Delegate<GeneratorDelegate> generator_delegate_t;
    	
        Generator();
    	~Generator();
        
        inline generator_delegate_t const& generatorDelegate() { return m_GeneratorDelegate; }
        
    protected:
        inline void notifyOnSampleGenerated()
        {
            m_GeneratorDelegate.call(&GeneratorDelegate::onSampleGenerated);
        }
        
    protected:
        generator_delegate_t m_GeneratorDelegate;
            
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_GENERATOR_H
