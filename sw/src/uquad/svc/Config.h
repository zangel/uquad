#ifndef UQUAD_SVC_CONFIG_H
#define UQUAD_SVC_CONFIG_H

#include "../Config.h"

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#if defined(UQUAD_PLATFORM_Mac)
#include "Mac/Config.h"
#elif defined(UQUAD_PLATFORM_Android)
#include "Android/Config.h"
#endif

#endif //UQUAD_SVC_CONFIG_H
