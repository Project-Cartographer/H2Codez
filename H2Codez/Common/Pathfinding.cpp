#include "pathfinding.h"
#include "../h2codez.h"
#include "../util/Logs.h"
#include "TagInterface.h"

bool pathfinding::generate(scenario_structure_bsp_block *sbsp)
{
	if (LOG_CHECK(sbsp->pathfindingData.size == 0)) // check the map has no pathfinding
	{
		tags::resize_block(&sbsp->pathfindingData, 1);
		if (LOG_CHECK(sbsp->pathfindingData.size == 1) && LOG_CHECK(sbsp->collisionBSP.size > 0))
		{
			auto pathfinding = sbsp->pathfindingData[0];
			auto collision_bsp = sbsp->collisionBSP[0];
			if (!LOG_CHECK(pathfinding && collision_bsp))
				return false;
			tags::resize_block(&pathfinding->vertices, collision_bsp->vertices.size);
			for (size_t i = 0; i < pathfinding->vertices.size; i++)
			{
				auto *p_vertex = pathfinding->vertices[i];
				auto *c_vertex = collision_bsp->vertices[i];
				if (!LOG_CHECK(p_vertex && c_vertex))
					break;
				p_vertex->point = c_vertex->point;
			}

			tags::resize_block(&pathfinding->links, collision_bsp->edges.size);
			for (size_t i = 0; i < pathfinding->links.size; i++)
			{
				auto *link = pathfinding->links[i];
				auto *edge = collision_bsp->edges[i];
				if (!LOG_CHECK(link && edge))
					break;
				link->vertex1 = edge->startVertex;
				link->vertex2 = edge->endVertex;
				link->forwardLink = edge->forwardEdge;
				link->reverseLink = edge->reverseEdge;

				link->rightSector = edge->rightSurface;
				link->leftSector = edge->leftSurface;
				int *flags_ptr = reinterpret_cast<int*>(&link->linkFlags);
				*flags_ptr = link->SectorLinkFromCollisionEdge;
				if (link->rightSector != NONE && link->rightSector != NONE)
					*flags_ptr |= link->SectorLinkBothSectorsWalkable;
			}

			tags::resize_block(&pathfinding->sectors, collision_bsp->surfaces.size);
			tags::resize_block(&pathfinding->refs, collision_bsp->surfaces.size);
			for (size_t i = 0; i < pathfinding->links.size; i++)
			{
				auto *ref = pathfinding->refs[i];
				auto *sector = pathfinding->sectors[i];
				auto *surface = collision_bsp->surfaces[i];
				if (!LOG_CHECK(sector && surface && ref))
					break;
				ref->nodeRefOrSectorRef = i;

				sector->firstLinkdoNotSetManually = surface->firstEdge;
				int *flags_ptr = reinterpret_cast<int*>(&sector->pathfindingSectorFlags);
				*flags_ptr = ((surface->flags & surface->Breakable) ? sector->SectorBreakable : 0) | 
					sector->SectorBspSource | sector->SectorWalkable | sector->Floor;
			}
			return true;
		}
	}
	return false;
}