/**************************************************************************/
/*  cell_map.h                                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef CELL_MAP_H
#define CELL_MAP_H

/* The idea here is to have a parent for all cell_map related and then separate them into SceneMap, ShapeMap and CellMap */

// TODO - CellSet should also have these variants, CellSet, ShapeSet, SceneSet.

#include "scene/2d/node_2d.h"
#include "scene/gui/control.h"
#include "scene/resources/cell_set.h" // TODO - Make one

class CellSetAtlasSource;

class CellMap : public Node2D {
	GDCLASS(CellMap, Node2D);

public:
	class TerrainConstraint {
	private:
		const CellMap *cell_map;
		Vector2i base_cell_coords;
		int bit = -1;
		int terrain = -1;

		int priority = 1;

	public:
		bool operator<(const TerrainConstraint &p_other) const {
			if (base_cell_coords == p_other.base_cell_coords) {
				return bit < p_other.bit;
			}
			return base_cell_coords < p_other.base_cell_coords;
		}

		String to_string() const {
			return vformat("Constraint {pos:%s, bit:%d, terrain:%d, priority:%d}", base_cell_coords, bit, terrain, priority);
		}

		Vector2i get_base_cell_coords() const {
			return base_cell_coords;
		}

		bool is_center_bit() const {
			return bit == 0;
		}

		HashMap<Vector2i, CellSet::CellNeighbor> get_overlapping_coords_and_peering_bits() const;

		void set_terrain(int p_terrain) {
			terrain = p_terrain;
		}

		int get_terrain() const {
			return terrain;
		}

		void set_priority(int p_priority) {
			priority = p_priority;
		}

		int get_priority() const {
			return priority;
		}

		TerrainConstraint(const CellMap *p_cell_map, const Vector2i &p_position, int p_terrain); // For the center terrain bit
		TerrainConstraint(const CellMap *p_cell_map, const Vector2i &p_position, const CellSet::CellNeighbor &p_bit, int p_terrain); // For peering bits
		TerrainConstraint(){};
	};

private:
	friend class CellSetPlugin;

	// A compatibility enum to specify how is the data if formatted.
	enum DataFormat {
		FORMAT_1 = 0,
		FORMAT_2,
		FORMAT_3
	};
	mutable DataFormat format = FORMAT_3;

	static constexpr float FP_ADJUST = 0.00001;

	// Properties.
	Ref<CellSet> cell_set;

	// Updates.
	bool pending_update = false;

	// Rect.
	Rect2 rect_cache;
	bool rect_cache_dirty = true;
	Rect2i used_rect_cache;
	bool used_rect_cache_dirty = true;

	// CellMap layers.
	struct CellMapLayer {
		String name;
		bool enabled = true;
		Color modulate = Color(1, 1, 1, 1);
		bool y_sort_enabled = false;
		int y_sort_origin = 0;
		int z_index = 0;
		RID canvas_item;
		HashMap<Vector2i, CellMapCell> cell_map;
		RID navigation_map;
		bool uses_world_navigation_map = false;
	};
	LocalVector<CellMapLayer> layers;
	int selected_layer = -1;

	void _recreate_layer_internals(int p_layer);
	void _recreate_internals();

	void _clear_layer_internals(int p_layer);
	void _clear_internals();

	HashSet<Vector3i> instantiated_scenes;

	// Rect caching.
	void _recompute_rect_cache();

	// Per-system methods.
	virtual void _rendering_notification(int p_what);
	void _rendering_update_layer(int p_layer);
	void _rendering_cleanup_layer(int p_layer);

	// Terrains.
	CellSet::TerrainsPattern _get_best_terrain_pattern_for_constraints(int p_terrain_set, const Vector2i &p_position, const RBSet<TerrainConstraint> &p_constraints, CellSet::TerrainsPattern p_current_pattern);
	RBSet<TerrainConstraint> _get_terrain_constraints_from_added_pattern(const Vector2i &p_position, int p_terrain_set, CellSet::TerrainsPattern p_terrains_pattern) const;
	RBSet<TerrainConstraint> _get_terrain_constraints_from_painted_cells_list(int p_layer, const RBSet<Vector2i> &p_painted, int p_terrain_set, bool p_ignore_empty_terrains) const;

	// Set and get cells from data arrays.
	void _set_cell_data(int p_layer, const Vector<int> &p_data);
	Vector<int> _get_cell_data(int p_layer) const;

	void _cell_set_changed();
	bool _cell_set_changed_deferred_update_needed = false;
	void _cell_set_changed_deferred_update();

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void _notification(int p_what);
	static void _bind_methods();

public:
	static Vector2i transform_coords_layout(const Vector2i &p_coords, CellSet::CellOffsetAxis p_offset_axis, CellSet::CellLayout p_from_layout, CellSet::CellLayout p_to_layout);

	enum {
		INVALID_CELL = -1
	};

#ifdef TOOLS_ENABLED
	virtual Rect2 _edit_get_rect() const override;
#endif

	virtual void set_cellset(const Ref<CellSet> &p_cellset) = 0;
	Ref<CellSet> get_cellset() const;

