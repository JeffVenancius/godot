/**************************************************************************/
/*  tile_map.cpp                                                          */
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
#include "servers/navigation_server_2d.h"

#ifdef DEBUG_ENABLED
#include "servers/navigation_server_3d.h"
#endif // DEBUG_ENABLED

void TileMap::_notification(int p_what) {
	CellMap::_notification(p_what);

	// Transfers the notification to tileset plugins.
	if (tile_set.is_valid()) {
		_rendering_notification(p_what);
		_physics_notification(p_what);
		_navigation_notification(p_what);
	}
}

void TileMap::set_collision_animatable(bool p_enabled) {
	if (collision_animatable == p_enabled) {
		return;
	}
	collision_animatable = p_enabled;
	_clear_internals();
	set_notify_local_transform(p_enabled);
	set_physics_process_internal(p_enabled);
	_recreate_internals();
	emit_signal(SNAME("changed"));
}

bool TileMap::is_collision_animatable() const {
	return collision_animatable;
}

void TileMap::set_collision_visibility_mode(TileMap::VisibilityMode p_show_collision) {
	if (collision_visibility_mode == p_show_collision) {
		return;
	}
	collision_visibility_mode = p_show_collision;
	_clear_internals();
	_recreate_internals();
	emit_signal(SNAME("changed"));
}

TileMap::VisibilityMode TileMap::get_collision_visibility_mode() {
	return collision_visibility_mode;
}

void TileMap::set_navigation_visibility_mode(TileMap::VisibilityMode p_show_navigation) {
	if (navigation_visibility_mode == p_show_navigation) {
		return;
	}
	navigation_visibility_mode = p_show_navigation;
	_clear_internals();
	_recreate_internals();
	emit_signal(SNAME("changed"));
}

TileMap::VisibilityMode TileMap::get_navigation_visibility_mode() {
	return navigation_visibility_mode;
}

void TileMap::set_navigation_map(int p_layer, RID p_map) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	ERR_FAIL_COND_MSG(!is_inside_tree(), "A TileMap navigation map can only be changed while inside the SceneTree.");
	layers[p_layer].navigation_map = p_map;
	layers[p_layer].uses_world_navigation_map = p_map == get_world_2d()->get_navigation_map();
}

RID TileMap::get_navigation_map(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), RID());
	if (layers[p_layer].navigation_map.is_valid()) {
		return layers[p_layer].navigation_map;
	}
	return RID();
}

void TileMap::set_y_sort_enabled(bool p_enable) {
	if (is_y_sort_enabled() == p_enable) {
		return;
	}
	Node2D::set_y_sort_enabled(p_enable);
	_clear_internals();
	_recreate_internals();
	emit_signal(SNAME("changed"));
	update_configuration_warnings();
}

void TileMap::_update_dirty_quadrants() {
	if (!pending_update) {
		return;
	}
	if (!is_inside_tree() || !tile_set.is_valid()) {
		pending_update = false;
		return;
	}

	CellMap::_update_dirty_quadrants();

	// Find TileData that need a runtime modification.
	_build_runtime_update_tile_data(dirty_quadrant_list);

	// Call the update_dirty_quadrant method on plugins.
	_rendering_update_dirty_quadrants(dirty_quadrant_list);
	_physics_update_dirty_quadrants(dirty_quadrant_list);
	_navigation_update_dirty_quadrants(dirty_quadrant_list);

	// Redraw the debug canvas_items.
	RenderingServer *rs = RenderingServer::get_singleton();
	for (SelfList<TileMapQuadrant> *q = dirty_quadrant_list.first(); q; q = q->next()) {
		rs->canvas_item_clear(q->self()->debug_canvas_item);
		Transform2D xform;
		xform.set_origin(map_to_local(q->self()->coords * get_effective_quadrant_size(layer)));
		rs->canvas_item_set_transform(q->self()->debug_canvas_item, xform);

		_rendering_draw_quadrant_debug(q->self());
		_physics_draw_quadrant_debug(q->self());
		_navigation_draw_quadrant_debug(q->self());
	}

	// Clear the list
	while (dirty_quadrant_list.first()) {
		// Clear the runtime tile data.
		for (const KeyValue<Vector2i, TileData *> &kv : dirty_quadrant_list.first()->self()->runtime_tile_data_cache) {
			memdelete(kv.value);
		}

		dirty_quadrant_list.remove(dirty_quadrant_list.first());
	}
	pending_update = false;

	_recompute_rect_cache();
}

void TileMap::_erase_quadrant(HashMap<Vector2i, TileMapQuadrant>::Iterator Q) {
	// Remove a quadrant.
	TileMapQuadrant *q = &(Q->value);

	// Call the cleanup_quadrant method on plugins.
	if (tile_set.is_valid()) {
		_rendering_cleanup_quadrant(q);
		_physics_cleanup_quadrant(q);
		_navigation_cleanup_quadrant(q);
	}
	CellMap::_erase_quadrant(Q);
}

void TileMap::_clear_layer_internals(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	// Clear quadrants.
	while (layers[p_layer].quadrant_map.size()) {
		_erase_quadrant(layers[p_layer].quadrant_map.begin());
	}

	// Clear the layers internals.
	_rendering_cleanup_layer(p_layer);

	// Clear the layers internal navigation maps.
	_navigation_cleanup_layer(p_layer);

	// Clear the dirty quadrants list.
	while (layers[p_layer].dirty_quadrant_list.first()) {
		layers[p_layer].dirty_quadrant_list.remove(layers[p_layer].dirty_quadrant_list.first());
	}
}

void TileMap::_clear_internals() {
	// Clear quadrants.
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		_clear_layer_internals(layer);
	}
}

void TileMap::_recompute_rect_cache() {
	// Compute the displayed area of the tilemap.
#ifdef DEBUG_ENABLED

	if (!rect_cache_dirty) {
		return;
	}

	Rect2 r_total;
	bool first = true;
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		for (const KeyValue<Vector2i, TileMapQuadrant> &E : layers[layer].quadrant_map) {
			Rect2 r;
			r.position = map_to_local(E.key * get_effective_quadrant_size(layer));
			r.expand_to(map_to_local((E.key + Vector2i(1, 0)) * get_effective_quadrant_size(layer)));
			r.expand_to(map_to_local((E.key + Vector2i(1, 1)) * get_effective_quadrant_size(layer)));
			r.expand_to(map_to_local((E.key + Vector2i(0, 1)) * get_effective_quadrant_size(layer)));
			if (first) {
				r_total = r;
				first = false;
			} else {
				r_total = r_total.merge(r);
			}
		}
	}

	bool changed = rect_cache != r_total;

	rect_cache = r_total;

	item_rect_changed(changed);

	rect_cache_dirty = false;
#endif
}

void TileMap::set_layer_modulate(int p_layer, Color p_modulate) {
	CellMap::set_layer_modulate(p_layer, p_modulate);
	_rendering_update_layer(p_layer);
	emit_signal(SNAME("changed"));
}

/////////////////////////////// Rendering //////////////////////////////////////

