#ifndef UQUAD_CV_ALGORITHM_LIBRARY_H
#define UQUAD_CV_ALGORITHM_LIBRARY_H

#include "Config.h"
#include "../base/RefCounted.h"
#include "Algorithm.h"

namespace uquad
{
namespace cv
{
    class PoseTracker;
    
    class AlgorithmLibrary
    {
	public:
        
        typedef mi::multi_index_container
        <
            intrusive_ptr<Algorithm::Registry>,
            mi::indexed_by
            <
                mi::random_access<>,
                mi::hashed_unique< mi::identity< intrusive_ptr<Algorithm::Registry> > >,
                mi::ordered_non_unique
                <
                    mi::member
                    <
                        Algorithm::Registry,
                        std::string,
                        &Algorithm::Registry::name
                    >
                >,
                mi::ordered_non_unique
                <
                    mi::member
                    <
                        Algorithm::Registry,
                        Algorithm::Type,
                        &Algorithm::Registry::type
                    >
                >
            >
        > registries_t;
        
        typedef mi::nth_index<registries_t, 1>::type registries_by_identity_t;
		typedef mi::nth_index<registries_t, 2>::type registries_by_name_t;
        typedef mi::nth_index<registries_t, 3>::type registries_by_type_t;
		
    protected:
        AlgorithmLibrary();

    public:
        ~AlgorithmLibrary();

        bool registerAlgorithm(intrusive_ptr<Algorithm::Registry> const &d);
        bool deregisterAlgorithm(intrusive_ptr<Algorithm::Registry> const &d);

        inline registries_t const& getAlgorithms() const { return m_Registries; }

        std::pair<
            registries_by_name_t::const_iterator,
            registries_by_name_t::const_iterator
        > findAlgorithmsByName(std::string const &n) const;

        std::pair<
           registries_by_type_t::const_iterator,
           registries_by_type_t::const_iterator
        > findAlgorithmsByType(Algorithm::Type const &type) const;

        template <typename A, typename Exp>
        inline void enumerateAlgorithms(Exp exp) const
        {
            std::pair<
               registries_by_type_t::const_iterator,
               registries_by_type_t::const_iterator
            > registries = findAlgorithmsByType(Algorithm::Type(typeid(A)));
            
            for(registries_by_type_t::const_iterator ir = registries.first; ir != registries.second; ++ir)
            {
                if(!exp(dynamic_pointer_cast<typename A::Registry>(*ir))) break;
            }
        }

        template <typename A, typename Exp>
        inline void enumerateAlgorithms(std::string const &name, Exp exp) const
        {
            std::pair<
                registries_by_name_t::const_iterator,
                registries_by_name_t::const_iterator
            > registries = findAlgorithmsByName(name);

            Algorithm::Type const t = typeid(A);
            for(registries_by_name_t::const_iterator ir = registries.first; ir != registries.second; ++ir)
            {
                if((*ir)->type != t) continue;
                if(!exp(dynamic_pointer_cast<typename A::Registry>(*ir))) break;
            }
        }

        template <typename Exp>
        inline void enumerateAlgorithms(std::string const &name, Exp exp) const
        {
            std::pair<
                registries_by_name_t::const_iterator,
                registries_by_name_t::const_iterator
            > registries = findAlgorithmsByName(name);

            for(registries_by_name_t::const_iterator ir = registries.first; ir != registries.second; ++ir)
            {
                if(!exp(*ir)) break;
            }
        }

        intrusive_ptr<PoseTracker> createPoseTracker(std::string const &name) const;

	private:
       mutable mutex m_Sync;
       registries_t m_Registries;
	};

} //namespace cv
} //namespace uquad

#endif //UQUAD_CV_ALGORITHM_LIBRARY_H
