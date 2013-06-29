#ifndef _RWBINARYSTREAM_H_
#define _RWBINARYSTREAM_H_
#include "gtfwpre.h"

/**
 * @file rwbinarystream.h 
 * Contains the structs for the shared Render Ware binary stream data.
 * Many thanks to http://www.gtamodding.com/index.php?title=RenderWare_binary_stream_file
 */

namespace RW
{
	enum {
		SID_Struct    = 0x0001,
		SID_String    = 0x0002,
		SID_Extension = 0x0003,
		
		SID_Clump     = 0x0010
	};
	
	struct BSSectionHeader
	{
		uint32_t id;
		uint32_t size;
		uint32_t versionid;
	};
	
	struct BSClump 
	{
		uint32_t numatomics;
		uint32_t numlights;
		uint32_t numcameras;
	};
};

#endif