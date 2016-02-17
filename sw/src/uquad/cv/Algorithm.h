#ifndef UQUAD_CV_ALGORITHM_H
#define UQUAD_CV_ALGORITHM_H

#include "Config.h"

#include "../base/RefCounted.h"
#include "../base/Delegate.h"

namespace uquad
{
namespace cv
{
    class AlgorithmLibrary;
    
    class Algorithm
    	: public base::RefCounted
    {
    public:
        typedef std::type_index Type;

        
        class Registry
            : public base::RefCounted
        {
        public:
            Registry(std::string const &n, Type const &t);
            ~Registry();
            
            virtual intrusive_ptr<Algorithm> createAlgorithm() const = 0;
            
            template <typename T>
            inline intrusive_ptr<T> createAlgorithm() const { return dynamic_pointer_cast<T>(createAlgorithm()); }
            
            std::string name;
            Type type;
        };
        
    	Algorithm();
    	~Algorithm();
        
        virtual bool isValid() const = 0;
        
        virtual system::error_code prepare() = 0;
        virtual bool isPrepared() const = 0;
        virtual void unprepare() = 0;
        
        template <typename A>
    	static system::error_code registerAlgorithms(AlgorithmLibrary &al);
    };

} //namespace cv
} //namespace uquad

#endif //UQUAD_CV_ALGORITHM_H
