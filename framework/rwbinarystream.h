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
		SID_Struct       = 0x0001,
		SID_String       = 0x0002,
		SID_Extension    = 0x0003,
		
		SID_Texture      = 0x0006,
		SID_Material     = 0x0007,
		SID_MaterialList = 0x0008,
		
		SID_FrameList    = 0x000E,
		SID_Geometry     = 0x000F,
		SID_Clump        = 0x0010
	};
	
	/**
	 * Vector data
	 */
	struct BSTVector3
	{
		float x, y, z;
	};
	
	/**
	 * Rotation Matrix
	 */
	struct BSTMatrix
	{
		BSTVector3 a, b, c;
	};
	
	struct BSSectionHeader
	{
		uint32_t id;
		uint32_t size;
		uint32_t versionid;
	};
	
	struct BSExtension
	{
		
	};
	
	struct BSFrameList
	{
		uint32_t numframes;
	};
	
	struct BSFrameListFrame //??????
	{
		BSTMatrix rotation;
		BSTVector3 postiion;
		uint32_t index;
		uint32_t matrixflags; // UNUSED BY ANYTHING.
	};
	
	struct BSClump 
	{
		uint32_t numatomics;
	};
	
	struct BSStruct
	{
		uint32_t id; // = 0x0001
	};
	
	struct BSGeometryList
	{
		uint32_t numgeometry;
	};
	
	struct BSGeometry
	{
		uint16_t flags;
		uint8_t numuvs;
		uint8_t geomflags;
		uint32_t numtris;
		uint32_t numverts;
		uint32_t numframes;
	};
};

#endif