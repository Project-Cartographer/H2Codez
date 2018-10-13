#include "pathfinding.h"
#include "../h2codez.h"
#include "../util/Logs.h"
#include "TagInterface.h"
#include <map>
#include <unordered_set>
#include <functional>

template<class T>
constexpr bool is_between(const T& v, const T& lo, const T& hi)
{
	return (v >= lo) && (v <= hi);
}

/*
	Helper function for getting blam indices from a map
*/
template<class t_index, class t_data, class Compare, class Allocator>
inline t_data get_idx(std::map<t_index, t_data, Compare, Allocator> map, t_index index)
{
	auto ilter = map.find(index);
	if (ilter != map.end())
		return ilter->second;
	else
		return NONE;
}

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

			std::cout << "Converting surfaces to sectors" << std::endl;

			// calculate surfaces to use
			std::map<unsigned short, unsigned short> sector_surface_mapping;
			std::map<unsigned short, unsigned short> surface_sector_mapping;
			std::unordered_set<unsigned short> edges_used;
			tags::resize_block(&pathfinding->refs, collision_bsp->surfaces.size);
			unsigned short sector_index = 0;
			for (unsigned short surface_idx = 0; surface_idx < collision_bsp->surfaces.size; surface_idx++)
			{
				size_t ref_index = NONE;
				auto surface = collision_bsp->surfaces[surface_idx];
				if (!LOG_CHECK(surface))
					return false;
				auto coll_plane = collision_bsp->planes[surface->plane];
				if (!LOG_CHECK(coll_plane))
				{
					cout << to_string(surface_idx) << endl;
					cout << to_string(surface->plane) << endl;
					continue;
				}
				auto normal_angle = coll_plane->plane.normal.get_angle();
				if (!is_between(normal_angle.roll.as_degree(), 45.0, 135.0))
				{
					edges_used.insert(surface->firstEdge);
					sector_surface_mapping[sector_index] = surface_idx;
					surface_sector_mapping[surface_idx] = sector_index;
					ref_index = sector_index;
					sector_index++;
				}
				auto *ref = pathfinding->refs[surface_idx];
				ref->nodeRefOrSectorRef = ref_index;
			}

			std::cout << "Culling edges and vertices" << std::endl;

			// Ilterating over edges and calculating edges + points used
			std::unordered_set<size_t> vertices_used;
			std::map<unsigned short, unsigned short> edge_link_mapping;
			unsigned short link_idx = 0;
			std::unordered_set<unsigned short> edges_to_check = edges_used; // make a copy for ilterating
			for (unsigned short edge_idx: edges_to_check)
			{
				std::function<void(short)> parse_edge;
				parse_edge = [=, &link_idx, &edge_link_mapping, &vertices_used, &edges_used, &parse_edge](short edge_idx)
				{
					edges_used.insert(edge_idx);
					auto *edge = collision_bsp->edges[edge_idx];

					vertices_used.insert(edge->startVertex);
					vertices_used.insert(edge->endVertex);
					edge_link_mapping[edge_idx] = link_idx;
					link_idx++;
					if (edge->reverseEdge != NONE && edges_used.count(edge->reverseEdge) == 0)
						parse_edge(edge->reverseEdge);
					if (edge->forwardEdge != NONE && edges_used.count(edge->forwardEdge) == 0)
						parse_edge(edge->forwardEdge);
				};
				parse_edge(edge_idx);
			}

			cout << "Starting data copy..." << std::endl;

			cout << "Copying vertices...";
			// copy over vertices
			std::map<unsigned short, unsigned short> coll_to_path_vertex;
			tags::resize_block(&pathfinding->vertices, vertices_used.size());
			size_t p_vertex_idx = 0;
			for (size_t c_vertex_idx: vertices_used)
			{
				auto *p_vertex = pathfinding->vertices[p_vertex_idx];
				auto *c_vertex = collision_bsp->vertices[c_vertex_idx];
				if (!LOG_CHECK(p_vertex && c_vertex))
					break;
				p_vertex->point = c_vertex->point;
				coll_to_path_vertex[c_vertex_idx] = p_vertex_idx;
				p_vertex_idx++;
			}
			cout << "Done" << std::endl;

			cout << "Writing links...";
			// copy over edges
			tags::resize_block(&pathfinding->links, edge_link_mapping.size());
			for (auto &ilter : edge_link_mapping)
			{
				auto link = pathfinding->links[ilter.second];
				auto edge = collision_bsp->edges[ilter.first];
				if (!LOG_CHECK(link && edge))
					break;
				link->vertex1 = get_idx(coll_to_path_vertex, edge->startVertex);
				link->vertex2 = get_idx(coll_to_path_vertex, edge->endVertex);
				link->forwardLink = get_idx(edge_link_mapping, edge->forwardEdge);
				link->reverseLink = get_idx(edge_link_mapping, edge->reverseEdge);

				link->rightSector = get_idx(surface_sector_mapping, edge->rightSurface);
				link->leftSector = get_idx(surface_sector_mapping, edge->leftSurface);
				int *flags_ptr = reinterpret_cast<int*>(&link->linkFlags);
				*flags_ptr = link->SectorLinkFromCollisionEdge;
				if (link->rightSector != NONE && link->rightSector != NONE)
					*flags_ptr |= link->SectorLinkBothSectorsWalkable;
				link_idx++;
			}
			cout << "Done" << std::endl;

			cout << "Writing sectors...";
			tags::resize_block(&pathfinding->sectors, sector_surface_mapping.size());
			for (auto &ilter : sector_surface_mapping)
			{
				auto *sector = pathfinding->sectors[ilter.first];
				auto *surface = collision_bsp->surfaces[ilter.second];
				if (!LOG_CHECK(sector && surface))
					break;

				sector->firstLinkdoNotSetManually = edge_link_mapping[surface->firstEdge];
				int *flags_ptr = reinterpret_cast<int*>(&sector->pathfindingSectorFlags);
				*flags_ptr = ((surface->flags & surface->Breakable) ? sector->SectorBreakable : 0) | 
					sector->SectorBspSource | sector->SectorWalkable | sector->Floor;
			}
			cout << "Done" << endl;

			cout << "Pathfiding generation done!" << endl << endl
				<< "Sectors: " << pathfinding->sectors.size << endl
				<< "Links: " << pathfinding->links.size << endl
				<< "Vertices: " << pathfinding->vertices.size << endl << endl;

			cout << "Coll info" << endl
				<< "Surfaces: " << collision_bsp->surfaces.size << endl
				<< "Edges: " << collision_bsp->edges.size << endl
				<< "Vertices: " << collision_bsp->vertices.size << endl << endl;
			return true;
		}
	}
	return false;
}