void TileMap::_rendering_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_CANVAS: {
			bool node_visible = is_visible_in_tree();
			for (TileMapLayer &layer : layers) {
				for (KeyValue<Vector2i, TileMapQuadrant> &E_quadrant : layer.quadrant_map) {
					TileMapQuadrant &q = E_quadrant.value;
					for (const KeyValue<Vector2i, RID> &kv : q.occluders) {
						Transform2D xform;
						xform.set_origin(map_to_local(kv.key));
						RS::get_singleton()->canvas_light_occluder_attach_to_canvas(kv.value, get_canvas());
						RS::get_singleton()->canvas_light_occluder_set_transform(kv.value, get_global_transform() * xform);
						RS::get_singleton()->canvas_light_occluder_set_enabled(kv.value, node_visible);
					}
				}
			}
		} break;

		case NOTIFICATION_VISIBILITY_CHANGED: {
			bool node_visible = is_visible_in_tree();
			for (TileMapLayer &layer : layers) {
				for (KeyValue<Vector2i, TileMapQuadrant> &E_quadrant : layer.quadrant_map) {
					TileMapQuadrant &q = E_quadrant.value;

					// Update occluders transform.
					for (const KeyValue<Vector2, Vector2i> &E_cell : q.local_to_map) {
						Transform2D xform;
						xform.set_origin(E_cell.key);
						for (const KeyValue<Vector2i, RID> &kv : q.occluders) {
							RS::get_singleton()->canvas_light_occluder_set_enabled(kv.value, node_visible);
						}
					}
				}
			}
		} break;

		case NOTIFICATION_TRANSFORM_CHANGED: {
			if (!is_inside_tree()) {
				return;
			}
			for (TileMapLayer &layer : layers) {
				for (KeyValue<Vector2i, TileMapQuadrant> &E_quadrant : layer.quadrant_map) {
					TileMapQuadrant &q = E_quadrant.value;

					// Update occluders transform.
					for (const KeyValue<Vector2i, RID> &kv : q.occluders) {
						Transform2D xform;
						xform.set_origin(map_to_local(kv.key));
						RenderingServer::get_singleton()->canvas_light_occluder_set_transform(kv.value, get_global_transform() * xform);
					}
				}
			}
		} break;

		case NOTIFICATION_DRAW: {
			if (tile_set.is_valid()) {
				RenderingServer::get_singleton()->canvas_item_set_sort_children_by_y(get_canvas_item(), is_y_sort_enabled());
			}
		} break;

		case NOTIFICATION_EXIT_CANVAS: {
			for (TileMapLayer &layer : layers) {
				for (KeyValue<Vector2i, TileMapQuadrant> &E_quadrant : layer.quadrant_map) {
					TileMapQuadrant &q = E_quadrant.value;
					for (const KeyValue<Vector2i, RID> &kv : q.occluders) {
						RS::get_singleton()->canvas_light_occluder_attach_to_canvas(kv.value, RID());
					}
				}
			}
		} break;
	}
}

void TileMap::_navigation_update_layer(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	ERR_FAIL_NULL(NavigationServer2D::get_singleton());

	if (!layers[p_layer].navigation_map.is_valid()) {
		if (p_layer == 0 && is_inside_tree()) {
			// Use the default World2D navigation map for the first layer when empty.
			layers[p_layer].navigation_map = get_world_2d()->get_navigation_map();
			layers[p_layer].uses_world_navigation_map = true;
		} else {
			RID new_layer_map = NavigationServer2D::get_singleton()->map_create();
			NavigationServer2D::get_singleton()->map_set_active(new_layer_map, true);
			layers[p_layer].navigation_map = new_layer_map;
			layers[p_layer].uses_world_navigation_map = false;
		}
	}
}

void TileMap::_navigation_cleanup_layer(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	ERR_FAIL_NULL(NavigationServer2D::get_singleton());

	if (layers[p_layer].navigation_map.is_valid()) {
		if (layers[p_layer].uses_world_navigation_map) {
			// Do not delete the World2D default navigation map.
			return;
		}
		NavigationServer2D::get_singleton()->free(layers[p_layer].navigation_map);
		layers[p_layer].navigation_map = RID();
	}
}

void TileMap::_rendering_update_layer(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	RenderingServer *rs = RenderingServer::get_singleton();
	if (!layers[p_layer].canvas_item.is_valid()) {
		RID ci = rs->canvas_item_create();
		rs->canvas_item_set_parent(ci, get_canvas_item());

		/*Transform2D xform;
		xform.set_origin(Vector2(0, p_layer));
		rs->canvas_item_set_transform(ci, xform);*/
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

	Color layer_modulate = get_layer_modulate(p_layer);
	if (selected_layer >= 0 && p_layer != selected_layer) {
		int z1 = get_layer_z_index(p_layer);
		int z2 = get_layer_z_index(selected_layer);
		if (z1 < z2 || (z1 == z2 && p_layer < selected_layer)) {
			layer_modulate = layer_modulate.darkened(0.5);
		} else if (z1 > z2 || (z1 == z2 && p_layer > selected_layer)) {
			layer_modulate = layer_modulate.darkened(0.5);
			layer_modulate.a *= 0.3;
		}
	}
	rs->canvas_item_set_modulate(ci, layer_modulate);
}

void TileMap::_rendering_cleanup_layer(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	ERR_FAIL_NULL(RenderingServer::get_singleton());
	RenderingServer *rs = RenderingServer::get_singleton();
	if (layers[p_layer].canvas_item.is_valid()) {
		rs->free(layers[p_layer].canvas_item);
		layers[p_layer].canvas_item = RID();
	}
}

void TileMap::_rendering_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list) {
	ERR_FAIL_COND(!is_inside_tree());
	ERR_FAIL_COND(!tile_set.is_valid());

	bool node_visible = is_visible_in_tree();

	SelfList<TileMapQuadrant> *q_list_element = r_dirty_quadrant_list.first();
	while (q_list_element) {
		TileMapQuadrant &q = *q_list_element->self();

		RenderingServer *rs = RenderingServer::get_singleton();

		// Free the canvas items.
		for (const RID &ci : q.canvas_items) {
			rs->free(ci);
		}
		q.canvas_items.clear();

		// Free the occluders.
		for (const KeyValue<Vector2i, RID> &kv : q.occluders) {
			rs->free(kv.value);
		}
		q.occluders.clear();

		// Those allow to group cell per material or z-index.
		Ref<Material> prev_material;
		int prev_z_index = 0;
		RID prev_ci;

		// Iterate over the cells of the quadrant.
		for (const KeyValue<Vector2, Vector2i> &E_cell : q.local_to_map) {
			TileMapCell c = get_cell(q.layer, E_cell.value, true);

			TileSetSource *source;
			if (tile_set->has_source(c.source_id)) {
				source = *tile_set->get_source(c.source_id);

				if (!source->has_tile(c.get_atlas_coords()) || !source->has_alternative_tile(c.get_atlas_coords(), c.alternative_tile)) {
					continue;
				}

				TileSetAtlasSource *atlas_source = Object::cast_to<TileSetAtlasSource>(source);
				if (atlas_source) {
					// Get the tile data.
					const TileData *tile_data;
					if (q.runtime_tile_data_cache.has(E_cell.value)) {
						tile_data = q.runtime_tile_data_cache[E_cell.value];
					} else {
						tile_data = atlas_source->get_tile_data(c.get_atlas_coords(), c.alternative_tile);
					}

					Ref<Material> mat = tile_data->get_material();
					int tile_z_index = tile_data->get_z_index();

					// Quandrant pos.
					Vector2 tile_position = map_to_local(q.coords * get_effective_quadrant_size(q.layer));
					if (is_y_sort_enabled() && layers[q.layer].y_sort_enabled) {
						// When Y-sorting, the quandrant size is sure to be 1, we can thus offset the CanvasItem.
						tile_position.y += layers[q.layer].y_sort_origin + tile_data->get_y_sort_origin();
					}

					// --- CanvasItems ---
					// Create two canvas items, for rendering and debug.
					RID ci;

					// Check if the material or the z_index changed.
					if (prev_ci == RID() || prev_material != mat || prev_z_index != tile_z_index) {
						// If so, create a new CanvasItem.
						ci = rs->canvas_item_create();
						if (mat.is_valid()) {
							rs->canvas_item_set_material(ci, mat->get_rid());
						}
						rs->canvas_item_set_parent(ci, layers[q.layer].canvas_item);
						rs->canvas_item_set_use_parent_material(ci, get_use_parent_material() || get_material().is_valid());

						Transform2D xform;
						xform.set_origin(tile_position);
						rs->canvas_item_set_transform(ci, xform);

						rs->canvas_item_set_light_mask(ci, get_light_mask());
						rs->canvas_item_set_z_as_relative_to_parent(ci, true);
						rs->canvas_item_set_z_index(ci, tile_z_index);

						rs->canvas_item_set_default_texture_filter(ci, RS::CanvasItemTextureFilter(get_texture_filter_in_tree()));
						rs->canvas_item_set_default_texture_repeat(ci, RS::CanvasItemTextureRepeat(get_texture_repeat_in_tree()));

						q.canvas_items.push_back(ci);

						prev_ci = ci;
						prev_material = mat;
						prev_z_index = tile_z_index;

					} else {
						// Keep the same canvas_item to draw on.
						ci = prev_ci;
					}

					// Drawing the tile in the canvas item.
					draw_tile(ci, E_cell.key - tile_position, tile_set, c.source_id, c.get_atlas_coords(), c.alternative_tile, -1, get_self_modulate(), tile_data);

					// --- Occluders ---
					for (int i = 0; i < tile_set->get_occlusion_layers_count(); i++) {
						Transform2D xform;
						xform.set_origin(E_cell.key);
						if (tile_data->get_occluder(i).is_valid()) {
							RID occluder_id = rs->canvas_light_occluder_create();
							rs->canvas_light_occluder_set_enabled(occluder_id, node_visible);
							rs->canvas_light_occluder_set_transform(occluder_id, get_global_transform() * xform);
							rs->canvas_light_occluder_set_polygon(occluder_id, tile_data->get_occluder(i)->get_rid());
							rs->canvas_light_occluder_attach_to_canvas(occluder_id, get_canvas());
							rs->canvas_light_occluder_set_light_mask(occluder_id, tile_set->get_occlusion_layer_light_mask(i));
							q.occluders[E_cell.value] = occluder_id;
						}
					}
				}
			}
		}

		_rendering_quadrant_order_dirty = true;
		q_list_element = q_list_element->next();
	}

	// Reset the drawing indices
	if (_rendering_quadrant_order_dirty) {
		int index = -(int64_t)0x80000000; //always must be drawn below children.

		for (TileMapLayer &layer : layers) {
			// Sort the quadrants coords per local coordinates.
			RBMap<Vector2, Vector2i, TileMapQuadrant::CoordsWorldComparator> local_to_map;
			for (const KeyValue<Vector2i, TileMapQuadrant> &E : layer.quadrant_map) {
				local_to_map[map_to_local(E.key)] = E.key;
			}

			// Sort the quadrants.
			for (const KeyValue<Vector2, Vector2i> &E : local_to_map) {
				TileMapQuadrant &q = layer.quadrant_map[E.value];
				for (const RID &ci : q.canvas_items) {
					RS::get_singleton()->canvas_item_set_draw_index(ci, index++);
				}
			}
		}
		_rendering_quadrant_order_dirty = false;
	}
}

