#ifndef UQUAD_BASE_CONFIG_H
#define UQUAD_BASE_CONFIG_H

#include "../Config.h"
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/toolbox/color_spaces/ycbcr.hpp>
#include <boost/gil/extension/yuv420sp/yuv420sp.hpp>
#include <boost/gil/extension/dynamic_image/dynamic_image_all.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/mpl/if.hpp>
#include <boost/pool/object_pool.hpp>

#include <boost/archive/basic_archive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/archive/basic_binary_iprimitive.hpp>
#include <boost/archive/basic_binary_oprimitive.hpp>
#include <boost/archive/detail/common_iarchive.hpp>
#include <boost/archive/detail/common_oarchive.hpp>
#include <boost/serialization/item_version_type.hpp>

//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/base_object.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>

#include "../utility/Singleton.h"

#endif //UQUAD_BASE_CONFIG_H
