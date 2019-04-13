#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct global_collision_bsp_block
{
	struct bsp3d_nodes_block
	{
		BYTE padding266[8];
	};
	CHECK_STRUCT_SIZE(bsp3d_nodes_block, 8);
	tag_block<bsp3d_nodes_block> bSP3DNodes;


	struct planes_block
	{
		real_plane3d plane;
	};
	CHECK_STRUCT_SIZE(planes_block, 16);
	tag_block<planes_block> planes;
	inline real_plane3d get_plane_by_ref(unsigned short ref)
	{
		auto plane_block = planes[ref & 0x7FFF];
		if (LOG_CHECK(plane_block))
		{
			if (ref & 0x8000) {
				real_plane3d out;
				out.distance = -plane_block->plane.distance;
				out.normal.i = -plane_block->plane.normal.i;
				out.normal.j = -plane_block->plane.normal.j;
				out.normal.k = -plane_block->plane.normal.k;
				return out;
			} else {
				return plane_block->plane;
			}
		}
		return real_plane3d();
	}

	struct leaves_block
	{

		enum Flags : BYTE
		{
			ContainsDoubleSidedSurfaces = 0x1,
		};
		Flags flags;
		BYTE bSP2DReferenceCount;
		short firstBSP2DReference;
	};
	CHECK_STRUCT_SIZE(leaves_block, 4);
	tag_block<leaves_block> leaves;


	struct bsp2d_references_block
	{
		short plane;
		short bSP2DNode;
	};
	CHECK_STRUCT_SIZE(bsp2d_references_block, 4);
	tag_block<bsp2d_references_block> bSP2DReferences;


	struct bsp2d_nodes_block
	{
		real_plane2d plane;
		short leftChild;
		short rightChild;
	};
	CHECK_STRUCT_SIZE(bsp2d_nodes_block, 16);
	tag_block<bsp2d_nodes_block> bSP2DNodes;


	struct surfaces_block
	{
		unsigned short plane;
		unsigned short firstEdge;

		enum Flags : BYTE
		{
			TwoSided = 0x1,
			Invisible = 0x2,
			Climbable = 0x4,
			Breakable = 0x8,
			Invalid = 0x10,
			Conveyor = 0x20,
		};
		Flags flags;
		BYTE breakableSurface;
		short material;
	};
	CHECK_STRUCT_SIZE(surfaces_block, 8);
	tag_block<surfaces_block> surfaces;


	struct edges_block
	{
		unsigned short startVertex;
		unsigned short endVertex;
		unsigned short forwardEdge;
		unsigned short reverseEdge;
		unsigned short leftSurface;
		unsigned short rightSurface;
	};
	CHECK_STRUCT_SIZE(edges_block, 12);
	tag_block<edges_block> edges;


	struct vertices_block
	{
		real_point3d point;
		short firstEdge;
		BYTE padding272[2];
	};
	CHECK_STRUCT_SIZE(vertices_block, 16);
	tag_block<vertices_block> vertices;

};

struct global_structure_physics_struct_block
{
	/****************************************
	* definition_name: mopp_code_data
	* flags: 0
	* alignment_bit: 16
	* byteswap_proc: 0x00531b20
	****************************************/
	// DataSize(1048576)
	byte_ref moppCode;
	BYTE padding307[4];
	real_point3d moppBoundsMin;
	real_point3d moppBoundsMax;
	/****************************************
	* definition_name: mopp_code_data
	* flags: 0
	* alignment_bit: 16
	* byteswap_proc: 0x00531b20
	****************************************/
	// DataSize(1048576)
	byte_ref BreakableSurfacesMoppCode;
	struct breakable_surface_key_table_block
	{
		short InstancedGeometryIndex;
		short BreakableSurfaceIndex;
		int SeedSurfaceIndex;
		float x0;
		float x1;
		float y0;
		float y1;
		float z0;
		float z1;
	};
	CHECK_STRUCT_SIZE(breakable_surface_key_table_block, 32);
	tag_block<breakable_surface_key_table_block> breakableSurfaceKeyTable;

};
CHECK_STRUCT_SIZE(global_structure_physics_struct_block, 80);

struct scenario_structure_bsp_block
{
	struct global_tag_import_info_block
	{
		int build;
		char version[256];
		char importDate[32];
		char culprit[32];
		BYTE padding218[96];
		char importTime[32];
		BYTE padding219[4];
		struct tag_import_file_block
		{
			char path[256];
			char modificationDate[32];
			FILETIME import_time;
			BYTE padding260[88];
			int checksumCrc32;
			int sizeBytes;
			/****************************************
			* definition_name: tag_import_file_zipped_data_definition
			* flags: 5
			* alignment_bit: 0
			****************************************/
			// DataSize(134217728)
			byte_ref zippedData;
			BYTE padding261[128];
		};
		CHECK_STRUCT_SIZE(tag_import_file_block, 540);

		tag_block<tag_import_file_block> files;