void TileMap::_rendering_create_quadrant(TileMapQuadrant *p_quadrant) {
	ERR_FAIL_COND(!tile_set.is_valid());

	_rendering_quadrant_order_dirty = true;
}

void TileMap::_rendering_cleanup_quadrant(TileMapQuadrant *p_quadrant) {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	// Free the canvas items.
	for (const RID &ci : p_quadrant->canvas_items) {
		RenderingServer::get_singleton()->free(ci);
	}
	p_quadrant->canvas_items.clear();

	// Free the occluders.
	for (const KeyValue<Vector2i, RID> &kv : p_quadrant->occluders) {
		RenderingServer::get_singleton()->free(kv.value);
	}
	p_quadrant->occluders.clear();
}

void TileMap::_rendering_draw_quadrant_debug(TileMapQuadrant *p_quadrant) {
	ERR_FAIL_COND(!tile_set.is_valid());

	if (!Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	// Draw a placeholder for tiles needing one.
	RenderingServer *rs = RenderingServer::get_singleton();
	Vector2 quadrant_pos = map_to_local(p_quadrant->coords * get_effective_quadrant_size(p_quadrant->layer));
	for (const Vector2i &E_cell : p_quadrant->cells) {
		const TileMapCell &c = get_cell(p_quadrant->layer, E_cell, true);

		TileSetSource *source;
		if (tile_set->has_source(c.source_id)) {
			source = *tile_set->get_source(c.source_id);

			if (!source->has_tile(c.get_atlas_coords()) || !source->has_alternative_tile(c.get_atlas_coords(), c.alternative_tile)) {
				continue;
			}

			TileSetAtlasSource *atlas_source = Object::cast_to<TileSetAtlasSource>(source);
			if (atlas_source) {
				Vector2i grid_size = atlas_source->get_atlas_grid_size();
				if (!atlas_source->get_runtime_texture().is_valid() || c.get_atlas_coords().x >= grid_size.x || c.get_atlas_coords().y >= grid_size.y) {
					// Generate a random color from the hashed values of the tiles.
					Array to_hash;
					to_hash.push_back(c.source_id);
					to_hash.push_back(c.get_atlas_coords());
					to_hash.push_back(c.alternative_tile);
					uint32_t hash = RandomPCG(to_hash.hash()).rand();

					Color color;
					color = color.from_hsv(
							(float)((hash >> 24) & 0xFF) / 256.0,
							Math::lerp(0.5, 1.0, (float)((hash >> 16) & 0xFF) / 256.0),
							Math::lerp(0.5, 1.0, (float)((hash >> 8) & 0xFF) / 256.0),
							0.8);

					// Draw a placeholder tile.
					Transform2D cell_to_quadrant;
					cell_to_quadrant.set_origin(map_to_local(E_cell) - quadrant_pos);
					rs->canvas_item_add_set_transform(p_quadrant->debug_canvas_item, cell_to_quadrant);
					rs->canvas_item_add_circle(p_quadrant->debug_canvas_item, Vector2(), MIN(tile_set->get_tile_size().x, tile_set->get_tile_size().y) / 4.0, color);
				}
			}
		}
	}
}

void TileMap::draw_tile(RID p_canvas_item, const Vector2 &p_position, const Ref<TileSet> p_tile_set, int p_atlas_source_id, const Vector2i &p_atlas_coords, int p_alternative_tile, int p_frame, Color p_modulation, const TileData *p_tile_data_override) {
	ERR_FAIL_COND(!p_tile_set.is_valid());
	ERR_FAIL_COND(!p_tile_set->has_source(p_atlas_source_id));
	ERR_FAIL_COND(!p_tile_set->get_source(p_atlas_source_id)->has_tile(p_atlas_coords));
	ERR_FAIL_COND(!p_tile_set->get_source(p_atlas_source_id)->has_alternative_tile(p_atlas_coords, p_alternative_tile));
	TileSetSource *source = *p_tile_set->get_source(p_atlas_source_id);
	TileSetAtlasSource *atlas_source = Object::cast_to<TileSetAtlasSource>(source);
	if (atlas_source) {
		// Check for the frame.
		if (p_frame >= 0) {
			ERR_FAIL_INDEX(p_frame, atlas_source->get_tile_animation_frames_count(p_atlas_coords));
		}

		// Get the texture.
		Ref<Texture2D> tex = atlas_source->get_runtime_texture();
		if (!tex.is_valid()) {
			return;
		}

		// Check if we are in the texture, return otherwise.
		Vector2i grid_size = atlas_source->get_atlas_grid_size();
		if (p_atlas_coords.x >= grid_size.x || p_atlas_coords.y >= grid_size.y) {
			return;
		}

		// Get tile data.
		const TileData *tile_data = p_tile_data_override ? p_tile_data_override : atlas_source->get_tile_data(p_atlas_coords, p_alternative_tile);

		// Get the tile modulation.
		Color modulate = tile_data->get_modulate() * p_modulation;

		// Compute the offset.
		Vector2 tile_offset = tile_data->get_texture_origin();

		// Get destination rect.
		Rect2 dest_rect;
		dest_rect.size = atlas_source->get_runtime_tile_texture_region(p_atlas_coords).size;
		dest_rect.size.x += FP_ADJUST;
		dest_rect.size.y += FP_ADJUST;

		bool transpose = tile_data->get_transpose();
		if (transpose) {
			dest_rect.position = (p_position - Vector2(dest_rect.size.y, dest_rect.size.x) / 2 - tile_offset);
		} else {
			dest_rect.position = (p_position - dest_rect.size / 2 - tile_offset);
		}

		if (tile_data->get_flip_h()) {
			dest_rect.size.x = -dest_rect.size.x;
		}

		if (tile_data->get_flip_v()) {
			dest_rect.size.y = -dest_rect.size.y;
		}

		// Draw the tile.
		if (p_frame >= 0) {
			Rect2i source_rect = atlas_source->get_runtime_tile_texture_region(p_atlas_coords, p_frame);
			tex->draw_rect_region(p_canvas_item, dest_rect, source_rect, modulate, transpose, p_tile_set->is_uv_clipping());
		} else if (atlas_source->get_tile_animation_frames_count(p_atlas_coords) == 1) {
			Rect2i source_rect = atlas_source->get_runtime_tile_texture_region(p_atlas_coords, 0);
			tex->draw_rect_region(p_canvas_item, dest_rect, source_rect, modulate, transpose, p_tile_set->is_uv_clipping());
		} else {
			real_t speed = atlas_source->get_tile_animation_speed(p_atlas_coords);
			real_t animation_duration = atlas_source->get_tile_animation_total_duration(p_atlas_coords) / speed;
			real_t time = 0.0;
			for (int frame = 0; frame < atlas_source->get_tile_animation_frames_count(p_atlas_coords); frame++) {
				real_t frame_duration = atlas_source->get_tile_animation_frame_duration(p_atlas_coords, frame) / speed;
				RenderingServer::get_singleton()->canvas_item_add_animation_slice(p_canvas_item, animation_duration, time, time + frame_duration, 0.0);

				Rect2i source_rect = atlas_source->get_runtime_tile_texture_region(p_atlas_coords, frame);
				tex->draw_rect_region(p_canvas_item, dest_rect, source_rect, modulate, transpose, p_tile_set->is_uv_clipping());

				time += frame_duration;
			}
			RenderingServer::get_singleton()->canvas_item_add_animation_slice(p_canvas_item, 1.0, 0.0, 1.0, 0.0);
		}
	}
}

/////////////////////////////// Physics //////////////////////////////////////

void TileMap::_physics_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			bool in_editor = false;
#ifdef TOOLS_ENABLED
			in_editor = Engine::get_singleton()->is_editor_hint();
#endif
			if (is_inside_tree() && collision_animatable && !in_editor) {
				// Update transform on the physics tick when in animatable mode.
				last_valid_transform = new_transform;
				set_notify_local_transform(false);
				set_global_transform(new_transform);
				set_notify_local_transform(true);
			}
		} break;

		case NOTIFICATION_TRANSFORM_CHANGED: {
			bool in_editor = false;
#ifdef TOOLS_ENABLED
			in_editor = Engine::get_singleton()->is_editor_hint();
#endif
			if (is_inside_tree() && (!collision_animatable || in_editor)) {
				// Update the new transform directly if we are not in animatable mode.
				Transform2D gl_transform = get_global_transform();
				for (TileMapLayer &layer : layers) {
					for (KeyValue<Vector2i, TileMapQuadrant> &E : layer.quadrant_map) {
						TileMapQuadrant &q = E.value;

						for (RID body : q.bodies) {
							Transform2D xform;
							xform.set_origin(map_to_local(bodies_coords[body]));
							xform = gl_transform * xform;
							PhysicsServer2D::get_singleton()->body_set_state(body, PhysicsServer2D::BODY_STATE_TRANSFORM, xform);
						}
					}
				}
			}
		} break;

		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			bool in_editor = false;
