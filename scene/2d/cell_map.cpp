/**************************************************************************/
/*  cell_map.cpp                                                          */
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

#include "cell_map.h"

#include "core/io/marshalls.h"
#include "scene/resources/world_2d.h"

Vector2i CellMap::_coords_to_quadrant_coords(int p_layer, const Vector2i &p_coords) const {
	int quad_size = get_effective_quadrant_size(p_layer);

	// Rounding down, instead of simply rounding towards zero (truncating)
	return Vector2i(
			p_coords.x > 0 ? p_coords.x / quad_size : (p_coords.x - (quad_size - 1)) / quad_size,
			p_coords.y > 0 ? p_coords.y / quad_size : (p_coords.y - (quad_size - 1)) / quad_size);
}

void CellMap::_queue_update_dirty_quadrants() {
	if (pending_update || !is_inside_tree()) {
		return;
	}
	pending_update = true;
	call_deferred(SNAME("_update_dirty_quadrants"));
}

void CellMap::_recreate_internals() {
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		_recreate_layer_internals(layer);
	}
}

void CellMap::_clear_internals() {
	// Clear quadrants.
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		_clear_layer_internals(layer);
	}
}

/////////////////////////////// Rendering //////////////////////////////////////

void CellMap::_rendering_update_layer(int p_layer) {
	RenderingServer *rs = RenderingServer::get_singleton();
	if (!layers[p_layer].canvas_item.is_valid()) {
		RID ci = rs->canvas_item_create();
		rs->canvas_item_set_parent(ci, get_canvas_item());

		rs->canvas_item_set_draw_index(ci, p_layer - (int64_t)0x80000000);

		layers[p_layer].canvas_item = ci;
	}
	RID &ci = layers[p_layer].canvas_item;
	rs->canvas_item_set_sort_children_by_y(ci, layers[p_layer].y_sort_enabled);
	rs->canvas_item_set_use_parent_material(ci, get_use_parent_material() || get_material().is_valid());
	rs->canvas_item_set_z_index(ci, layers[p_layer].z_index);
	rs->canvas_item_set_default_texture_filter(ci, RS::CanvasItemTextureFilter(get_texture_filter_in_tree()));
	rs->canvas_item_set_default_texture_repeat(ci, RS::CanvasItemTextureRepeat(get_texture_repeat_in_tree()));
	rs->canvas_item_set_light_mask(ci, get_light_mask());
}

void CellMap::_rendering_cleanup_layer(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	ERR_FAIL_NULL(RenderingServer::get_singleton());
	RenderingServer *rs = RenderingServer::get_singleton();
	if (layers[p_layer].canvas_item.is_valid()) {
		rs->free(layers[p_layer].canvas_item);
		layers[p_layer].canvas_item = RID();
	}
}

void CellMap::_rendering_cleanup_quadrant(Quadrant *p_quadrant) {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	// Free the canvas items.
	for (const RID &ci : p_quadrant->canvas_items) {
		RenderingServer::get_singleton()->free(ci);
	}
	p_quadrant->canvas_items.clear();
}

void CellMap::set_quadrant_size(int p_size) {
	quadrant_size = p_size;
	_clear_internals();
	_recreate_internals();
	emit_signal(SNAME("changed"));
}

int CellMap::get_quadrant_size() const {
	return quadrant_size;
}

int CellMap::get_layers_count() const {
	return layers.size();
}

void CellMap::add_layer(int p_to_pos) {
	if (p_to_pos < 0) {
		p_to_pos = layers.size() + p_to_pos + 1;
	}

	ERR_FAIL_INDEX(p_to_pos, (int)layers.size() + 1);

	// Must clear before adding the layer.
	_clear_internals();

	layers.insert(p_to_pos, CellMapLayer());
	_recreate_internals();
	notify_property_list_changed();

	emit_signal(SNAME("changed"));

	update_configuration_warnings();
}

void CellMap::move_layer(int p_layer, int p_to_pos) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	ERR_FAIL_INDEX(p_to_pos, (int)layers.size() + 1);

	// Clear before shuffling layers.
	_clear_internals();

	CellMapLayer tl = layers[p_layer];
	layers.insert(p_to_pos, tl);
	layers.remove_at(p_to_pos < p_layer ? p_layer + 1 : p_layer);
	_recreate_internals();
	notify_property_list_changed();

	if (selected_layer == p_layer) {
		selected_layer = p_to_pos < p_layer ? p_to_pos - 1 : p_to_pos;
	}

	emit_signal(SNAME("changed"));

	update_configuration_warnings();
}

void CellMap::remove_layer(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	// Clear before removing the layer.
	_clear_internals();

	layers.remove_at(p_layer);
	_recreate_internals();
	notify_property_list_changed();

	if (selected_layer >= p_layer) {
		selected_layer -= 1;
	}

	emit_signal(SNAME("changed"));

	update_configuration_warnings();
}

void CellMap::set_layer_name(int p_layer, String p_name) {
	if (p_layer < 0) {
		p_layer = layers.size() + p_layer;
	}
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	layers[p_layer].name = p_name;
	emit_signal(SNAME("changed"));
}