		BYTE padding220[128];
	};
	CHECK_STRUCT_SIZE(global_tag_import_info_block, 596);
	
	tag_block<global_tag_import_info_block> importInfo;

	BYTE padding207[4];

	struct structure_collision_materials_block
	{
		// TagReference("shad")
		tag_ref oldShader;
		BYTE padding231[2];
		// BlockIndex1("structure_bsp_conveyor_surface_block")
		short conveyorSurfaceIndex;
		// TagReference("shad")
		tag_ref newShader;
	};
	CHECK_STRUCT_SIZE(structure_collision_materials_block, 36);

	tag_block<structure_collision_materials_block> collisionMaterials;

	tag_block<global_collision_bsp_block> collisionBSP;

	/// Height below which vehicles get pushed up by an unstoppable force.
	float vehicleFloorWorldUnits;
	/// Height above which vehicles get pushed down by an unstoppable force.
	float vehicleCeilingWorldUnits;
	struct UNUSED_structure_bsp_node_block
	{
		BYTE padding211[6];
	};
	CHECK_STRUCT_SIZE(UNUSED_structure_bsp_node_block, 6);
	tag_block<UNUSED_structure_bsp_node_block> uNUSEDNodes;

	struct structure_bsp_leaf_block
	{
		short cluster;
		short surfaceReferenceCount;
		int firstSurfaceReferenceIndex;
	};
	CHECK_STRUCT_SIZE(structure_bsp_leaf_block, 8);
	tag_block<structure_bsp_leaf_block> leaves;

	real_bounds worldBoundsX;
	real_bounds worldBoundsY;
	real_bounds worldBoundsZ;

	struct structure_bsp_surface_reference_block
	{
		short stripIndex;
		short lightmapTriangleIndex;
		int bSPNodeIndex;
	};
	CHECK_STRUCT_SIZE(structure_bsp_surface_reference_block, 8);
	tag_block<structure_bsp_surface_reference_block> surfaceReferences;

	/****************************************
	* definition_name: structure_bsp_cluster_data_definition
	* flags: 0
	* alignment_bit: 0
	****************************************/
	// DataSize(65536)
	byte_ref clusterData;
	tag_block_ref clusterPortals;

	tag_block_ref fogPlanes;

	BYTE padding208[24];
	tag_block_ref weatherPalette;

	tag_block_ref weatherPolyhedra;

	tag_block_ref detailObjects;

	tag_block_ref clusters;

	tag_block_ref materials;

	tag_block_ref skyOwnerCluster;

	tag_block_ref conveyorSurfaces;

	tag_block_ref breakableSurfaces;

	struct pathfinding_data_block
	{
		struct sector_block
		{

			enum PathfindingSectorFlags : short
			{
				SectorWalkable = 0x1,
				SectorBreakable = 0x2,
				SectorMobile = 0x4,
				SectorBspSource = 0x8,
				Floor = 0x10,
				Ceiling = 0x20,
				WallNorth = 0x40,
				WallSouth = 0x80,
				WallEast = 0x100,
				WallWest = 0x200,
				Crouchable = 0x400,
				Aligned = 0x800,
				SectorStep = 0x1000,
				SectorInterior = 0x2000,
			};
			PathfindingSectorFlags pathfindingSectorFlags;
			short hintIndex;
			int firstLinkdoNotSetManually;

			inline bool is_walkable()
			{
				return pathfindingSectorFlags & SectorWalkable;
			}
		};
		CHECK_STRUCT_SIZE(sector_block, 8);
		tag_block<sector_block> sectors;

		struct sector_link_block
		{
			short vertex1;
			short vertex2;

			enum LinkFlags : short
			{
				SectorLinkFromCollisionEdge = 0x1,
				SectorIntersectionLink = 0x2,
				SectorLinkBsp2dCreationError = 0x4,
				SectorLinkTopologyError = 0x8,
				SectorLinkChainError = 0x10,
				SectorLinkBothSectorsWalkable = 0x20,
				SectorLinkMagicHangingLink = 0x40,
				SectorLinkThreshold = 0x80,
				SectorLinkCrouchable = 0x100,
				SectorLinkWallBase = 0x200,
				SectorLinkLedge = 0x400,
				SectorLinkLeanable = 0x800,
				SectorLinkStartCorner = 0x1000,
				SectorLinkEndCorner = 0x2000,
			};
			LinkFlags linkFlags;
			short hintIndex;
			short forwardLink;
			short reverseLink;
			short leftSector;
			short rightSector;
		};
		CHECK_STRUCT_SIZE(sector_link_block, 16);
		tag_block<sector_link_block> links;

		struct ref_block
		{
			int nodeRefOrSectorRef;
		};
		CHECK_STRUCT_SIZE(ref_block, 4);
		tag_block<ref_block> refs;

		struct sector_bsp2d_nodes_block
		{
			real_plane2d plane;
			int leftChild;
			int rightChild;
		};
		CHECK_STRUCT_SIZE(sector_bsp2d_nodes_block, 20);
		tag_block<sector_bsp2d_nodes_block> bsp2dNodes;

