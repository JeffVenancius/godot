/**************************************************************************/
/*  tile_map.h                                                            */
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

#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "scene/2d/cell_map.h"
#include "scene/resources/tile_set.h"

class TileSetAtlasSource;

struct TileMapQuadrant : public Quadrant {
	// Dirty list element.
	SelfList<TileMapQuadrant> dirty_list_element;

	// Physics.
	List<RID> bodies;
	HashMap<Vector2i, RID> occluders;

	// Navigation.
	HashMap<Vector2i, Vector<RID>> navigation_regions;

	// Scenes.
	HashMap<Vector2i, String> scenes;

	// Runtime TileData cache.
	HashMap<Vector2i, TileData *> runtime_tile_data_cache;

	void operator=(const TileMapQuadrant &q) {
		Quadrant::operator=(q);
		occluders = q.occluders;
		bodies = q.bodies;
		navigation_regions = q.navigation_regions;
	}

	TileMapQuadrant(const TileMapQuadrant &q) :
		dirty_list_element(this) {
			layer = q.layer;
			coords = q.coords;
			debug_canvas_item = q.debug_canvas_item;
			canvas_items = q.canvas_items;
			occluders = q.occluders;
			bodies = q.bodies;
			navigation_regions = q.navigation_regions;
	}

	TileMapQuadrant() :
			dirty_list_element(this) {
	}
};

class TileMap : public CellMap {
	GDCLASS(TileMap, CellMap);

public:
	class TerrainConstraint {
	private:
		const TileMap *tile_map;
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

		HashMap<Vector2i, TileSet::CellNeighbor> get_overlapping_coords_and_peering_bits() const;

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

		TerrainConstraint(const TileMap *p_tile_map, const Vector2i &p_position, int p_terrain); // For the center terrain bit
		TerrainConstraint(const TileMap *p_tile_map, const Vector2i &p_position, const TileSet::CellNeighbor &p_bit, int p_terrain); // For peering bits
		TerrainConstraint(){};
	};

	enum VisibilityMode {
		VISIBILITY_MODE_DEFAULT,
		VISIBILITY_MODE_FORCE_SHOW,
		VISIBILITY_MODE_FORCE_HIDE,
	};

private:
	friend class TileSetPlugin;

	// A compatibility enum to specify how is the data if formatted.
	enum DataFormat {
		FORMAT_1 = 0,
		FORMAT_2,
		FORMAT_3
	};
	mutable DataFormat format = FORMAT_3;

	static constexpr float FP_ADJUST = 0.00001;

	// Properties.
	Ref<TileSet> tile_set;
	int quadrant_size = 16;
	bool collision_animatable = false;
	VisibilityMode collision_visibility_mode = VISIBILITY_MODE_DEFAULT;
	VisibilityMode navigation_visibility_mode = VISIBILITY_MODE_DEFAULT;

	// TileMap layers.
	struct TileMapLayer : CellMapLayer {
		HashMap<Vector2i, TileMapCell> tile_map;
		HashMap<Vector2i, TileMapQuadrant> quadrant_map;
		SelfList<TileMapQuadrant>::List dirty_quadrant_list;
	};
	LocalVector<TileMapLayer> layers;

	// Mapping for RID to coords.
	HashMap<RID, Vector2i> bodies_coords;

	// Quadrants and internals management.
	HashMap<Vector2i, TileMapQuadrant>::Iterator _create_quadrant(int p_layer, const Vector2i &p_qk);
	void _make_quadrant_dirty(HashMap<Vector2i, TileMapQuadrant>::Iterator Q);
	virtual void _make_all_quadrants_dirty() override;
	void _update_dirty_quadrants();


	virtual void _recreate_layer_internals(int p_layer) override;
	virtual void _clear_layer_internals(int p_layer) override;

	void _erase_quadrant(HashMap<Vector2i, TileMapQuadrant>::Iterator Q);

	HashSet<Vector3i> instantiated_scenes;

	// Rect caching.
	virtual void _recompute_rect_cache() override;

	void _rendering_cleanup_quadrant(TileMapQuadrant *p_quadrant);
	// Per-system methods.
	virtual void _rendering_notification(int p_what) override;
	void _rendering_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list);
	void _rendering_create_quadrant(TileMapQuadrant *p_quadrant);
	void _rendering_draw_quadrant_debug(TileMapQuadrant *p_quadrant);

	Transform2D last_valid_transform;
	Transform2D new_transform;
	void _physics_notification(int p_what);
	void _physics_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list);
	void _physics_cleanup_quadrant(TileMapQuadrant *p_quadrant);
	void _physics_draw_quadrant_debug(TileMapQuadrant *p_quadrant);

	void _navigation_notification(int p_what);
	void _navigation_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list);
	void _navigation_cleanup_quadrant(TileMapQuadrant *p_quadrant);
	void _navigation_draw_quadrant_debug(TileMapQuadrant *p_quadrant);

	// Scenes
	void _scenes_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list);
	void _scenes_cleanup_quadrant(TileMapQuadrant *p_quadrant);
	void _scenes_draw_quadrant_debug(TileMapQuadrant *p_quadrant);

	// Terrains.
	TileSet::TerrainsPattern _get_best_terrain_pattern_for_constraints(int p_terrain_set, const Vector2i &p_position, const RBSet<TerrainConstraint> &p_constraints, TileSet::TerrainsPattern p_current_pattern);
	RBSet<TerrainConstraint> _get_terrain_constraints_from_added_pattern(const Vector2i &p_position, int p_terrain_set, TileSet::TerrainsPattern p_terrains_pattern) const;
	RBSet<TerrainConstraint> _get_terrain_constraints_from_painted_cells_list(int p_layer, const RBSet<Vector2i> &p_painted, int p_terrain_set, bool p_ignore_empty_terrains) const;

	// Set and get tiles from data arrays.
	virtual void _set_tile_data(int p_layer, const Vector<int> &p_data) override;
	virtual Vector<int> _get_tile_data(int p_layer) const override;

	void _build_runtime_update_tile_data(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list);

	void _tile_set_changed();
	bool _tile_set_changed_deferred_update_needed = false;
	void _tile_set_changed_deferred_update();

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void _notification(int p_what);
	static void _bind_methods();

	virtual void _rendering_update_layer(int p_layer) override;
