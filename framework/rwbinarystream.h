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
		SID_Clump        = 0x0010,
		
		SID_GeometryList = 0x001A,
		
		SID_HAnimPLG     = 0x011E,
		
		SID_NodeName     = 0x0253F2FE
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
		
		enum {
			IsTriangleStrip   = 0x1,
			VertexTranslation = 0x2,
			TexCoords1        = 0x4,
			VertexColors      = 0x8,
			StoreNormals      = 0x16,
			DynamicVertexLighting = 0x32,
			ModuleMaterialColor   = 0x64,
			TexCoords2        = 0x128
		};
	};
	
	typedef uint32_t BSColor;
	
	struct BSGeometryColor
	{
		BSColor ambient;
		BSColor diffuse;
		BSColor specular;
	};
	
	struct BSGeometryUV
	{
		float u;
		float v;
	};
	
	struct BSGeometryTriangle
	{
		uint16_t first;
		uint16_t second;
		uint16_t attrib; // Who designed this nonsense.
		uint16_t third;
	};
	
	struct BSGeometryBounds
	{
		BSTVector3 center;
		float radius;
		uint32_t positions;
		uint32_t normals;
	};
};

#endif