		struct surface_flags_block
		{
			int flags;
		};
		CHECK_STRUCT_SIZE(surface_flags_block, 4);
		tag_block<surface_flags_block> surfaceFlags;

		struct sector_vertex_block
		{
			real_point3d point;
		};
		CHECK_STRUCT_SIZE(sector_vertex_block, 12);
		tag_block<sector_vertex_block> vertices;

		struct environment_object_refs
		{

			enum Flags : short
			{
				Mobile = 0x1,
			};
			Flags flags;
			BYTE padding284[2];
			int firstSector;
			int lastSector;
			struct environment_object_bsp_refs
			{
				int bspReference;
				int firstSector;
				int lastSector;
				short nodeindex;
				BYTE padding291[2];
			};
			CHECK_STRUCT_SIZE(environment_object_bsp_refs, 16);
			tag_block<environment_object_bsp_refs> bsps;

			struct environment_object_nodes
			{
				short referenceFrameIndex;
				BYTE projectionAxis;

				enum ProjectionSign : BYTE
				{
					_ProjectionSign = 0x1,
				};
				ProjectionSign projectionSign;
			};
			CHECK_STRUCT_SIZE(environment_object_nodes, 4);
			tag_block<environment_object_nodes> nodes;

		};
		CHECK_STRUCT_SIZE(environment_object_refs, 36);
		tag_block<environment_object_refs> objectRefs;

		struct pathfinding_hints_block
		{

			enum HintType : short
			{
				IntersectionLink = 0,
				JumpLink = 1,
				ClimbLink = 2,
				VaultLink = 3,
				MountLink = 4,
				HoistLink = 5,
				WallJumpLink = 6,
				BreakableFloor = 7,
			};
			HintType hintType;
			short nextHintIndex;
			short hintData0;
			short hintData1;
			short hintData2;
			short hintData3;
			short hintData4;
			short hintData5;
			short hintData6;
			short hintData7;
		};
		CHECK_STRUCT_SIZE(pathfinding_hints_block, 20);
		tag_block<pathfinding_hints_block> pathfindingHints;

		struct instanced_geometry_reference_block
		{
			short pathfindingObjectindex;
			BYTE padding296[2];
		};
		CHECK_STRUCT_SIZE(instanced_geometry_reference_block, 4);
		tag_block<instanced_geometry_reference_block> instancedGeometryRefs;

		int structureChecksum;
		BYTE padding279[32];
		struct user_hint_block
		{
			tag_block_ref pointGeometry;

			tag_block_ref rayGeometry;

			tag_block_ref lineSegmentGeometry;

			tag_block_ref parallelogramGeometry;

			tag_block_ref polygonGeometry;

			tag_block_ref jumpHints;

			tag_block_ref climbHints;

			tag_block_ref wellHints;

			tag_block_ref flightHints;

		};
		tag_block<user_hint_block> userplacedHints;

	};
	CHECK_STRUCT_SIZE(pathfinding_data_block, 156);
	tag_block<pathfinding_data_block> pathfindingData;


	struct structure_bsp_pathfinding_edges_block
	{
		BYTE midpoint;
	};
	CHECK_STRUCT_SIZE(structure_bsp_pathfinding_edges_block, 1);
	tag_block<structure_bsp_pathfinding_edges_block> pathfindingEdges;

	tag_block_ref backgroundSoundPalette;

	tag_block_ref soundEnvironmentPalette;

	/****************************************
	* definition_name: structure_bsp_cluster_encoded_sound_data
	* flags: 0
	* alignment_bit: 0
	****************************************/
	// DataSize(131072)
	byte_ref soundPASData;
	tag_block_ref markers;

	tag_block_ref runtimeDecals;

	tag_block_ref environmentObjectPalette;

	tag_block_ref environmentObjects;

	tag_block_ref lightmaps;

	BYTE padding209[4];
	tag_block_ref leafMapLeaves;

	tag_block_ref leafMapConnections;

	tag_block_ref errors;

	tag_block_ref precomputedLighting;

	tag_block_ref instancedGeometriesDefinitions;

	tag_block_ref instancedGeometryInstances;

	tag_block_ref AmbienceSoundClusters;

	tag_block_ref ReverbSoundClusters;

	tag_block_ref transparentPlanes;

	BYTE padding210[96];
	/// Distances this far and longer from limit origin will pull you back in.
	float vehicleSpericalLimitRadius;
	/// Center of space in which vehicle can move.
	real_point3d vehicleSpericalLimitCenter;
	tag_block_ref debugInfo;

	// TagReference("DECP")
	tag_ref decorators_tag;
	global_structure_physics_struct_block structurephysics;

	tag_block_ref waterDefinitions;

	tag_block_ref portaldeviceMapping;

	tag_block_ref Audibility;

	tag_block_ref ObjectFakeLightprobes;

	tag_block_ref decorators_block;

};
CHECK_STRUCT_SIZE(scenario_structure_bsp_block, 792);