	static void draw_cell(RID p_canvas_item, const Vector2 &p_position, const Ref<CellSet> p_cell_set, int p_atlas_source_id, const Vector2i &p_atlas_coords, int p_alternative_cell, int p_frame = -1, Color p_modulation = Color(1.0, 1.0, 1.0, 1.0), const CellData *p_cell_data_override = nullptr);

	// Layers management.
	int get_layers_count() const;
	void add_layer(int p_to_pos);
	void move_layer(int p_layer, int p_to_pos);
	void remove_layer(int p_layer);
	void set_layer_name(int p_layer, String p_name);
	String get_layer_name(int p_layer) const;
	void set_layer_enabled(int p_layer, bool p_visible);
	bool is_layer_enabled(int p_layer) const;
	void set_layer_modulate(int p_layer, Color p_modulate);
	Color get_layer_modulate(int p_layer) const;
	void set_layer_y_sort_enabled(int p_layer, bool p_enabled);
	bool is_layer_y_sort_enabled(int p_layer) const;
	void set_layer_y_sort_origin(int p_layer, int p_y_sort_origin);
	int get_layer_y_sort_origin(int p_layer) const;
	void set_layer_z_index(int p_layer, int p_z_index);
	int get_layer_z_index(int p_layer) const;
	void set_selected_layer(int p_layer_id); // For editor use.
	int get_selected_layer() const;

	// Cells accessors.
	void set_cell(int p_layer, const Vector2i &p_coords, int p_source_id = CellSet::INVALID_SOURCE, const Vector2i p_atlas_coords = CellSetSource::INVALID_ATLAS_COORDS, int p_alternative_cell = 0);
	void erase_cell(int p_layer, const Vector2i &p_coords);
	int get_cell_source_id(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	Vector2i get_cell_atlas_coords(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	int get_cell_alternative_cell(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	// Helper method to make accessing the data easier.
	CellData *get_cell_data(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;

	// Patterns.
	virtual Ref<TileMapPattern> get_pattern(int p_layer, TypedArray<Vector2i> p_coords_array) = 0;
	Vector2i map_pattern(const Vector2i &p_position_in_cellmap, const Vector2i &p_coords_in_pattern, Ref<CellMapPattern> p_pattern);
	void set_pattern(int p_layer, const Vector2i &p_position, const Ref<CellMapPattern> p_pattern);

	// Terrains.
	HashMap<Vector2i, CellSet::TerrainsPattern> terrain_fill_constraints(int p_layer, const Vector<Vector2i> &p_to_replace, int p_terrain_set, const RBSet<TerrainConstraint> &p_constraints); // Not exposed.
	HashMap<Vector2i, CellSet::TerrainsPattern> terrain_fill_connect(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true); // Not exposed.
	HashMap<Vector2i, CellSet::TerrainsPattern> terrain_fill_path(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true); // Not exposed.
	HashMap<Vector2i, CellSet::TerrainsPattern> terrain_fill_pattern(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, CellSet::TerrainsPattern p_terrains_pattern, bool p_ignore_empty_terrains = true); // Not exposed.

	void set_cells_terrain_connect(int p_layer, TypedArray<Vector2i> p_cells, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);
	void set_cells_terrain_path(int p_layer, TypedArray<Vector2i> p_path, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);

	// Not exposed to users
	CellMapCell get_cell(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	//---

	Vector2 map_to_local(const Vector2i &p_pos) const;
	Vector2i local_to_map(const Vector2 &p_pos) const;

	bool is_existing_neighbor(CellSet::CellNeighbor p_cell_neighbor) const;
	Vector2i get_neighbor_cell(const Vector2i &p_coords, CellSet::CellNeighbor p_cell_neighbor) const;

	TypedArray<Vector2i> get_used_cells(int p_layer) const;
	TypedArray<Vector2i> get_used_cells_by_id(int p_layer, int p_source_id = CellSet::INVALID_SOURCE, const Vector2i p_atlas_coords = CellSetSource::INVALID_ATLAS_COORDS, int p_alternative_cell = CellSetSource::INVALID_cell_ALTERNATIVE) const;
	Rect2i get_used_rect(); // Not const because of cache

	// Fixing and clearing methods.
	void fix_invalid_cells();

	// Clears cells from a given layer
	void clear_layer(int p_layer);
	void clear();

	// Force a CellMap update
	void force_update(int p_layer = -1);

	// Helpers?
	TypedArray<Vector2i> get_surrounding_cells(const Vector2i &coords);
	void draw_cells_outline(Control *p_control, const RBSet<Vector2i> &p_cells, Color p_color, Transform2D p_transform = Transform2D());

	// Virtual function to modify the CellData at runtime
	GDVIRTUAL2R(bool, _use_cell_data_runtime_update, int, Vector2i);
	GDVIRTUAL3(_cell_data_runtime_update, int, Vector2i, CellData *);

	// Configuration warnings.
	PackedStringArray get_configuration_warnings() const override;

	CellMap();
	~CellMap();
};

#endif // cell_map_H

