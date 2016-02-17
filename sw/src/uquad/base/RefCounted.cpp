#include "RefCounted.h"

namespace uquad
{
namespace base
{
	RefCounted::RefCounted()
        : m_RefCounter(0)
	{
	}

	RefCounted::~RefCounted()
	{
	}

} //namespace base
} //namespace uquad