#ifdef TOOLS_ENABLED
			in_editor = Engine::get_singleton()->is_editor_hint();
#endif
			if (is_inside_tree() && !in_editor && collision_animatable) {
				// Only active when animatable. Send the new transform to the physics...
				new_transform = get_global_transform();
				for (TileMapLayer &layer : layers) {
					for (KeyValue<Vector2i, TileMapQuadrant> &E : layer.quadrant_map) {
						TileMapQuadrant &q = E.value;

						for (RID body : q.bodies) {
							Transform2D xform;
							xform.set_origin(map_to_local(bodies_coords[body]));
							xform = new_transform * xform;

							PhysicsServer2D::get_singleton()->body_set_state(body, PhysicsServer2D::BODY_STATE_TRANSFORM, xform);
						}
					}
				}

				// ... but then revert changes.
				set_notify_local_transform(false);
				set_global_transform(last_valid_transform);
				set_notify_local_transform(true);
			}
		} break;
	}
}

void TileMap::_physics_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list) {
	ERR_FAIL_COND(!is_inside_tree());
	ERR_FAIL_COND(!tile_set.is_valid());

	Transform2D gl_transform = get_global_transform();
	last_valid_transform = gl_transform;
	new_transform = gl_transform;
	PhysicsServer2D *ps = PhysicsServer2D::get_singleton();
	RID space = get_world_2d()->get_space();

	SelfList<TileMapQuadrant> *q_list_element = r_dirty_quadrant_list.first();
	while (q_list_element) {
		TileMapQuadrant &q = *q_list_element->self();

		// Clear bodies.
		for (RID body : q.bodies) {
			bodies_coords.erase(body);
			ps->free(body);
		}
		q.bodies.clear();

		// Recreate bodies and shapes.
		for (const Vector2i &E_cell : q.cells) {
			TileMapCell c = get_cell(q.layer, E_cell, true);

			TileSetSource *source;
			if (tile_set->has_source(c.source_id)) {
				source = *tile_set->get_source(c.source_id);

				if (!source->has_tile(c.get_atlas_coords()) || !source->has_alternative_tile(c.get_atlas_coords(), c.alternative_tile)) {
					continue;
				}

				TileSetAtlasSource *atlas_source = Object::cast_to<TileSetAtlasSource>(source);
				if (atlas_source) {
					const TileData *tile_data;
					if (q.runtime_tile_data_cache.has(E_cell)) {
						tile_data = q.runtime_tile_data_cache[E_cell];
					} else {
						tile_data = atlas_source->get_tile_data(c.get_atlas_coords(), c.alternative_tile);
					}
					for (int tile_set_physics_layer = 0; tile_set_physics_layer < tile_set->get_physics_layers_count(); tile_set_physics_layer++) {
						Ref<PhysicsMaterial> physics_material = tile_set->get_physics_layer_physics_material(tile_set_physics_layer);
						uint32_t physics_layer = tile_set->get_physics_layer_collision_layer(tile_set_physics_layer);
						uint32_t physics_mask = tile_set->get_physics_layer_collision_mask(tile_set_physics_layer);

						// Create the body.
						RID body = ps->body_create();
						bodies_coords[body] = E_cell;
						bodies_layers[body] = q.layer;
						ps->body_set_mode(body, collision_animatable ? PhysicsServer2D::BODY_MODE_KINEMATIC : PhysicsServer2D::BODY_MODE_STATIC);
						ps->body_set_space(body, space);

						Transform2D xform;
						xform.set_origin(map_to_local(E_cell));
						xform = gl_transform * xform;
						ps->body_set_state(body, PhysicsServer2D::BODY_STATE_TRANSFORM, xform);

						ps->body_attach_object_instance_id(body, get_instance_id());
						ps->body_set_collision_layer(body, physics_layer);
						ps->body_set_collision_mask(body, physics_mask);
						ps->body_set_pickable(body, false);
						ps->body_set_state(body, PhysicsServer2D::BODY_STATE_LINEAR_VELOCITY, tile_data->get_constant_linear_velocity(tile_set_physics_layer));
						ps->body_set_state(body, PhysicsServer2D::BODY_STATE_ANGULAR_VELOCITY, tile_data->get_constant_angular_velocity(tile_set_physics_layer));

						if (!physics_material.is_valid()) {
							ps->body_set_param(body, PhysicsServer2D::BODY_PARAM_BOUNCE, 0);
							ps->body_set_param(body, PhysicsServer2D::BODY_PARAM_FRICTION, 1);
						} else {
							ps->body_set_param(body, PhysicsServer2D::BODY_PARAM_BOUNCE, physics_material->computed_bounce());
							ps->body_set_param(body, PhysicsServer2D::BODY_PARAM_FRICTION, physics_material->computed_friction());
						}

						q.bodies.push_back(body);

						// Add the shapes to the body.
						int body_shape_index = 0;
						for (int polygon_index = 0; polygon_index < tile_data->get_collision_polygons_count(tile_set_physics_layer); polygon_index++) {
							// Iterate over the polygons.
							bool one_way_collision = tile_data->is_collision_polygon_one_way(tile_set_physics_layer, polygon_index);
							float one_way_collision_margin = tile_data->get_collision_polygon_one_way_margin(tile_set_physics_layer, polygon_index);
							int shapes_count = tile_data->get_collision_polygon_shapes_count(tile_set_physics_layer, polygon_index);
							for (int shape_index = 0; shape_index < shapes_count; shape_index++) {
								// Add decomposed convex shapes.
								Ref<ConvexPolygonShape2D> shape = tile_data->get_collision_polygon_shape(tile_set_physics_layer, polygon_index, shape_index);
								ps->body_add_shape(body, shape->get_rid());
								ps->body_set_shape_as_one_way_collision(body, body_shape_index, one_way_collision, one_way_collision_margin);

								body_shape_index++;
							}
						}
					}
				}
			}
		}

		q_list_element = q_list_element->next();
	}
}

