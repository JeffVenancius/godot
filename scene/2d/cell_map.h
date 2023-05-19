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

#include "scene/2d/node_2d.h"
#include "scene/gui/control.h"

struct Quadrant {
	struct CoordsWorldComparator {
		_ALWAYS_INLINE_ bool operator()(const Vector2i &p_a, const Vector2i &p_b) const {
			// We sort the cells by their local coords, as it is needed by rendering.
			if (p_a.y == p_b.y) {
				return p_a.x > p_b.x;
			} else {
				return p_a.y < p_b.y;
			}
		}
	};

	// Dirty list element.
	SelfList<Quadrant> dirty_list_element;

	// Quadrant layer and coords.
	int layer = -1;
	Vector2i coords;

	// CellMapCells
	RBSet<Vector2i> cells;
	// We need those two maps to sort by local position for rendering
	// This is kind of workaround, it would be better to sort the cells directly in the "cells" set instead.
	RBMap<Vector2i, Vector2i> map_to_local;
	RBMap<Vector2i, Vector2i, CoordsWorldComparator> local_to_map;

	// Debug.
	RID debug_canvas_item;

	// Rendering.
	List<RID> canvas_items;

	void operator=(const Quadrant &q) {
		layer = q.layer;
		coords = q.coords;
		debug_canvas_item = q.debug_canvas_item;
		canvas_items = q.canvas_items;
	}

	Quadrant(const Quadrant &q) :
		dirty_list_element(this) {
			layer = q.layer;
			coords = q.coords;
			debug_canvas_item = q.debug_canvas_item;
			canvas_items = q.canvas_items;
		}

	Quadrant() :
			dirty_list_element(this) {
	}
};

class CellMap : public Node2D {
	GDCLASS(CellMap, Node2D);

protected:
	// Rect.
	Rect2i used_rect_cache;
	bool rect_cache_dirty = true;
	Rect2 rect_cache;
	bool pending_update = false;
	int selected_layer = -1;
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
};

	Vector2i _coords_to_quadrant_coords(int p_layer, const Vector2i &p_coords) const;
	void _queue_update_dirty_quadrants();
	void _rendering_cleanup_layer(int p_layer);
	void _rendering_cleanup_quadrant(Quadrant *p_quadrant);
	bool _rendering_quadrant_order_dirty = false;

private:
	static constexpr float FP_ADJUST = 0.00001;

	// Properties.
	int quadrant_size;


	LocalVector<CellMapLayer> layers;

	// Quadrants and internals management.

	virtual void _make_all_quadrants_dirty() = 0;

	virtual void _recreate_layer_internals(int p_layer);

	virtual void _clear_layer_internals(int p_layer) = 0;

	// Rect caching.
	virtual void _recompute_rect_cache() = 0;

	// Per-system methods.
	virtual void _rendering_notification(int p_what) = 0;

// Set and get tiles from data arrays.
	virtual void _set_tile_data(int p_layer, const Vector<int> &p_data) = 0;
	virtual Vector<int> _get_tile_data(int p_layer) const = 0;
protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void _notification(int p_what);
	static void _bind_methods();

	void _recreate_internals();
	void _clear_internals();
	virtual void _rendering_update_layer(int p_layer) = 0;

public:
	enum {
		INVALID_CELL = -1
	};

#ifdef TOOLS_ENABLED
	virtual Rect2 _edit_get_rect() const override;
#endif

	bool can_set_get(const Vector<String> &p_components) const; // internal function.

	virtual void set_quadrant_size(int p_size);
	int get_quadrant_size() const;

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
	virtual void erase_cell(int p_layer, const Vector2i &p_coords) = 0;
	int get_cell_source_id(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;

	// Not exposed to users
	int get_effective_quadrant_size(int p_layer) const;
	//---

	virtual void set_y_sort_enabled(bool p_enable) override;

	virtual Vector2 map_to_local(const Vector2i &p_pos) const = 0;
	virtual Vector2i local_to_map(const Vector2 &p_pos) const = 0;

	virtual TypedArray<Vector2i> get_used_cells(int p_layer) const;
	virtual Rect2i get_used_rect() = 0; // Not const because of cache


	// Fixing and clearing methods.
	// Clears cells from a given layer
	virtual void clear_layer(int p_layer) = 0;
	virtual void clear() = 0;

	// Force a CellMap update
	void force_update(int p_layer = -1);

	CellMap();
	~CellMap();
};

#endif // CELLMAP_H