public:
	static Vector2i transform_coords_layout(const Vector2i &p_coords, TileSet::TileOffsetAxis p_offset_axis, TileSet::TileLayout p_from_layout, TileSet::TileLayout p_to_layout);

	void set_tileset(const Ref<TileSet> &p_tileset);
	Ref<TileSet> get_tileset() const;

	virtual void set_quadrant_size(int p_size) override;
	int get_quadrant_size() const;

	static void draw_tile(RID p_canvas_item, const Vector2i &p_position, const Ref<TileSet> p_tile_set, int p_atlas_source_id, const Vector2i &p_atlas_coords, int p_alternative_tile, int p_frame = -1, Color p_modulation = Color(1.0, 1.0, 1.0, 1.0), const TileData *p_tile_data_override = nullptr);

	void set_collision_animatable(bool p_enabled);
	bool is_collision_animatable() const;

	// Debug visibility modes.
	void set_collision_visibility_mode(VisibilityMode p_show_collision);
	VisibilityMode get_collision_visibility_mode();

	void set_navigation_visibility_mode(VisibilityMode p_show_navigation);
	VisibilityMode get_navigation_visibility_mode();

	// Cells accessors.
	void set_cell(int p_layer, const Vector2i &p_coords, int p_source_id = TileSet::INVALID_SOURCE, const Vector2i p_atlas_coords = TileSetSource::INVALID_ATLAS_COORDS, int p_alternative_tile = 0);
	virtual void erase_cell(int p_layer, const Vector2i &p_coords) override;
	int get_cell_source_id(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	Vector2i get_cell_atlas_coords(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	int get_cell_alternative_tile(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	TileData *get_cell_tile_data(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;

	// Patterns.
	Ref<TileMapPattern> get_pattern(int p_layer, TypedArray<Vector2i> p_coords_array);
	Vector2i map_pattern(const Vector2i &p_position_in_tilemap, const Vector2i &p_coords_in_pattern, Ref<TileMapPattern> p_pattern);
	void set_pattern(int p_layer, const Vector2i &p_position, const Ref<TileMapPattern> p_pattern);

	// Terrains.
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_constraints(int p_layer, const Vector<Vector2i> &p_to_replace, int p_terrain_set, const RBSet<TerrainConstraint> &p_constraints); // Not exposed.
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_connect(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true); // Not exposed.
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_path(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true); // Not exposed.
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_pattern(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, TileSet::TerrainsPattern p_terrains_pattern, bool p_ignore_empty_terrains = true); // Not exposed.

	void set_cells_terrain_connect(int p_layer, TypedArray<Vector2i> p_cells, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);
	void set_cells_terrain_path(int p_layer, TypedArray<Vector2i> p_path, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);

	// Not exposed to users
	TileMapCell get_cell(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	HashMap<Vector2i, TileMapQuadrant> *get_quadrant_map(int p_layer);
	int get_effective_quadrant_size(int p_layer) const;
	//---

	bool is_existing_neighbor(TileSet::CellNeighbor p_cell_neighbor) const;
	Vector2i get_neighbor_cell(const Vector2i &p_coords, TileSet::CellNeighbor p_cell_neighbor) const;

	virtual Vector2 map_to_local(const Vector2i &p_pos) const override;
	virtual Vector2i local_to_map(const Vector2 &p_pos) const override;

	virtual TypedArray<Vector2i> get_used_cells(int p_layer) const override;
	TypedArray<Vector2i> get_used_cells_by_id(int p_layer, int p_source_id = TileSet::INVALID_SOURCE, const Vector2i p_atlas_coords = TileSetSource::INVALID_ATLAS_COORDS, int p_alternative_tile = TileSetSource::INVALID_TILE_ALTERNATIVE) const;
	virtual Rect2i get_used_rect() override; // Not const because of cache

	// Override some methods of the CanvasItem class to pass the changes to the quadrants CanvasItems
	virtual void set_light_mask(int p_light_mask) override;
	virtual void set_material(const Ref<Material> &p_material) override;
	virtual void set_use_parent_material(bool p_use_parent_material) override;
	virtual void set_texture_filter(CanvasItem::TextureFilter p_texture_filter) override;
	virtual void set_texture_repeat(CanvasItem::TextureRepeat p_texture_repeat) override;

	// For finding tiles from collision.
	Vector2i get_coords_for_body_rid(RID p_physics_body);

	// Fixing and clearing methods.
	void fix_invalid_tiles();
	virtual void clear_layer(int p_layer) override;
	virtual void clear() override;

	// Helpers?
	TypedArray<Vector2i> get_surrounding_cells(const Vector2i &coords);
	void draw_cells_outline(Control *p_control, const RBSet<Vector2i> &p_cells, Color p_color, Transform2D p_transform = Transform2D());

	// Virtual function to modify the TileData at runtime
	GDVIRTUAL2R(bool, _use_tile_data_runtime_update, int, Vector2i);
	GDVIRTUAL3(_tile_data_runtime_update, int, Vector2i, TileData *);

	// Configuration warnings.
	PackedStringArray get_configuration_warnings() const override;

	TileMap();
	~TileMap();
};

VARIANT_ENUM_CAST(TileMap::VisibilityMode);

#endif // TILE_MAP_H