void TileMap::_physics_cleanup_quadrant(TileMapQuadrant *p_quadrant) {
	// Remove a quadrant.
	ERR_FAIL_NULL(PhysicsServer2D::get_singleton());
	for (RID body : p_quadrant->bodies) {
		bodies_coords.erase(body);
		bodies_layers.erase(body);
		PhysicsServer2D::get_singleton()->free(body);
	}
	p_quadrant->bodies.clear();
}

void TileMap::_physics_draw_quadrant_debug(TileMapQuadrant *p_quadrant) {
	// Draw the debug collision shapes.
	ERR_FAIL_COND(!tile_set.is_valid());

	if (!get_tree()) {
		return;
	}

	bool show_collision = false;
	switch (collision_visibility_mode) {
		case TileMap::VISIBILITY_MODE_DEFAULT:
			show_collision = !Engine::get_singleton()->is_editor_hint() && (get_tree() && get_tree()->is_debugging_collisions_hint());
			break;
		case TileMap::VISIBILITY_MODE_FORCE_HIDE:
			show_collision = false;
			break;
		case TileMap::VISIBILITY_MODE_FORCE_SHOW:
			show_collision = true;
			break;
	}
	if (!show_collision) {
		return;
	}

	RenderingServer *rs = RenderingServer::get_singleton();
	PhysicsServer2D *ps = PhysicsServer2D::get_singleton();

	Color debug_collision_color = get_tree()->get_debug_collisions_color();
	Vector<Color> color;
	color.push_back(debug_collision_color);

	Vector2 quadrant_pos = map_to_local(p_quadrant->coords * get_effective_quadrant_size(p_quadrant->layer));
	Transform2D quadrant_to_local;
	quadrant_to_local.set_origin(quadrant_pos);
	Transform2D global_to_quadrant = (get_global_transform() * quadrant_to_local).affine_inverse();

	for (RID body : p_quadrant->bodies) {
		Transform2D body_to_quadrant = global_to_quadrant * Transform2D(ps->body_get_state(body, PhysicsServer2D::BODY_STATE_TRANSFORM));
		rs->canvas_item_add_set_transform(p_quadrant->debug_canvas_item, body_to_quadrant);
		for (int shape_index = 0; shape_index < ps->body_get_shape_count(body); shape_index++) {
			const RID &shape = ps->body_get_shape(body, shape_index);
			PhysicsServer2D::ShapeType type = ps->shape_get_type(shape);
			if (type == PhysicsServer2D::SHAPE_CONVEX_POLYGON) {
				Vector<Vector2> polygon = ps->shape_get_data(shape);
				rs->canvas_item_add_polygon(p_quadrant->debug_canvas_item, polygon, color);
			} else {
				WARN_PRINT("Wrong shape type for a tile, should be SHAPE_CONVEX_POLYGON.");
			}
		}
		rs->canvas_item_add_set_transform(p_quadrant->debug_canvas_item, Transform2D());
	}
};

/////////////////////////////// Navigation //////////////////////////////////////

void TileMap::_navigation_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_TRANSFORM_CHANGED: {
			if (is_inside_tree()) {
				for (TileMapLayer &layer : layers) {
					Transform2D tilemap_xform = get_global_transform();
					for (KeyValue<Vector2i, TileMapQuadrant> &E_quadrant : layer.quadrant_map) {
						TileMapQuadrant &q = E_quadrant.value;
						for (const KeyValue<Vector2i, Vector<RID>> &E_region : q.navigation_regions) {
							for (const RID &region : E_region.value) {
								if (!region.is_valid()) {
									continue;
								}
								Transform2D tile_transform;
								tile_transform.set_origin(map_to_local(E_region.key));
								NavigationServer2D::get_singleton()->region_set_transform(region, tilemap_xform * tile_transform);
							}
						}
					}
				}
			}
		} break;
	}
}

void TileMap::_navigation_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list) {
	ERR_FAIL_COND(!is_inside_tree());
	ERR_FAIL_COND(!tile_set.is_valid());

	Transform2D tilemap_xform = get_global_transform();
	SelfList<TileMapQuadrant> *q_list_element = r_dirty_quadrant_list.first();
	while (q_list_element) {
		TileMapQuadrant &q = *q_list_element->self();

		// Clear navigation shapes in the quadrant.
		for (const KeyValue<Vector2i, Vector<RID>> &E : q.navigation_regions) {
			for (int i = 0; i < E.value.size(); i++) {
				RID region = E.value[i];
				if (!region.is_valid()) {
					continue;
				}
				NavigationServer2D::get_singleton()->region_set_map(region, RID());
			}
		}
		q.navigation_regions.clear();

		// Get the navigation polygons and create regions.
		for (const Vector2i &E_cell : q.cells) {
			TileMapCell c = get_cell(q.layer, E_cell, true);

			TileSetSource *source;
			if (tile_set->has_source(c.source_id)) {
				source = *tile_set->get_source(c.source_id);

				if (!source->has_tile(c.get_atlas_coords()) || !source->has_alternative_tile(c.get_atlas_coords(), c.alternative_tile)) {
					continue;
				}

				TileSetAtlasSource *atlas_source = Object::cast_to<TileSetAtlasSource>(source);
				if (atlas_source) {
					const TileData *tile_data;
					if (q.runtime_tile_data_cache.has(E_cell)) {
						tile_data = q.runtime_tile_data_cache[E_cell];
					} else {
						tile_data = atlas_source->get_tile_data(c.get_atlas_coords(), c.alternative_tile);
					}
					q.navigation_regions[E_cell].resize(tile_set->get_navigation_layers_count());

					for (int layer_index = 0; layer_index < tile_set->get_navigation_layers_count(); layer_index++) {
						if (layer_index >= (int)layers.size() || !layers[layer_index].navigation_map.is_valid()) {
							continue;
						}
						Ref<NavigationPolygon> navigation_polygon;
						navigation_polygon = tile_data->get_navigation_polygon(layer_index);

						if (navigation_polygon.is_valid()) {
							Transform2D tile_transform;
							tile_transform.set_origin(map_to_local(E_cell));

							RID region = NavigationServer2D::get_singleton()->region_create();
							NavigationServer2D::get_singleton()->region_set_owner_id(region, get_instance_id());
							NavigationServer2D::get_singleton()->region_set_map(region, layers[layer_index].navigation_map);
							NavigationServer2D::get_singleton()->region_set_transform(region, tilemap_xform * tile_transform);
							NavigationServer2D::get_singleton()->region_set_navigation_layers(region, tile_set->get_navigation_layer_layers(layer_index));
							NavigationServer2D::get_singleton()->region_set_navigation_polygon(region, navigation_polygon);
							q.navigation_regions[E_cell].write[layer_index] = region;
						}
					}
				}
			}
		}

		q_list_element = q_list_element->next();
	}
}

void TileMap::_navigation_cleanup_quadrant(TileMapQuadrant *p_quadrant) {
	// Clear navigation shapes in the quadrant.
	ERR_FAIL_NULL(NavigationServer2D::get_singleton());
	for (const KeyValue<Vector2i, Vector<RID>> &E : p_quadrant->navigation_regions) {
		for (int i = 0; i < E.value.size(); i++) {
			RID region = E.value[i];
			if (!region.is_valid()) {
				continue;
			}
			NavigationServer2D::get_singleton()->free(region);
		}
	}
	p_quadrant->navigation_regions.clear();
}