String CellMap::get_layer_name(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), String());
	return layers[p_layer].name;
}

void CellMap::set_layer_enabled(int p_layer, bool p_enabled) {
	if (p_layer < 0) {
		p_layer = layers.size() + p_layer;
	}
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	layers[p_layer].enabled = p_enabled;
	_clear_layer_internals(p_layer);
	_recreate_layer_internals(p_layer);
	emit_signal(SNAME("changed"));

	update_configuration_warnings();
}

bool CellMap::is_layer_enabled(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), false);
	return layers[p_layer].enabled;
}

void CellMap::set_layer_modulate(int p_layer, Color p_modulate) {
	if (p_layer < 0) {
		p_layer = layers.size() + p_layer;
	}
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	layers[p_layer].modulate = p_modulate;
	_rendering_update_layer(p_layer);
	emit_signal(SNAME("changed"));
}

Color CellMap::get_layer_modulate(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), Color());
	return layers[p_layer].modulate;
}

void CellMap::set_layer_y_sort_enabled(int p_layer, bool p_y_sort_enabled) {
	if (p_layer < 0) {
		p_layer = layers.size() + p_layer;
	}
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	layers[p_layer].y_sort_enabled = p_y_sort_enabled;
	_clear_layer_internals(p_layer);
	_recreate_layer_internals(p_layer);
	emit_signal(SNAME("changed"));

	update_configuration_warnings();
}

bool CellMap::is_layer_y_sort_enabled(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), false);
	return layers[p_layer].y_sort_enabled;
}

void CellMap::set_layer_y_sort_origin(int p_layer, int p_y_sort_origin) {
	if (p_layer < 0) {
		p_layer = layers.size() + p_layer;
	}
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	layers[p_layer].y_sort_origin = p_y_sort_origin;
	_clear_layer_internals(p_layer);
	_recreate_layer_internals(p_layer);
	emit_signal(SNAME("changed"));
}

int CellMap::get_layer_y_sort_origin(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), false);
	return layers[p_layer].y_sort_origin;
}

void CellMap::set_layer_z_index(int p_layer, int p_z_index) {
	if (p_layer < 0) {
		p_layer = layers.size() + p_layer;
	}
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	layers[p_layer].z_index = p_z_index;
	_rendering_update_layer(p_layer);
	emit_signal(SNAME("changed"));

	update_configuration_warnings();
}

int CellMap::get_layer_z_index(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), false);
	return layers[p_layer].z_index;
}

void CellMap::set_selected_layer(int p_layer_id) {
	ERR_FAIL_COND(p_layer_id < -1 || p_layer_id >= (int)layers.size());
	selected_layer = p_layer_id;
	emit_signal(SNAME("changed"));

	// Update the layers modulation.
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		_rendering_update_layer(layer);
	}
}

int CellMap::get_selected_layer() const {
	return selected_layer;
}

int CellMap::get_effective_quadrant_size(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), 1);

	// When using YSort, the quadrant size is reduced to 1 to have one CanvasItem per quadrant
	if (is_y_sort_enabled() && layers[p_layer].y_sort_enabled) {
		return 1;
	} else {
		return quadrant_size;
	}
}

void CellMap::set_y_sort_enabled(bool p_enable) {
	Node2D::set_y_sort_enabled(p_enable);
	_clear_internals();
	_recreate_internals();
	emit_signal(SNAME("changed"));
	update_configuration_warnings();
}

void CellMap::force_update(int p_layer) {
	if (p_layer >= 0) {
		ERR_FAIL_INDEX(p_layer, (int)layers.size());
		_clear_layer_internals(p_layer);
		_recreate_layer_internals(p_layer);
	} else {
		_clear_internals();
		_recreate_internals();
	}
}

#ifdef TOOLS_ENABLED
Rect2 CellMap::_edit_get_rect() const {
	// Return the visible rect of the cellmap
	const_cast<CellMap *>(this)->_recompute_rect_cache();
	return rect_cache;
}
#endif

bool CellMap::_set(const StringName &p_name, const Variant &p_value) {
	Vector<String> components = String(p_name).split("/", true, 2);
	if (can_set_get(components)) {
		int index = components[0].trim_prefix("layer_").to_int();
		if (index < 0) {
			return false;
		}

		if (components[1] == "name") {
			set_layer_name(index, p_value);
			return true;
		} else if (components[1] == "enabled") {
			set_layer_enabled(index, p_value);
			return true;
		} else if (components[1] == "modulate") {
			set_layer_modulate(index, p_value);
			return true;
		} else if (components[1] == "y_sort_enabled") {
			set_layer_y_sort_enabled(index, p_value);
			return true;
		} else if (components[1] == "y_sort_origin") {
			set_layer_y_sort_origin(index, p_value);
			return true;
		} else if (components[1] == "z_index") {
			set_layer_z_index(index, p_value);
			return true;
		} else {
			return false;
		}
	}
	return false;
}

bool CellMap::can_set_get(const Vector<String> &p_components) const {
	return p_components.size() == 2 && p_components[0].begins_with("layer_") && p_components[0].trim_prefix("layer_").is_valid_int();
}

