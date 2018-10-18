#include "pathfinding.h"
#include "../h2codez.h"
#include "../util/Logs.h"
#include "TagInterface.h"
#include <map>
#include <unordered_set>
#include <functional>
#include "../util/string_util.h"

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

class pathfinding_settings_parser
{
public:
	/* Reads in settings from a file */
	bool parse_file(std::ifstream &file)
	{
		enum mode
		{
			undefined,
			keep,
			remove
		};
		mode current_mode = undefined;
		while (file)
		{
			std::string line;
			std::getline(file, line);
			str_trim(line);
			line = tolower(line);

			if (line == get_keep_header())
			{
				current_mode = keep;
				continue;
			}
			if (line == get_remove_header())
			{
				current_mode = remove;
				continue;
			}

			if (current_mode == undefined)
				continue;

			unsigned short surface_idx = NONE;
			try {
				surface_idx = static_cast<unsigned short>(std::stoul(line));
			} catch(...) {
				continue;
			}
			if (LOG_CHECK(current_mode != undefined))
			{
				switch (current_mode)
				{
				case remove:
					surfaces_to_remove.insert(surface_idx);
				case keep:
					surfaces_to_keep.insert(surface_idx);
				}
			} else {
				return false;
			}
		}
		return !file.bad();
	}

	/* Reads in settings from a file */
	bool parse_file(const std::string &file_name)
	{
		std::ifstream file(file_name);
		if (file)
		{
			return parse_file(file);
		}
		return false;
	}

	bool write_to_file(std::ofstream &file)
	{
		if (file)
		{
			std::unordered_set<unsigned short> surfaces_kept;
			for (const auto surface : surfaces_to_keep)
			{
				if (surfaces_to_remove.count(surface) == 0)
					surfaces_kept.insert(surface);
			}

			if (surfaces_kept.size() > 0)
			{
				file << get_keep_header() << std::endl;
				for (const size_t surface : surfaces_kept)
					file << surface << std::endl;
				file << std::endl;
			}

			if (surfaces_to_remove.size() > 0)
			{
				file << get_remove_header() << std::endl;
				for (const size_t surface : surfaces_to_remove)
					file << surface << std::endl;
				file << std::endl;
			}
		}
		return !file.bad();
	}

	bool write_to_file(const std::string &file_name)
	{
		std::ofstream file(file_name);
		if (file)
		{
			return write_to_file(file);
		}
		return false;
	}

	pathfinding_settings_parser(const std::string &file_name)
	{
		parse_file(file_name);
	}
	pathfinding_settings_parser() {};

	/* Should include surface even if other checks fail */
	bool should_force_keep_surface(unsigned short surface)
	{
		return should_keep_surface(surface) && surfaces_to_keep.count(surface) > 0;
	}

	/* Surface **not** marked for removal */
	bool should_keep_surface(unsigned short surface)
	{
		return !should_remove_surface(surface);
	}

	/* Surface marked for removal */
	bool should_remove_surface(unsigned short surface)
	{
		return surfaces_to_remove.count(surface) > 0;
	}

	/* Mark a surface as removed, overrides keep_surface */
	void remove_surface(unsigned short surface)
	{
		surfaces_to_remove.insert(surface);
	}

	/* Mark a surface as included, overriden by remove_surface */
	void keep_surface(unsigned short surface)
	{
		surfaces_to_keep.insert(surface);
	}

private:

	const std::string &get_remove_header() const
	{
		const static std::string remove_header = "[remove]";
		return remove_header;
	}

	const std::string &get_keep_header() const
	{
		const static std::string keep_header = "[keep]";
		return keep_header;
	}

	std::unordered_set<unsigned short> surfaces_to_remove;
	std::unordered_set<unsigned short> surfaces_to_keep;
};

bool pathfinding::generate(datum sbsp_tag)
{
	auto *sbsp = tags::get_tag<scenario_structure_bsp_block>('sbsp', sbsp_tag);
	if (LOG_CHECK(sbsp->pathfindingData.size == 0)) // check the map has no pathfinding
	{
		tags::resize_block(&sbsp->pathfindingData, 1);
		if (LOG_CHECK(sbsp->pathfindingData.size == 1) && LOG_CHECK(sbsp->collisionBSP.size > 0))
		{
			auto pathfinding = sbsp->pathfindingData[0];
			auto collision_bsp = sbsp->collisionBSP[0];
			if (!LOG_CHECK(pathfinding && collision_bsp))
				return false;

			std::string import_settings_file = "tags\\" + tags::get_name(sbsp_tag) + "_import_setting.txt";
			pathfinding_settings_parser importer;
			pathfinding_settings_parser exporter;
			double surface_range = conf.getNumber("pathfinding_keep_angle_range", 45.0);
			std::cout << "Import settings file: \"" + import_settings_file <<  "\"" << std::endl;
			if (!importer.parse_file(import_settings_file))
				std::cout << "Import settings file not found or unreadable" << std::endl;

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
				if (importer.should_force_keep_surface(surface_idx)
					|| !is_between(normal_angle.roll.as_degree(), 90.0 - surface_range, 90 + surface_range) && !importer.should_remove_surface(surface_idx))
				{
					exporter.keep_surface(surface_idx);
					surface_sector_mapping[surface_idx] = sector_index;
					ref_index = sector_index;
					sector_index++;
				} else {
					exporter.remove_surface(surface_idx);
				}
				auto *ref = pathfinding->refs[surface_idx];
				ref->nodeRefOrSectorRef = ref_index;
			}

			std::cout << "Culling edges and vertices" << std::endl;

			// Ilterating over edges and calculating edges + points used
			std::unordered_set<unsigned short> vertices_used;
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
			unsigned short p_vertex_idx = 0;
			for (auto c_vertex_idx: vertices_used)
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

			std::string import_results = "tags\\" + tags::get_name(sbsp_tag) + "_import_results.txt";
			if (!exporter.write_to_file(import_results))
				std::cout << "Failed to write import results to file" << std::endl;
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