void TileMap::_navigation_draw_quadrant_debug(TileMapQuadrant *p_quadrant) {
	// Draw the debug collision shapes.
	ERR_FAIL_COND(!tile_set.is_valid());

	if (!get_tree()) {
		return;
	}

	bool show_navigation = false;
	switch (navigation_visibility_mode) {
		case TileMap::VISIBILITY_MODE_DEFAULT:
			show_navigation = !Engine::get_singleton()->is_editor_hint() && (get_tree() && get_tree()->is_debugging_navigation_hint());
			break;
		case TileMap::VISIBILITY_MODE_FORCE_HIDE:
			show_navigation = false;
			break;
		case TileMap::VISIBILITY_MODE_FORCE_SHOW:
			show_navigation = true;
			break;
	}
	if (!show_navigation) {
		return;
	}

#ifdef DEBUG_ENABLED
	RenderingServer *rs = RenderingServer::get_singleton();
	const NavigationServer2D *ns2d = NavigationServer2D::get_singleton();

	bool enabled_geometry_face_random_color = ns2d->get_debug_navigation_enable_geometry_face_random_color();
	bool enabled_edge_lines = ns2d->get_debug_navigation_enable_edge_lines();

	Color debug_face_color = ns2d->get_debug_navigation_geometry_face_color();
	Color debug_edge_color = ns2d->get_debug_navigation_geometry_edge_color();

	RandomPCG rand;

	Vector2 quadrant_pos = map_to_local(p_quadrant->coords * get_effective_quadrant_size(p_quadrant->layer));

	for (const Vector2i &E_cell : p_quadrant->cells) {
		TileMapCell c = get_cell(p_quadrant->layer, E_cell, true);

		TileSetSource *source;
		if (tile_set->has_source(c.source_id)) {
			source = *tile_set->get_source(c.source_id);

			if (!source->has_tile(c.get_atlas_coords()) || !source->has_alternative_tile(c.get_atlas_coords(), c.alternative_tile)) {
				continue;
			}

			TileSetAtlasSource *atlas_source = Object::cast_to<TileSetAtlasSource>(source);
			if (atlas_source) {
				const TileData *tile_data;
				if (p_quadrant->runtime_tile_data_cache.has(E_cell)) {
					tile_data = p_quadrant->runtime_tile_data_cache[E_cell];
				} else {
					tile_data = atlas_source->get_tile_data(c.get_atlas_coords(), c.alternative_tile);
				}

				Transform2D cell_to_quadrant;
				cell_to_quadrant.set_origin(map_to_local(E_cell) - quadrant_pos);
				rs->canvas_item_add_set_transform(p_quadrant->debug_canvas_item, cell_to_quadrant);

				for (int layer_index = 0; layer_index < tile_set->get_navigation_layers_count(); layer_index++) {
					Ref<NavigationPolygon> navigation_polygon = tile_data->get_navigation_polygon(layer_index);
					if (navigation_polygon.is_valid()) {
						Vector<Vector2> navigation_polygon_vertices = navigation_polygon->get_vertices();
						if (navigation_polygon_vertices.size() < 3) {
							continue;
						}

						for (int i = 0; i < navigation_polygon->get_polygon_count(); i++) {
							// An array of vertices for this polygon.
							Vector<int> polygon = navigation_polygon->get_polygon(i);
							Vector<Vector2> debug_polygon_vertices;
							debug_polygon_vertices.resize(polygon.size());
							for (int j = 0; j < polygon.size(); j++) {
								ERR_FAIL_INDEX(polygon[j], navigation_polygon_vertices.size());
								debug_polygon_vertices.write[j] = navigation_polygon_vertices[polygon[j]];
							}

							// Generate the polygon color, slightly randomly modified from the settings one.
							Color random_variation_color = debug_face_color;
							if (enabled_geometry_face_random_color) {
								random_variation_color.set_hsv(
										debug_face_color.get_h() + rand.random(-1.0, 1.0) * 0.1,
										debug_face_color.get_s(),
										debug_face_color.get_v() + rand.random(-1.0, 1.0) * 0.2);
							}
							random_variation_color.a = debug_face_color.a;

							Vector<Color> debug_face_colors;
							debug_face_colors.push_back(random_variation_color);
							rs->canvas_item_add_polygon(p_quadrant->debug_canvas_item, debug_polygon_vertices, debug_face_colors);

							if (enabled_edge_lines) {
								Vector<Color> debug_edge_colors;
								debug_edge_colors.push_back(debug_edge_color);
								debug_polygon_vertices.push_back(debug_polygon_vertices[0]); // Add first again for closing polyline.
								rs->canvas_item_add_polyline(p_quadrant->debug_canvas_item, debug_polygon_vertices, debug_edge_colors);
							}
						}
					}
				}
			}
		}
	}
#endif // DEBUG_ENABLED
}

void TileMap::set_cell(int p_layer, const Vector2i &p_coords, int p_source_id, const Vector2i p_atlas_coords, int p_alternative_tile) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	// Set the current cell tile (using integer position).
	HashMap<Vector2i, TileMapCell> &tile_map = layers[p_layer].tile_map;
	Vector2i pk(p_coords);
	HashMap<Vector2i, TileMapCell>::Iterator E = tile_map.find(pk);

	int source_id = p_source_id;
	Vector2i atlas_coords = p_atlas_coords;
	int alternative_tile = p_alternative_tile;

	if ((source_id == TileSet::INVALID_SOURCE || atlas_coords == TileSetSource::INVALID_ATLAS_COORDS || alternative_tile == TileSetSource::INVALID_TILE_ALTERNATIVE) &&
			(source_id != TileSet::INVALID_SOURCE || atlas_coords != TileSetSource::INVALID_ATLAS_COORDS || alternative_tile != TileSetSource::INVALID_TILE_ALTERNATIVE)) {
		source_id = TileSet::INVALID_SOURCE;
		atlas_coords = TileSetSource::INVALID_ATLAS_COORDS;
		alternative_tile = TileSetSource::INVALID_TILE_ALTERNATIVE;
	}

	if (!E && source_id == TileSet::INVALID_SOURCE) {
		return; // Nothing to do, the tile is already empty.
	}

	// Get the quadrant
	Vector2i qk = _coords_to_quadrant_coords(p_layer, pk);

	HashMap<Vector2i, TileMapQuadrant>::Iterator Q = layers[p_layer].quadrant_map.find(qk);

	if (source_id == TileSet::INVALID_SOURCE) {
		// Erase existing cell in the tile map.
		tile_map.erase(pk);

		// Erase existing cell in the quadrant.
		ERR_FAIL_COND(!Q);
		TileMapQuadrant &q = Q->value;

		q.cells.erase(pk);

		// Remove or make the quadrant dirty.
		if (q.cells.size() == 0) {
			_erase_quadrant(Q);
		} else {
			_make_quadrant_dirty(Q);
		}

		used_rect_cache_dirty = true;
	} else {
		if (!E) {
			// Insert a new cell in the tile map.
			E = tile_map.insert(pk, TileMapCell());

			// Create a new quadrant if needed, then insert the cell if needed.
			if (!Q) {
				Q = _create_quadrant(p_layer, qk);
			}
			TileMapQuadrant &q = Q->value;
			q.cells.insert(pk);

		} else {
			ERR_FAIL_COND(!Q); // TileMapQuadrant should exist...

			if (E->value.source_id == source_id && E->value.get_atlas_coords() == atlas_coords && E->value.alternative_tile == alternative_tile) {
				return; // Nothing changed.
			}
		}

		TileMapCell &c = E->value;

		c.source_id = source_id;
		c.set_atlas_coords(atlas_coords);
		c.alternative_tile = alternative_tile;

		_make_quadrant_dirty(Q);
		used_rect_cache_dirty = true;
	}
}

