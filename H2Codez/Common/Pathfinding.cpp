#include "pathfinding.h"
#include "../h2codez.h"
#include "../util/Logs.h"
#include "TagInterface.h"
#include <map>
#include <unordered_set>
#include <functional>

using namespace pathfinding;

template<class T>
constexpr bool is_between(const T& v, const T& lo, const T& hi)
{
	return (v >= lo) && (v <= hi);
}

/*
	Helper function for getting blam indices from a map
*/
template<class t_index, class t_data, class Compare, class Allocator>
inline t_data get_idx(const std::map<t_index, t_data, Compare, Allocator> &map, t_index index)
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
			std::map<unsigned short, unsigned short> surface_sector_mapping;
			tags::resize_block(&pathfinding->refs, collision_bsp->surfaces.size);
			unsigned short sector_index = 0;
			for (unsigned short surface_idx = 0; surface_idx < collision_bsp->surfaces.size; surface_idx++)
			{
				size_t ref_index = NONE;
				auto surface = collision_bsp->surfaces[surface_idx];
				if (!LOG_CHECK(surface))
					return false;
				auto plane = collision_bsp->get_plane_by_ref(surface->plane);
				auto normal_angle = plane.normal.get_angle();
				if (!is_between(normal_angle.roll.as_degree(), 45.0, 135.0))
				{
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
			std::unordered_set<unsigned short> edges_used; // make a copy for ilterating
			for (auto &ilter : surface_sector_mapping)
			{
				auto surface_idx = ilter.first;
				std::unordered_set<unsigned short> edges_checked;
				std::function<void(unsigned short)> parse_edge;
				parse_edge = [&](unsigned short edge_idx)
				{
					// check if we already checked this edge for the current surface
					// stops it from locking up
					if (edges_checked.count(edge_idx) != 0)
						return;
					edges_checked.insert(edge_idx);

					auto *edge = collision_bsp->edges[edge_idx];
					if (!edge) //invalid edge
						return;

					if (edge->rightSurface != surface_idx && edge->leftSurface != surface_idx) // not part of current surface
						return;

					if (edges_used.count(edge_idx) == 0) // add new edge to be copied over
					{
						vertices_used.insert(edge->startVertex);
						vertices_used.insert(edge->endVertex);
						edge_link_mapping[edge_idx] = link_idx;
						link_idx++;
					}
					edges_used.insert(edge_idx);

					parse_edge(edge->reverseEdge);
					parse_edge(edge->forwardEdge);
				};
				auto surface = collision_bsp->surfaces[surface_idx];
				if (LOG_CHECK(surface))
					parse_edge(surface->firstEdge);
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
				link->hintIndex = NONE;

				link->vertex1 = get_idx(coll_to_path_vertex, edge->startVertex);
				link->vertex2 = get_idx(coll_to_path_vertex, edge->endVertex);
				link->forwardLink = get_idx(edge_link_mapping, edge->forwardEdge);
				link->reverseLink = get_idx(edge_link_mapping, edge->reverseEdge);

				link->rightSector = get_idx(surface_sector_mapping, edge->rightSurface);
				link->leftSector = get_idx(surface_sector_mapping, edge->leftSurface);
			}
			cout << "Done" << std::endl;

			cout << "Writing sectors...";
			tags::resize_block(&pathfinding->sectors, surface_sector_mapping.size());
			for (auto &ilter : surface_sector_mapping)
			{
				auto *sector = pathfinding->sectors[ilter.second];
				auto *surface = collision_bsp->surfaces[ilter.first];
				if (!LOG_CHECK(sector && surface))
					break;

				sector->hintIndex = NONE;
				sector->firstLinkdoNotSetManually = edge_link_mapping[surface->firstEdge];
				int *flags_ptr = reinterpret_cast<int*>(&sector->pathfindingSectorFlags);
				*flags_ptr = ((surface->flags & surface->Breakable) ? sector->SectorBreakable : 0) | 
					sector->SectorBspSource | sector->SectorWalkable | sector->Floor;
			}
			cout << "Done" << endl;

			cout << "Setting link flags...";
			for (auto &link : pathfinding->links)
			{
				int flags = link.SectorLinkFromCollisionEdge;
				auto sector_left = pathfinding->sectors[link.leftSector];
				auto sector_right = pathfinding->sectors[link.rightSector];
				if (sector_left && sector_right)
				{
					if (sector_left->is_walkable() && sector_right->is_walkable())
						flags |= link.SectorLinkBothSectorsWalkable;
				}
				int *flags_ptr = reinterpret_cast<int*>(&link.linkFlags);
				*flags_ptr = flags;
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

render_info pathfinding::get_render_info(scenario_structure_bsp_block *sbsp)
{
	render_info info_out;
	if (LOG_CHECK(sbsp))
	{
		auto pathfinding = sbsp->pathfindingData[0];
		auto collision_bsp = sbsp->collisionBSP[0];
		if (pathfinding)
		{
			for (const auto ref : pathfinding->refs)
			{
				auto sector = pathfinding->sectors[ref.nodeRefOrSectorRef];
				if (sector)
				{
					render_info::line_set line_set;
					std::unordered_set<size_t> links_checked;

					std::function<void(size_t link)> walk_link_recursive;
					walk_link_recursive = [&](size_t link) {
						if (links_checked.count(link) != 0)
							return;
						links_checked.insert(link); // mark as visted

						auto current_link = pathfinding->links[link];
						if (!current_link) // incalid
							return;

						if (current_link->leftSector != ref.nodeRefOrSectorRef && current_link->rightSector != ref.nodeRefOrSectorRef)
							return; // not part of sector

						line_set.lines.insert(link);

						walk_link_recursive(current_link->forwardLink);
						walk_link_recursive(current_link->reverseLink);
					};
					walk_link_recursive(sector->firstLinkdoNotSetManually);
					info_out.sector_lines[ref.nodeRefOrSectorRef] = line_set;
				}
			}
		}
	}
	return info_out;
}