bool CellMap::_get(const StringName &p_name, Variant &r_ret) const {
	Vector<String> components = String(p_name).split("/", true, 2);
	if (can_set_get(components)) {
		int index = components[0].trim_prefix("layer_").to_int();
		if (index < 0 || index >= (int)layers.size()) {
			return false;
		}

		if (components[1] == "name") {
			r_ret = get_layer_name(index);
			return true;
		} else if (components[1] == "enabled") {
			r_ret = is_layer_enabled(index);
			return true;
		} else if (components[1] == "modulate") {
			r_ret = get_layer_modulate(index);
			return true;
		} else if (components[1] == "y_sort_enabled") {
			r_ret = is_layer_y_sort_enabled(index);
			return true;
		} else if (components[1] == "y_sort_origin") {
			r_ret = get_layer_y_sort_origin(index);
			return true;
		} else if (components[1] == "z_index") {
			r_ret = get_layer_z_index(index);
			return true;
		} else if (components[1] == "tile_data") {
			r_ret = _get_tile_data(index);
			return true;
		} else {
			return false;
		}
	}
	return false;
}

void CellMap::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::NIL, "Layers", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_GROUP));
	for (unsigned int i = 0; i < layers.size(); i++) {
		p_list->push_back(PropertyInfo(Variant::STRING, vformat("layer_%d/name", i), PROPERTY_HINT_NONE));
		p_list->push_back(PropertyInfo(Variant::BOOL, vformat("layer_%d/enabled", i), PROPERTY_HINT_NONE));
		p_list->push_back(PropertyInfo(Variant::COLOR, vformat("layer_%d/modulate", i), PROPERTY_HINT_NONE));
		p_list->push_back(PropertyInfo(Variant::BOOL, vformat("layer_%d/y_sort_enabled", i), PROPERTY_HINT_NONE));
		p_list->push_back(PropertyInfo(Variant::INT, vformat("layer_%d/y_sort_origin", i), PROPERTY_HINT_NONE, "suffix:px"));
		p_list->push_back(PropertyInfo(Variant::INT, vformat("layer_%d/z_index", i), PROPERTY_HINT_NONE));
		p_list->push_back(PropertyInfo(Variant::OBJECT, vformat("layer_%d/cell_data", i), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR));
	}
}

void CellMap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_quadrant_size", "size"), &CellMap::set_quadrant_size);
	ClassDB::bind_method(D_METHOD("get_quadrant_size"), &CellMap::get_quadrant_size);

	ClassDB::bind_method(D_METHOD("get_layers_count"), &CellMap::get_layers_count);
	ClassDB::bind_method(D_METHOD("add_layer", "to_position"), &CellMap::add_layer);
	ClassDB::bind_method(D_METHOD("move_layer", "layer", "to_position"), &CellMap::move_layer);
	ClassDB::bind_method(D_METHOD("remove_layer", "layer"), &CellMap::remove_layer);

	ClassDB::bind_method(D_METHOD("set_layer_name", "layer", "name"), &CellMap::set_layer_name);
	ClassDB::bind_method(D_METHOD("get_layer_name", "layer"), &CellMap::get_layer_name);

	ClassDB::bind_method(D_METHOD("set_layer_enabled", "layer", "enabled"), &CellMap::set_layer_enabled);
	ClassDB::bind_method(D_METHOD("is_layer_enabled", "layer"), &CellMap::is_layer_enabled);

	ClassDB::bind_method(D_METHOD("set_layer_modulate", "layer", "modulate"), &CellMap::set_layer_modulate);
	ClassDB::bind_method(D_METHOD("get_layer_modulate", "layer"), &CellMap::get_layer_modulate);

	ClassDB::bind_method(D_METHOD("set_layer_y_sort_enabled", "layer", "y_sort_enabled"), &CellMap::set_layer_y_sort_enabled);
	ClassDB::bind_method(D_METHOD("is_layer_y_sort_enabled", "layer"), &CellMap::is_layer_y_sort_enabled);

	ClassDB::bind_method(D_METHOD("set_layer_y_sort_origin", "layer", "y_sort_origin"), &CellMap::set_layer_y_sort_origin);
	ClassDB::bind_method(D_METHOD("get_layer_y_sort_origin", "layer"), &CellMap::get_layer_y_sort_origin);

	ClassDB::bind_method(D_METHOD("set_layer_z_index", "layer", "z_index"), &CellMap::set_layer_z_index);
	ClassDB::bind_method(D_METHOD("get_layer_z_index", "layer"), &CellMap::get_layer_z_index);

	ClassDB::bind_method(D_METHOD("force_update", "layer"), &CellMap::force_update, DEFVAL(-1));

	ClassDB::bind_method(D_METHOD("get_used_rect"), &CellMap::get_used_rect);

	ClassDB::bind_method(D_METHOD("map_to_local", "map_position"), &CellMap::map_to_local);
	ClassDB::bind_method(D_METHOD("local_to_map", "local_position"), &CellMap::local_to_map);

	ADD_SIGNAL(MethodInfo("changed"));
}