Ref<TileMapPattern> TileMap::get_pattern(int p_layer, TypedArray<Vector2i> p_coords_array) {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), nullptr);
	ERR_FAIL_COND_V(!tile_set.is_valid(), nullptr);

	Ref<TileMapPattern> output;
	output.instantiate();
	if (p_coords_array.is_empty()) {
		return output;
	}

	Vector2i min = Vector2i(p_coords_array[0]);
	for (int i = 1; i < p_coords_array.size(); i++) {
		min = min.min(p_coords_array[i]);
	}

	Vector<Vector2i> coords_in_pattern_array;
	coords_in_pattern_array.resize(p_coords_array.size());
	Vector2i ensure_positive_offset;
	for (int i = 0; i < p_coords_array.size(); i++) {
		Vector2i coords = p_coords_array[i];
		Vector2i coords_in_pattern = coords - min;
		if (tile_set->get_tile_shape() != TileSet::TILE_SHAPE_SQUARE) {
			if (tile_set->get_tile_layout() == TileSet::TILE_LAYOUT_STACKED) {
				if (tile_set->get_tile_offset_axis() == TileSet::TILE_OFFSET_AXIS_HORIZONTAL && bool(min.y % 2) && bool(coords_in_pattern.y % 2)) {
					coords_in_pattern.x -= 1;
					if (coords_in_pattern.x < 0) {
						ensure_positive_offset.x = 1;
					}
				} else if (tile_set->get_tile_offset_axis() == TileSet::TILE_OFFSET_AXIS_VERTICAL && bool(min.x % 2) && bool(coords_in_pattern.x % 2)) {
					coords_in_pattern.y -= 1;
					if (coords_in_pattern.y < 0) {
						ensure_positive_offset.y = 1;
					}
				}
			} else if (tile_set->get_tile_layout() == TileSet::TILE_LAYOUT_STACKED_OFFSET) {
				if (tile_set->get_tile_offset_axis() == TileSet::TILE_OFFSET_AXIS_HORIZONTAL && bool(min.y % 2) && bool(coords_in_pattern.y % 2)) {
					coords_in_pattern.x += 1;
				} else if (tile_set->get_tile_offset_axis() == TileSet::TILE_OFFSET_AXIS_VERTICAL && bool(min.x % 2) && bool(coords_in_pattern.x % 2)) {
					coords_in_pattern.y += 1;
				}
			}
		}
		coords_in_pattern_array.write[i] = coords_in_pattern;
	}

	for (int i = 0; i < coords_in_pattern_array.size(); i++) {
		Vector2i coords = p_coords_array[i];
		Vector2i coords_in_pattern = coords_in_pattern_array[i];
		output->set_cell(coords_in_pattern + ensure_positive_offset, get_cell_source_id(p_layer, coords), get_cell_atlas_coords(p_layer, coords), get_cell_alternative_tile(p_layer, coords));
	}

	return output;
}

Vector2i TileMap::get_coords_for_body_rid(RID p_physics_body) {
	ERR_FAIL_COND_V_MSG(!bodies_coords.has(p_physics_body), Vector2i(), vformat("No tiles for the given body RID %d.", p_physics_body));
	return bodies_coords[p_physics_body];
}

int TileMap::get_layer_for_body_rid(RID p_physics_body) {
	ERR_FAIL_COND_V_MSG(!bodies_layers.has(p_physics_body), int(), vformat("No tiles for the given body RID %d.", p_physics_body));
	return bodies_layers[p_physics_body];
}

void TileMap::fix_invalid_tiles() {
	ERR_FAIL_COND_MSG(tile_set.is_null(), "Cannot fix invalid tiles if Tileset is not open.");

	for (unsigned int i = 0; i < layers.size(); i++) {
		const HashMap<Vector2i, TileMapCell> &tile_map = layers[i].tile_map;
		RBSet<Vector2i> coords;
		for (const KeyValue<Vector2i, TileMapCell> &E : tile_map) {
			TileSetSource *source = *tile_set->get_source(E.value.source_id);
			if (!source || !source->has_tile(E.value.get_atlas_coords()) || !source->has_alternative_tile(E.value.get_atlas_coords(), E.value.alternative_tile)) {
				coords.insert(E.key);
			}
		}
		for (const Vector2i &E : coords) {
			set_cell(i, E, TileSet::INVALID_SOURCE, TileSetSource::INVALID_ATLAS_COORDS, TileSetSource::INVALID_TILE_ALTERNATIVE);
		}
	}
}

#ifdef TOOLS_ENABLED
Rect2 TileMap::_edit_get_rect() const {
	// Return the visible rect of the tilemap
	const_cast<TileMap *>(this)->_recompute_rect_cache();
	return rect_cache;
}
#endif

// --- Override some methods of the CanvasItem class to pass the changes to the quadrants CanvasItems ---

void TileMap::set_light_mask(int p_light_mask) {
	// Occlusion: set light mask.
	CanvasItem::set_light_mask(p_light_mask);
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		for (const KeyValue<Vector2i, TileMapQuadrant> &E : layers[layer].quadrant_map) {
			for (const RID &ci : E.value.canvas_items) {
				RenderingServer::get_singleton()->canvas_item_set_light_mask(ci, get_light_mask());
			}
		}
		_rendering_update_layer(layer);
	}
}

void TileMap::set_material(const Ref<Material> &p_material) {
	// Set material for the whole tilemap.
	CanvasItem::set_material(p_material);

	// Update material for the whole tilemap.
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		for (KeyValue<Vector2i, TileMapQuadrant> &E : layers[layer].quadrant_map) {
			TileMapQuadrant &q = E.value;
			for (const RID &ci : q.canvas_items) {
				RS::get_singleton()->canvas_item_set_use_parent_material(ci, get_use_parent_material() || get_material().is_valid());
			}
		}
		_rendering_update_layer(layer);
	}
}

void TileMap::set_use_parent_material(bool p_use_parent_material) {
	// Set use_parent_material for the whole tilemap.
	CanvasItem::set_use_parent_material(p_use_parent_material);

	// Update use_parent_material for the whole tilemap.
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		for (KeyValue<Vector2i, TileMapQuadrant> &E : layers[layer].quadrant_map) {
			TileMapQuadrant &q = E.value;
			for (const RID &ci : q.canvas_items) {
				RS::get_singleton()->canvas_item_set_use_parent_material(ci, get_use_parent_material() || get_material().is_valid());
			}
		}
		_rendering_update_layer(layer);
	}
}

void TileMap::set_texture_filter(TextureFilter p_texture_filter) {
	// Set a default texture filter for the whole tilemap.
	CanvasItem::set_texture_filter(p_texture_filter);
	TextureFilter target_filter = get_texture_filter_in_tree();
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		for (HashMap<Vector2i, TileMapQuadrant>::Iterator F = layers[layer].quadrant_map.begin(); F; ++F) {
			TileMapQuadrant &q = F->value;
			for (const RID &ci : q.canvas_items) {
				RenderingServer::get_singleton()->canvas_item_set_default_texture_filter(ci, RS::CanvasItemTextureFilter(target_filter));
				_make_quadrant_dirty(F);
			}
		}
		_rendering_update_layer(layer);
	}
}

void TileMap::set_texture_repeat(CanvasItem::TextureRepeat p_texture_repeat) {
	// Set a default texture repeat for the whole tilemap.
	CanvasItem::set_texture_repeat(p_texture_repeat);
	TextureRepeat target_repeat = get_texture_repeat_in_tree();
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		for (HashMap<Vector2i, TileMapQuadrant>::Iterator F = layers[layer].quadrant_map.begin(); F; ++F) {
			TileMapQuadrant &q = F->value;
			for (const RID &ci : q.canvas_items) {
				RenderingServer::get_singleton()->canvas_item_set_default_texture_repeat(ci, RS::CanvasItemTextureRepeat(target_repeat));
				_make_quadrant_dirty(F);
			}
		}
		_rendering_update_layer(layer);
	}
}

void TileMap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tileset", "tileset"), &TileMap::set_tileset);
	ClassDB::bind_method(D_METHOD("get_tileset"), &TileMap::get_tileset);

	ClassDB::bind_method(D_METHOD("set_quadrant_size", "size"), &TileMap::set_quadrant_size);
	ClassDB::bind_method(D_METHOD("get_quadrant_size"), &TileMap::get_quadrant_size);

	ClassDB::bind_method(D_METHOD("get_layers_count"), &TileMap::get_layers_count);
	ClassDB::bind_method(D_METHOD("add_layer", "to_position"), &TileMap::add_layer);
	ClassDB::bind_method(D_METHOD("move_layer", "layer", "to_position"), &TileMap::move_layer);
	ClassDB::bind_method(D_METHOD("remove_layer", "layer"), &TileMap::remove_layer);
	ClassDB::bind_method(D_METHOD("set_layer_name", "layer", "name"), &TileMap::set_layer_name);
	ClassDB::bind_method(D_METHOD("get_layer_name", "layer"), &TileMap::get_layer_name);
	ClassDB::bind_method(D_METHOD("set_layer_enabled", "layer", "enabled"), &TileMap::set_layer_enabled);
	ClassDB::bind_method(D_METHOD("is_layer_enabled", "layer"), &TileMap::is_layer_enabled);
	ClassDB::bind_method(D_METHOD("set_layer_modulate", "layer", "modulate"), &TileMap::set_layer_modulate);
	ClassDB::bind_method(D_METHOD("get_layer_modulate", "layer"), &TileMap::get_layer_modulate);
	ClassDB::bind_method(D_METHOD("set_layer_y_sort_enabled", "layer", "y_sort_enabled"), &TileMap::set_layer_y_sort_enabled);
	ClassDB::bind_method(D_METHOD("is_layer_y_sort_enabled", "layer"), &TileMap::is_layer_y_sort_enabled);
	ClassDB::bind_method(D_METHOD("set_layer_y_sort_origin", "layer", "y_sort_origin"), &TileMap::set_layer_y_sort_origin);
	ClassDB::bind_method(D_METHOD("get_layer_y_sort_origin", "layer"), &TileMap::get_layer_y_sort_origin);
	ClassDB::bind_method(D_METHOD("set_layer_z_index", "layer", "z_index"), &TileMap::set_layer_z_index);
	ClassDB::bind_method(D_METHOD("get_layer_z_index", "layer"), &TileMap::get_layer_z_index);

	ClassDB::bind_method(D_METHOD("set_collision_animatable", "enabled"), &TileMap::set_collision_animatable);
	ClassDB::bind_method(D_METHOD("is_collision_animatable"), &TileMap::is_collision_animatable);
	ClassDB::bind_method(D_METHOD("set_collision_visibility_mode", "collision_visibility_mode"), &TileMap::set_collision_visibility_mode);
	ClassDB::bind_method(D_METHOD("get_collision_visibility_mode"), &TileMap::get_collision_visibility_mode);

	ClassDB::bind_method(D_METHOD("set_navigation_visibility_mode", "navigation_visibility_mode"), &TileMap::set_navigation_visibility_mode);
	ClassDB::bind_method(D_METHOD("get_navigation_visibility_mode"), &TileMap::get_navigation_visibility_mode);

	ClassDB::bind_method(D_METHOD("set_navigation_map", "layer", "map"), &TileMap::set_navigation_map);
	ClassDB::bind_method(D_METHOD("get_navigation_map", "layer"), &TileMap::get_navigation_map);

	ClassDB::bind_method(D_METHOD("set_cell", "layer", "coords", "source_id", "atlas_coords", "alternative_tile"), &TileMap::set_cell, DEFVAL(TileSet::INVALID_SOURCE), DEFVAL(TileSetSource::INVALID_ATLAS_COORDS), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("erase_cell", "layer", "coords"), &TileMap::erase_cell);
	ClassDB::bind_method(D_METHOD("get_cell_source_id", "layer", "coords", "use_proxies"), &TileMap::get_cell_source_id, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_cell_atlas_coords", "layer", "coords", "use_proxies"), &TileMap::get_cell_atlas_coords, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_cell_alternative_tile", "layer", "coords", "use_proxies"), &TileMap::get_cell_alternative_tile, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_cell_tile_data", "layer", "coords", "use_proxies"), &TileMap::get_cell_tile_data, DEFVAL(false));

	ClassDB::bind_method(D_METHOD("get_coords_for_body_rid", "body"), &TileMap::get_coords_for_body_rid);
	ClassDB::bind_method(D_METHOD("get_layer_for_body_rid", "body"), &TileMap::get_layer_for_body_rid);

	ClassDB::bind_method(D_METHOD("get_pattern", "layer", "coords_array"), &TileMap::get_pattern);
	ClassDB::bind_method(D_METHOD("map_pattern", "position_in_tilemap", "coords_in_pattern", "pattern"), &TileMap::map_pattern);
	ClassDB::bind_method(D_METHOD("set_pattern", "layer", "position", "pattern"), &TileMap::set_pattern);

	ClassDB::bind_method(D_METHOD("set_cells_terrain_connect", "layer", "cells", "terrain_set", "terrain", "ignore_empty_terrains"), &TileMap::set_cells_terrain_connect, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("set_cells_terrain_path", "layer", "path", "terrain_set", "terrain", "ignore_empty_terrains"), &TileMap::set_cells_terrain_path, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("fix_invalid_tiles"), &TileMap::fix_invalid_tiles);
	ClassDB::bind_method(D_METHOD("clear_layer", "layer"), &TileMap::clear_layer);
	ClassDB::bind_method(D_METHOD("clear"), &TileMap::clear);

	ClassDB::bind_method(D_METHOD("force_update", "layer"), &TileMap::force_update, DEFVAL(-1));

	ClassDB::bind_method(D_METHOD("get_surrounding_cells", "coords"), &TileMap::get_surrounding_cells);

	ClassDB::bind_method(D_METHOD("get_used_cells", "layer"), &TileMap::get_used_cells);
	ClassDB::bind_method(D_METHOD("get_used_cells_by_id", "layer", "source_id", "atlas_coords", "alternative_tile"), &TileMap::get_used_cells_by_id, DEFVAL(TileSet::INVALID_SOURCE), DEFVAL(TileSetSource::INVALID_ATLAS_COORDS), DEFVAL(TileSetSource::INVALID_TILE_ALTERNATIVE));
	ClassDB::bind_method(D_METHOD("get_used_rect"), &TileMap::get_used_rect);

	ClassDB::bind_method(D_METHOD("map_to_local", "map_position"), &TileMap::map_to_local);
	ClassDB::bind_method(D_METHOD("local_to_map", "local_position"), &TileMap::local_to_map);

	ClassDB::bind_method(D_METHOD("get_neighbor_cell", "coords", "neighbor"), &TileMap::get_neighbor_cell);

	ClassDB::bind_method(D_METHOD("_update_dirty_quadrants"), &TileMap::_update_dirty_quadrants);

	ClassDB::bind_method(D_METHOD("_tile_set_changed_deferred_update"), &TileMap::_tile_set_changed_deferred_update);

	GDVIRTUAL_BIND(_use_tile_data_runtime_update, "layer", "coords");
	GDVIRTUAL_BIND(_tile_data_runtime_update, "layer", "coords", "tile_data");

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tile_set", PROPERTY_HINT_RESOURCE_TYPE, "TileSet"), "set_tileset", "get_tileset");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_quadrant_size", PROPERTY_HINT_RANGE, "1,128,1"), "set_quadrant_size", "get_quadrant_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_animatable"), "set_collision_animatable", "is_collision_animatable");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_visibility_mode", PROPERTY_HINT_ENUM, "Default,Force Show,Force Hide"), "set_collision_visibility_mode", "get_collision_visibility_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "navigation_visibility_mode", PROPERTY_HINT_ENUM, "Default,Force Show,Force Hide"), "set_navigation_visibility_mode", "get_navigation_visibility_mode");

	ADD_ARRAY("layers", "layer_");

	ADD_PROPERTY_DEFAULT("format", FORMAT_1);

	ADD_SIGNAL(MethodInfo("changed"));

	BIND_ENUM_CONSTANT(VISIBILITY_MODE_DEFAULT);
	BIND_ENUM_CONSTANT(VISIBILITY_MODE_FORCE_HIDE);
	BIND_ENUM_CONSTANT(VISIBILITY_MODE_FORCE_SHOW);
}

void TileMap::_tile_set_changed() {
	emit_signal(SNAME("changed"));
	_tile_set_changed_deferred_update_needed = true;
	call_deferred(SNAME("_tile_set_changed_deferred_update"));
	update_configuration_warnings();
}

void TileMap::_tile_set_changed_deferred_update() {
	if (_tile_set_changed_deferred_update_needed) {
		_clear_internals();
		_recreate_internals();
		_tile_set_changed_deferred_update_needed = false;
	}
}

TileMap::TileMap() {
	set_notify_transform(true);
	set_notify_local_transform(false);

	layers.resize(1);
}

TileMap::~TileMap() {
	if (tile_set.is_valid()) {
		tile_set->disconnect("changed", callable_mp(this, &TileMap::_tile_set_changed));
	}

	_clear_internals();
}
