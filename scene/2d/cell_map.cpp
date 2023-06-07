/**************************************************************************/
/*  cell_map.cpp                                                            */
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

/* The idea here is to have a parent for all cell_map related and then separate them into SceneMap, ShapeMap and CellMap */

#include "cell_map.h"
#include "core/io/marshalls.h"

HashMap<Vector2i, CellSet::CellNeighbor> CellMap::TerrainConstraint::get_overlapping_coords_and_peering_bits() const {
	HashMap<Vector2i, CellSet::CellNeighbor> output;

	ERR_FAIL_COND_V(is_center_bit(), output);

	Ref<CellSet> cs = cell_map->get_cellset();
	ERR_FAIL_COND_V(!cs.is_valid(), output);

	CellSet::CellShape shape = cs->get_cell_shape();
	if (shape == CellSet::CELL_SHAPE_SQUARE) {
		switch (bit) {
			case 1:
				output[base_cell_coords] = CellSet::CELL_NEIGHBOR_RIGHT_SIDE;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_LEFT_SIDE;
				break;
			case 2:
				output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_CORNER;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER)] = CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_RIGHT_CORNER;
				break;
			case 3:
				output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_SIDE;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_SIDE;
				break;
			default:
				ERR_FAIL_V(output);
		}
	} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC) {
		switch (bit) {
			case 1:
				output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE;
				break;
			case 2:
				output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_CORNER;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_LEFT_CORNER;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_CORNER)] = CellSet::CELL_NEIGHBOR_TOP_CORNER;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE)] = CellSet::CELL_NEIGHBOR_RIGHT_CORNER;
				break;
			case 3:
				output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE;
				output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE;
				break;
			default:
				ERR_FAIL_V(output);
		}
	} else {
		// Half offset shapes.
		CellSet::CellOffsetAxis offset_axis = cs->get_cell_offset_axis();
		if (offset_axis == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
			switch (bit) {
				case 1:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_RIGHT_SIDE;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_LEFT_SIDE;
					break;
				case 2:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_CORNER;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_CORNER;
					break;
				case 3:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE;
					break;
				case 4:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_CORNER;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_RIGHT_CORNER;
					break;
				case 5:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE;
					break;
				default:
					ERR_FAIL_V(output);
			}
		} else {
			switch (bit) {
				case 1:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_RIGHT_CORNER;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_CORNER;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER;
					break;
				case 2:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE;
					break;
				case 3:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)] = CellSet::CELL_NEIGHBOR_LEFT_CORNER;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER;
					break;
				case 4:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_SIDE;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_SIDE;
					break;
				case 5:
					output[base_cell_coords] = CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE;
					output[cell_map->get_neighbor_cell(base_cell_coords, CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE)] = CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE;
					break;
				default:
					ERR_FAIL_V(output);
			}
		}
	}
	return output;
}

CellMap::TerrainConstraint::TerrainConstraint(const CellMap *p_cell_map, const Vector2i &p_position, int p_terrain) {
	cell_map = p_cell_map;

	Ref<CellSet> cs = cell_map->get_cellset();
	ERR_FAIL_COND(!cs.is_valid());

	bit = 0;
	base_cell_coords = p_position;
	terrain = p_terrain;
}

CellMap::TerrainConstraint::TerrainConstraint(const CellMap *p_cell_map, const Vector2i &p_position, const CellSet::CellNeighbor &p_bit, int p_terrain) {
	// The way we build the constraint make it easy to detect conflicting constraints.
	cell_map = p_cell_map;

	Ref<CellSet> cs = cell_map->get_cellset();
	ERR_FAIL_COND(!cs.is_valid());

	CellSet::CellShape shape = cs->get_cell_shape();
	if (shape == CellSet::CELL_SHAPE_SQUARE) {
		switch (p_bit) {
			case CellSet::CELL_NEIGHBOR_RIGHT_SIDE:
				bit = 1;
				base_cell_coords = p_position;
				break;
			case CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER:
				bit = 2;
				base_cell_coords = p_position;
				break;
			case CellSet::CELL_NEIGHBOR_BOTTOM_SIDE:
				bit = 3;
				base_cell_coords = p_position;
				break;
			case CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_CORNER:
				bit = 2;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_LEFT_SIDE);
				break;
			case CellSet::CELL_NEIGHBOR_LEFT_SIDE:
				bit = 1;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_LEFT_SIDE);
				break;
			case CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER:
				bit = 2;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER);
				break;
			case CellSet::CELL_NEIGHBOR_TOP_SIDE:
				bit = 3;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_SIDE);
				break;
			case CellSet::CELL_NEIGHBOR_TOP_RIGHT_CORNER:
				bit = 2;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_SIDE);
				break;
			default:
				ERR_FAIL();
				break;
		}
	} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC) {
		switch (p_bit) {
			case CellSet::CELL_NEIGHBOR_RIGHT_CORNER:
				bit = 2;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE);
				break;
			case CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE:
				bit = 1;
				base_cell_coords = p_position;
				break;
			case CellSet::CELL_NEIGHBOR_BOTTOM_CORNER:
				bit = 2;
				base_cell_coords = p_position;
				break;
			case CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE:
				bit = 3;
				base_cell_coords = p_position;
				break;
			case CellSet::CELL_NEIGHBOR_LEFT_CORNER:
				bit = 2;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE);
				break;
			case CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE:
				bit = 1;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE);
				break;
			case CellSet::CELL_NEIGHBOR_TOP_CORNER:
				bit = 2;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_CORNER);
				break;
			case CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE:
				bit = 3;
				base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE);
				break;
			default:
				ERR_FAIL();
				break;
		}
	} else {
		// Half-offset shapes
		CellSet::CellOffsetAxis offset_axis = cs->get_cell_offset_axis();
		if (offset_axis == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
			switch (p_bit) {
				case CellSet::CELL_NEIGHBOR_RIGHT_SIDE:
					bit = 1;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER:
					bit = 2;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE:
					bit = 3;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_CORNER:
					bit = 4;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE:
					bit = 5;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_CORNER:
					bit = 2;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_LEFT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_LEFT_SIDE:
					bit = 1;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_LEFT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER:
					bit = 4;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE:
					bit = 3;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_CORNER:
					bit = 2;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE:
					bit = 5;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_RIGHT_CORNER:
					bit = 4;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE);
					break;
				default:
					ERR_FAIL();
					break;
			}
		} else {
			switch (p_bit) {
				case CellSet::CELL_NEIGHBOR_RIGHT_CORNER:
					bit = 1;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE:
					bit = 2;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER:
					bit = 3;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_SIDE:
					bit = 4;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_CORNER:
					bit = 1;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE:
					bit = 5;
					base_cell_coords = p_position;
					break;
				case CellSet::CELL_NEIGHBOR_LEFT_CORNER:
					bit = 3;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE:
					bit = 2;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER:
					bit = 1;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_SIDE:
					bit = 4;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_RIGHT_CORNER:
					bit = 3;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_SIDE);
					break;
				case CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE:
					bit = 5;
					base_cell_coords = p_cell_map->get_neighbor_cell(p_position, CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE);
					break;
				default:
					ERR_FAIL();
					break;
			}
		}
	}
	terrain = p_terrain;
}

Vector2i CellMap::transform_coords_layout(const Vector2i &p_coords, CellSet::CellOffsetAxis p_offset_axis, CellSet::CellLayout p_from_layout, CellSet::CellLayout p_to_layout) {
	// Transform to stacked layout.
	Vector2i output = p_coords;
	if (p_offset_axis == CellSet::CELL_OFFSET_AXIS_VERTICAL) {
		SWAP(output.x, output.y);
	}
	switch (p_from_layout) {
		case CellSet::CELL_LAYOUT_STACKED:
			break;
		case CellSet::CELL_LAYOUT_STACKED_OFFSET:
			if (output.y % 2) {
				output.x -= 1;
			}
			break;
		case CellSet::CELL_LAYOUT_STAIRS_RIGHT:
		case CellSet::CELL_LAYOUT_STAIRS_DOWN:
			if ((p_from_layout == CellSet::CELL_LAYOUT_STAIRS_RIGHT) ^ (p_offset_axis == CellSet::CELL_OFFSET_AXIS_VERTICAL)) {
				if (output.y < 0 && bool(output.y % 2)) {
					output = Vector2i(output.x + output.y / 2 - 1, output.y);
				} else {
					output = Vector2i(output.x + output.y / 2, output.y);
				}
			} else {
				if (output.x < 0 && bool(output.x % 2)) {
					output = Vector2i(output.x / 2 - 1, output.x + output.y * 2);
				} else {
					output = Vector2i(output.x / 2, output.x + output.y * 2);
				}
			}
			break;
		case CellSet::CELL_LAYOUT_DIAMOND_RIGHT:
		case CellSet::CELL_LAYOUT_DIAMOND_DOWN:
			if ((p_from_layout == CellSet::CELL_LAYOUT_DIAMOND_RIGHT) ^ (p_offset_axis == CellSet::CELL_OFFSET_AXIS_VERTICAL)) {
				if ((output.x + output.y) < 0 && (output.x - output.y) % 2) {
					output = Vector2i((output.x + output.y) / 2 - 1, output.y - output.x);
				} else {
					output = Vector2i((output.x + output.y) / 2, -output.x + output.y);
				}
			} else {
				if ((output.x - output.y) < 0 && (output.x + output.y) % 2) {
					output = Vector2i((output.x - output.y) / 2 - 1, output.x + output.y);
				} else {
					output = Vector2i((output.x - output.y) / 2, output.x + output.y);
				}
			}
			break;
	}

	switch (p_to_layout) {
		case CellSet::CELL_LAYOUT_STACKED:
			break;
		case CellSet::CELL_LAYOUT_STACKED_OFFSET:
			if (output.y % 2) {
				output.x += 1;
			}
			break;
		case CellSet::CELL_LAYOUT_STAIRS_RIGHT:
		case CellSet::CELL_LAYOUT_STAIRS_DOWN:
			if ((p_to_layout == CellSet::CELL_LAYOUT_STAIRS_RIGHT) ^ (p_offset_axis == CellSet::CELL_OFFSET_AXIS_VERTICAL)) {
				if (output.y < 0 && (output.y % 2)) {
					output = Vector2i(output.x - output.y / 2 + 1, output.y);
				} else {
					output = Vector2i(output.x - output.y / 2, output.y);
				}
			} else {
				if (output.y % 2) {
					if (output.y < 0) {
						output = Vector2i(2 * output.x + 1, -output.x + output.y / 2 - 1);
					} else {
						output = Vector2i(2 * output.x + 1, -output.x + output.y / 2);
					}
				} else {
					output = Vector2i(2 * output.x, -output.x + output.y / 2);
				}
			}
			break;
		case CellSet::CELL_LAYOUT_DIAMOND_RIGHT:
		case CellSet::CELL_LAYOUT_DIAMOND_DOWN:
			if ((p_to_layout == CellSet::CELL_LAYOUT_DIAMOND_RIGHT) ^ (p_offset_axis == CellSet::CELL_OFFSET_AXIS_VERTICAL)) {
				if (output.y % 2) {
					if (output.y > 0) {
						output = Vector2i(output.x - output.y / 2, output.x + output.y / 2 + 1);
					} else {
						output = Vector2i(output.x - output.y / 2 + 1, output.x + output.y / 2);
					}
				} else {
					output = Vector2i(output.x - output.y / 2, output.x + output.y / 2);
				}
			} else {
				if (output.y % 2) {
					if (output.y < 0) {
						output = Vector2i(output.x + output.y / 2, -output.x + output.y / 2 - 1);
					} else {
						output = Vector2i(output.x + output.y / 2 + 1, -output.x + output.y / 2);
					}
				} else {
					output = Vector2i(output.x + output.y / 2, -output.x + output.y / 2);
				}
			}
			break;
	}

	if (p_offset_axis == CellSet::CELL_OFFSET_AXIS_VERTICAL) {
		SWAP(output.x, output.y);
	}

	return output;
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

void CellMap::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_clear_internals();
			_recreate_internals();
		} break;

		case NOTIFICATION_EXIT_TREE: {
			_clear_internals();
		} break;
	}
}

Ref<CellSet> CellMap::get_cellset() const {
	return cell_set;
}

void CellMap::set_cellset(const Ref<CellSet> &p_cellset) {
	if (p_cellset == cell_set) {
		return;
	}

	// Set the cellset, registering to its changes.
	if (cell_set.is_valid()) {
		cell_set->disconnect("changed", callable_mp(this, &CellMap::_cell_set_changed));
	}

	if (!p_cellset.is_valid()) {
		_clear_internals();
	}

	cell_set = p_cellset;

	if (cell_set.is_valid()) {
		cell_set->connect("changed", callable_mp(this, &CellMap::_cell_set_changed));
		_clear_internals();
		_recreate_internals();
	}

	emit_signal(SNAME("changed"));
}

void CellMap::set_quadrant_size(int p_size) {
	ERR_FAIL_COND_MSG(p_size < 1, "CellMapQuadrant size cannot be smaller than 1.");

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

	if (layers[p_layer].name == p_name) {
		return;
	}
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

	if (layers[p_layer].enabled == p_enabled) {
		return;
	}
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

	if (layers[p_layer].modulate == p_modulate) {
		return;
	}
	layers[p_layer].modulate = p_modulate;
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

	if (layers[p_layer].y_sort_enabled == p_y_sort_enabled) {
		return;
	}
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

void CellMap::set_y_sort_enabled(bool p_enable) {
	if (is_y_sort_enabled() == p_enable) {
		return;
	}
	Node2D::set_y_sort_enabled(p_enable);
	_clear_internals();
	_recreate_internals();
	emit_signal(SNAME("changed"));
	update_configuration_warnings();
}

void CellMap::set_layer_y_sort_origin(int p_layer, int p_y_sort_origin) {
	if (p_layer < 0) {
		p_layer = layers.size() + p_layer;
	}
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	if (layers[p_layer].y_sort_origin == p_y_sort_origin) {
		return;
	}
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

	if (layers[p_layer].z_index == p_z_index) {
		return;
	}
	layers[p_layer].z_index = p_z_index;
	update_configuration_warnings();
}

int CellMap::get_layer_z_index(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), false);
	return layers[p_layer].z_index;
}

Vector2i CellMap::_coords_to_quadrant_coords(int p_layer, const Vector2i &p_coords) const {
	int quad_size = get_effective_quadrant_size(p_layer);

	// Rounding down, instead of simply rounding towards zero (truncating)
	return Vector2i(
			p_coords.x > 0 ? p_coords.x / quad_size : (p_coords.x - (quad_size - 1)) / quad_size,
			p_coords.y > 0 ? p_coords.y / quad_size : (p_coords.y - (quad_size - 1)) / quad_size);
}

HashMap<Vector2i, CellMapQuadrant>::Iterator CellMap::_create_quadrant(int p_layer, const Vector2i &p_qk) {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), nullptr);

	CellMapQuadrant q;
	q.layer = p_layer;
	q.coords = p_qk;

	rect_cache_dirty = true;

	// Create the debug canvas item.
	RenderingServer *rs = RenderingServer::get_singleton();
	q.debug_canvas_item = rs->canvas_item_create();
	rs->canvas_item_set_z_index(q.debug_canvas_item, RS::CANVAS_ITEM_Z_MAX - 1);
	rs->canvas_item_set_parent(q.debug_canvas_item, get_canvas_item());

	// Call the create_quadrant method on plugins
	if (cell_set.is_valid()) {
		_rendering_create_quadrant(&q);
	}

	return layers[p_layer].quadrant_map.insert(p_qk, q);
}

void CellMap::_make_quadrant_dirty(HashMap<Vector2i, CellMapQuadrant>::Iterator Q) {
	// Make the given quadrant dirty, then trigger an update later.
	CellMapQuadrant &q = Q->value;
	if (!q.dirty_list_element.in_list()) {
		layers[q.layer].dirty_quadrant_list.add(&q.dirty_list_element);
	}
	_queue_update_dirty_quadrants();
}

void CellMap::_make_all_quadrants_dirty() {
	// Make all quandrants dirty, then trigger an update later.
	for (CellMapLayer &layer : layers) {
		for (KeyValue<Vector2i, CellMapQuadrant> &E : layer.quadrant_map) {
			if (!E.value.dirty_list_element.in_list()) {
				layer.dirty_quadrant_list.add(&E.value.dirty_list_element);
			}
		}
	}
	_queue_update_dirty_quadrants();
}

void CellMap::_queue_update_dirty_quadrants() {
	if (pending_update || !is_inside_tree()) {
		return;
	}
	pending_update = true;
	call_deferred(SNAME("_update_dirty_quadrants"));
}

void CellMap::_update_dirty_quadrants() {
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		SelfList<CellMapQuadrant>::List &dirty_quadrant_list = layers[layer].dirty_quadrant_list;

		// Update the coords cache.
		for (SelfList<CellMapQuadrant> *q = dirty_quadrant_list.first(); q; q = q->next()) {
			q->self()->map_to_local.clear();
			q->self()->local_to_map.clear();
			for (const Vector2i &E : q->self()->cells) {
				Vector2i pk = E;
				Vector2 pk_local_coords = map_to_local(pk);
				q->self()->map_to_local[pk] = pk_local_coords;
				q->self()->local_to_map[pk_local_coords] = pk;
			}
		}
		}
}

void CellMap::_recreate_layer_internals(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	// Make sure that _clear_internals() was called prior.
	ERR_FAIL_COND_MSG(layers[p_layer].quadrant_map.size() > 0, "CellMap layer " + itos(p_layer) + " had a non-empty quadrant map.");

	if (!layers[p_layer].enabled) {
		return;
	}

	// Recreate the quadrants.
	const HashMap<Vector2i, CellMapCell> &cell_map = layers[p_layer].cell_map;
	for (const KeyValue<Vector2i, CellMapCell> &E : cell_map) {
		Vector2i qk = _coords_to_quadrant_coords(p_layer, Vector2i(E.key.x, E.key.y));

		HashMap<Vector2i, CellMapQuadrant>::Iterator Q = layers[p_layer].quadrant_map.find(qk);
		if (!Q) {
			Q = _create_quadrant(p_layer, qk);
			layers[p_layer].dirty_quadrant_list.add(&Q->value.dirty_list_element);
		}

		Vector2i pk = E.key;
		Q->value.cells.insert(pk);

		_make_quadrant_dirty(Q);
	}

	_queue_update_dirty_quadrants();
}

void CellMap::_recreate_internals() {
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		_recreate_layer_internals(layer);
	}
}

void CellMap::_erase_quadrant(HashMap<Vector2i, CellMapQuadrant>::Iterator Q) {
	// Remove a quadrant.
	CellMapQuadrant *q = &(Q->value);
	// Remove the quadrant from the dirty_list if it is there.
	if (q->dirty_list_element.in_list()) {
		layers[q->layer].dirty_quadrant_list.remove(&(q->dirty_list_element));
	}

	// Free the debug canvas item.
	RenderingServer *rs = RenderingServer::get_singleton();
	rs->free(q->debug_canvas_item);

	layers[q->layer].quadrant_map.remove(Q);
	rect_cache_dirty = true;
}

void CellMap::_clear_layer_internals(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	// Clear quadrants.
	while (layers[p_layer].quadrant_map.size()) {
		_erase_quadrant(layers[p_layer].quadrant_map.begin());
	}

	// Clear the dirty quadrants list.
	while (layers[p_layer].dirty_quadrant_list.first()) {
		layers[p_layer].dirty_quadrant_list.remove(layers[p_layer].dirty_quadrant_list.first());
	}
}

void CellMap::_clear_internals() {
	// Clear quadrants.
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		_clear_layer_internals(layer);
	}
}

void CellMap::_recompute_rect_cache() {
	// Compute the displayed area of the cellmap.
#ifdef DEBUG_ENABLED

	if (!rect_cache_dirty) {
		return;
	}

	Rect2 r_total;
	bool first = true;
	for (unsigned int layer = 0; layer < layers.size(); layer++) {
		for (const KeyValue<Vector2i, CellMapQuadrant> &E : layers[layer].quadrant_map) {
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

void CellMap::set_cell(int p_layer, const Vector2i &p_coords, int p_source_id, const Vector2i p_atlas_coords, int p_alternative_cell) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	// Set the current cell cell (using integer position).
	HashMap<Vector2i, CellMapCell> &cell_map = layers[p_layer].cell_map;
	Vector2i pk(p_coords);
	HashMap<Vector2i, CellMapCell>::Iterator E = cell_map.find(pk);

	int source_id = p_source_id;
	Vector2i atlas_coords = p_atlas_coords;
	int alternative_cell = p_alternative_cell;

	if ((source_id == CellSet::INVALID_SOURCE || atlas_coords == CellSetSource::INVALID_ATLAS_COORDS || alternative_cell == CellSetSource::INVALID_cell_ALTERNATIVE) &&
			(source_id != CellSet::INVALID_SOURCE || atlas_coords != CellSetSource::INVALID_ATLAS_COORDS || alternative_cell != CellSetSource::INVALID_cell_ALTERNATIVE)) {
		source_id = CellSet::INVALID_SOURCE;
		atlas_coords = CellSetSource::INVALID_ATLAS_COORDS;
		alternative_cell = CellSetSource::INVALID_cell_ALTERNATIVE;
	}

	if (!E && source_id == CellSet::INVALID_SOURCE) {
		return; // Nothing to do, the cell is already empty.
	}

	// Get the quadrant
	Vector2i qk = _coords_to_quadrant_coords(p_layer, pk);

	HashMap<Vector2i, CellMapQuadrant>::Iterator Q = layers[p_layer].quadrant_map.find(qk);

	if (source_id == CellSet::INVALID_SOURCE) {
		// Erase existing cell in the cell map.
		cell_map.erase(pk);

		// Erase existing cell in the quadrant.
		ERR_FAIL_COND(!Q);
		CellMapQuadrant &q = Q->value;

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
			// Insert a new cell in the cell map.
			E = cell_map.insert(pk, CellMapCell());

			// Create a new quadrant if needed, then insert the cell if needed.
			if (!Q) {
				Q = _create_quadrant(p_layer, qk);
			}
			CellMapQuadrant &q = Q->value;
			q.cells.insert(pk);

		} else {
			ERR_FAIL_COND(!Q); // CellMapQuadrant should exist...

			if (E->value.source_id == source_id && E->value.get_atlas_coords() == atlas_coords && E->value.alternative_cell == alternative_cell) {
				return; // Nothing changed.
			}
		}

		CellMapCell &c = E->value;

		c.source_id = source_id;
		c.set_atlas_coords(atlas_coords);
		c.alternative_cell = alternative_cell;

		_make_quadrant_dirty(Q);
		used_rect_cache_dirty = true;
	}
}

void CellMap::erase_cell(int p_layer, const Vector2i &p_coords) {
	set_cell(p_layer, p_coords, CellSet::INVALID_SOURCE, CellSetSource::INVALID_ATLAS_COORDS, CellSetSource::INVALID_cell_ALTERNATIVE);
}

int CellMap::get_cell_source_id(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), CellSet::INVALID_SOURCE);

	// Get a cell source id from position.
	const HashMap<Vector2i, CellMapCell> &cell_map = layers[p_layer].cell_map;
	HashMap<Vector2i, CellMapCell>::ConstIterator E = cell_map.find(p_coords);

	if (!E) {
		return CellSet::INVALID_SOURCE;
	}

	if (p_use_proxies && cell_set.is_valid()) {
		Array proxyed = cell_set->map_cell_proxy(E->value.source_id, E->value.get_atlas_coords(), E->value.alternative_cell);
		return proxyed[0];
	}

	return E->value.source_id;
}

Vector2i CellMap::get_cell_atlas_coords(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), CellSetSource::INVALID_ATLAS_COORDS);

	// Get a cell source id from position
	const HashMap<Vector2i, CellMapCell> &cell_map = layers[p_layer].cell_map;
	HashMap<Vector2i, CellMapCell>::ConstIterator E = cell_map.find(p_coords);

	if (!E) {
		return CellSetSource::INVALID_ATLAS_COORDS;
	}

	if (p_use_proxies && cell_set.is_valid()) {
		Array proxyed = cell_set->map_cell_proxy(E->value.source_id, E->value.get_atlas_coords(), E->value.alternative_cell);
		return proxyed[1];
	}

	return E->value.get_atlas_coords();
}

int CellMap::get_cell_alternative_cell(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), CellSetSource::INVALID_cell_ALTERNATIVE);

	// Get a cell source id from position
	const HashMap<Vector2i, CellMapCell> &cell_map = layers[p_layer].cell_map;
	HashMap<Vector2i, CellMapCell>::ConstIterator E = cell_map.find(p_coords);

	if (!E) {
		return CellSetSource::INVALID_cell_ALTERNATIVE;
	}

	if (p_use_proxies && cell_set.is_valid()) {
		Array proxyed = cell_set->map_cell_proxy(E->value.source_id, E->value.get_atlas_coords(), E->value.alternative_cell);
		return proxyed[2];
	}

	return E->value.alternative_cell;
}

CellData *CellMap::get_cell_data(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	int source_id = get_cell_source_id(p_layer, p_coords, p_use_proxies);
	if (source_id == CellSet::INVALID_SOURCE) {
		return nullptr;
	}

	Ref<CellSetAtlasSource> source = cell_set->get_source(source_id);
	if (source.is_valid()) {
		return source->get_cell_data(get_cell_atlas_coords(p_layer, p_coords, p_use_proxies), get_cell_alternative_cell(p_layer, p_coords, p_use_proxies));
	}

	return nullptr;
}

Vector2i CellMap::map_pattern(const Vector2i &p_position_in_cellmap, const Vector2i &p_coords_in_pattern, Ref<CellMapPattern> p_pattern) {
	ERR_FAIL_COND_V(p_pattern.is_null(), Vector2i());
	ERR_FAIL_COND_V(!p_pattern->has_cell(p_coords_in_pattern), Vector2i());

	Vector2i output = p_position_in_cellmap + p_coords_in_pattern;
	if (cell_set->get_cell_shape() != CellSet::CELL_SHAPE_SQUARE) {
		if (cell_set->get_cell_layout() == CellSet::CELL_LAYOUT_STACKED) {
			if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL && bool(p_position_in_cellmap.y % 2) && bool(p_coords_in_pattern.y % 2)) {
				output.x += 1;
			} else if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_VERTICAL && bool(p_position_in_cellmap.x % 2) && bool(p_coords_in_pattern.x % 2)) {
				output.y += 1;
			}
		} else if (cell_set->get_cell_layout() == CellSet::CELL_LAYOUT_STACKED_OFFSET) {
			if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL && bool(p_position_in_cellmap.y % 2) && bool(p_coords_in_pattern.y % 2)) {
				output.x -= 1;
			} else if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_VERTICAL && bool(p_position_in_cellmap.x % 2) && bool(p_coords_in_pattern.x % 2)) {
				output.y -= 1;
			}
		}
	}

	return output;
}

void CellMap::set_pattern(int p_layer, const Vector2i &p_position, const Ref<CellMapPattern> p_pattern) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	ERR_FAIL_COND(!cell_set.is_valid());

	TypedArray<Vector2i> used_cells = p_pattern->get_used_cells();
	for (int i = 0; i < used_cells.size(); i++) {
		Vector2i coords = map_pattern(p_position, used_cells[i], p_pattern);
		set_cell(p_layer, coords, p_pattern->get_cell_source_id(used_cells[i]), p_pattern->get_cell_atlas_coords(used_cells[i]), p_pattern->get_cell_alternative_cell(used_cells[i]));
	}
}

CellSet::TerrainsPattern CellMap::_get_best_terrain_pattern_for_constraints(int p_terrain_set, const Vector2i &p_position, const RBSet<TerrainConstraint> &p_constraints, CellSet::TerrainsPattern p_current_pattern) {
	if (!cell_set.is_valid()) {
		return CellSet::TerrainsPattern();
	}
	// Returns all cells compatible with the given constraints.
	RBMap<CellSet::TerrainsPattern, int> terrain_pattern_score;
	RBSet<CellSet::TerrainsPattern> pattern_set = cell_set->get_terrains_pattern_set(p_terrain_set);
	ERR_FAIL_COND_V(pattern_set.is_empty(), CellSet::TerrainsPattern());
	for (CellSet::TerrainsPattern &terrain_pattern : pattern_set) {
		int score = 0;

		// Check the center bit constraint
		TerrainConstraint terrain_constraint = TerrainConstraint(this, p_position, terrain_pattern.get_terrain());
		const RBSet<TerrainConstraint>::Element *in_set_constraint_element = p_constraints.find(terrain_constraint);
		if (in_set_constraint_element) {
			if (in_set_constraint_element->get().get_terrain() != terrain_constraint.get_terrain()) {
				score += in_set_constraint_element->get().get_priority();
			}
		} else if (p_current_pattern.get_terrain() != terrain_pattern.get_terrain()) {
			continue; // Ignore a pattern that cannot keep bits without constraints unmodified.
		}

		// Check the surrounding bits
		bool invalid_pattern = false;
		for (int i = 0; i < CellSet::CELL_NEIGHBOR_MAX; i++) {
			CellSet::CellNeighbor bit = CellSet::CellNeighbor(i);
			if (cell_set->is_valid_terrain_peering_bit(p_terrain_set, bit)) {
				// Check if the bit is compatible with the constraints.
				TerrainConstraint terrain_bit_constraint = TerrainConstraint(this, p_position, bit, terrain_pattern.get_terrain_peering_bit(bit));
				in_set_constraint_element = p_constraints.find(terrain_bit_constraint);
				if (in_set_constraint_element) {
					if (in_set_constraint_element->get().get_terrain() != terrain_bit_constraint.get_terrain()) {
						score += in_set_constraint_element->get().get_priority();
					}
				} else if (p_current_pattern.get_terrain_peering_bit(bit) != terrain_pattern.get_terrain_peering_bit(bit)) {
					invalid_pattern = true; // Ignore a pattern that cannot keep bits without constraints unmodified.
					break;
				}
			}
		}
		if (invalid_pattern) {
			continue;
		}

		terrain_pattern_score[terrain_pattern] = score;
	}

	// Compute the minimum score
	CellSet::TerrainsPattern min_score_pattern = p_current_pattern;
	int min_score = INT32_MAX;
	for (KeyValue<CellSet::TerrainsPattern, int> E : terrain_pattern_score) {
		if (E.value < min_score) {
			min_score_pattern = E.key;
			min_score = E.value;
		}
	}

	return min_score_pattern;
}

RBSet<CellMap::TerrainConstraint> CellMap::_get_terrain_constraints_from_added_pattern(const Vector2i &p_position, int p_terrain_set, CellSet::TerrainsPattern p_terrains_pattern) const {
	if (!cell_set.is_valid()) {
		return RBSet<TerrainConstraint>();
	}

	// Compute the constraints needed from the surrounding cells.
	RBSet<TerrainConstraint> output;
	output.insert(TerrainConstraint(this, p_position, p_terrains_pattern.get_terrain()));

	for (uint32_t i = 0; i < CellSet::CELL_NEIGHBOR_MAX; i++) {
		CellSet::CellNeighbor side = CellSet::CellNeighbor(i);
		if (cell_set->is_valid_terrain_peering_bit(p_terrain_set, side)) {
			TerrainConstraint c = TerrainConstraint(this, p_position, side, p_terrains_pattern.get_terrain_peering_bit(side));
			output.insert(c);
		}
	}

	return output;
}

RBSet<CellMap::TerrainConstraint> CellMap::_get_terrain_constraints_from_painted_cells_list(int p_layer, const RBSet<Vector2i> &p_painted, int p_terrain_set, bool p_ignore_empty_terrains) const {
	if (!cell_set.is_valid()) {
		return RBSet<TerrainConstraint>();
	}

	ERR_FAIL_INDEX_V(p_terrain_set, cell_set->get_terrain_sets_count(), RBSet<TerrainConstraint>());
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), RBSet<TerrainConstraint>());

	// Build a set of dummy constraints to get the constrained points.
	RBSet<TerrainConstraint> dummy_constraints;
	for (const Vector2i &E : p_painted) {
		for (int i = 0; i < CellSet::CELL_NEIGHBOR_MAX; i++) { // Iterates over neighbor bits.
			CellSet::CellNeighbor bit = CellSet::CellNeighbor(i);
			if (cell_set->is_valid_terrain_peering_bit(p_terrain_set, bit)) {
				dummy_constraints.insert(TerrainConstraint(this, E, bit, -1));
			}
		}
	}

	// For each constrained point, we get all overlapping cells, and select the most adequate terrain for it.
	RBSet<TerrainConstraint> constraints;
	for (const TerrainConstraint &E_constraint : dummy_constraints) {
		HashMap<int, int> terrain_count;

		// Count the number of occurrences per terrain.
		HashMap<Vector2i, CellSet::CellNeighbor> overlapping_terrain_bits = E_constraint.get_overlapping_coords_and_peering_bits();
		for (const KeyValue<Vector2i, CellSet::CellNeighbor> &E_overlapping : overlapping_terrain_bits) {
			CellData *neighbor_cell_data = nullptr;
			CellMapCell neighbor_cell = get_cell(p_layer, E_overlapping.key);
			if (neighbor_cell.source_id != CellSet::INVALID_SOURCE) {
				Ref<CellSetSource> source = cell_set->get_source(neighbor_cell.source_id);
				Ref<CellSetAtlasSource> atlas_source = source;
				if (atlas_source.is_valid()) {
					CellData *cell_data = atlas_source->get_cell_data(neighbor_cell.get_atlas_coords(), neighbor_cell.alternative_cell);
					if (cell_data && cell_data->get_terrain_set() == p_terrain_set) {
						neighbor_cell_data = cell_data;
					}
				}
			}

			int terrain = neighbor_cell_data ? neighbor_cell_data->get_terrain_peering_bit(CellSet::CellNeighbor(E_overlapping.value)) : -1;
			if (!p_ignore_empty_terrains || terrain >= 0) {
				if (!terrain_count.has(terrain)) {
					terrain_count[terrain] = 0;
				}
				terrain_count[terrain] += 1;
			}
		}

		// Get the terrain with the max number of occurrences.
		int max = 0;
		int max_terrain = -1;
		for (const KeyValue<int, int> &E_terrain_count : terrain_count) {
			if (E_terrain_count.value > max) {
				max = E_terrain_count.value;
				max_terrain = E_terrain_count.key;
			}
		}

		// Set the adequate terrain.
		if (max > 0) {
			TerrainConstraint c = E_constraint;
			c.set_terrain(max_terrain);
			constraints.insert(c);
		}
	}

	// Add the centers as constraints
	for (Vector2i E_coords : p_painted) {
		CellData *cell_data = nullptr;
		CellMapCell cell = get_cell(p_layer, E_coords);
		if (cell.source_id != CellSet::INVALID_SOURCE) {
			Ref<CellSetSource> source = cell_set->get_source(cell.source_id);
			Ref<CellSetAtlasSource> atlas_source = source;
			if (atlas_source.is_valid()) {
				cell_data = atlas_source->get_cell_data(cell.get_atlas_coords(), cell.alternative_cell);
			}
		}

		int terrain = (cell_data && cell_data->get_terrain_set() == p_terrain_set) ? cell_data->get_terrain() : -1;
		if (!p_ignore_empty_terrains || terrain >= 0) {
			constraints.insert(TerrainConstraint(this, E_coords, terrain));
		}
	}

	return constraints;
}

HashMap<Vector2i, CellSet::TerrainsPattern> CellMap::terrain_fill_constraints(int p_layer, const Vector<Vector2i> &p_to_replace, int p_terrain_set, const RBSet<TerrainConstraint> &p_constraints) {
	if (!cell_set.is_valid()) {
		return HashMap<Vector2i, CellSet::TerrainsPattern>();
	}

	// Copy the constraints set.
	RBSet<TerrainConstraint> constraints = p_constraints;

	// Output map.
	HashMap<Vector2i, CellSet::TerrainsPattern> output;

	// Add all positions to a set.
	for (int i = 0; i < p_to_replace.size(); i++) {
		const Vector2i &coords = p_to_replace[i];

		// Select the best pattern for the given constraints
		CellSet::TerrainsPattern current_pattern = CellSet::TerrainsPattern(*cell_set, p_terrain_set);
		CellMapCell cell = get_cell(p_layer, coords);
		if (cell.source_id != CellSet::INVALID_SOURCE) {
			CellSetSource *source = *cell_set->get_source(cell.source_id);
			CellSetAtlasSource *atlas_source = Object::cast_to<CellSetAtlasSource>(source);
			if (atlas_source) {
				// Get cell data.
				CellData *cell_data = atlas_source->get_cell_data(cell.get_atlas_coords(), cell.alternative_cell);
				if (cell_data && cell_data->get_terrain_set() == p_terrain_set) {
					current_pattern = cell_data->get_terrains_pattern();
				}
			}
		}
		CellSet::TerrainsPattern pattern = _get_best_terrain_pattern_for_constraints(p_terrain_set, coords, constraints, current_pattern);

		// Update the constraint set with the new ones
		RBSet<TerrainConstraint> new_constraints = _get_terrain_constraints_from_added_pattern(coords, p_terrain_set, pattern);
		for (const TerrainConstraint &E_constraint : new_constraints) {
			if (constraints.has(E_constraint)) {
				constraints.erase(E_constraint);
			}
			TerrainConstraint c = E_constraint;
			c.set_priority(5);
			constraints.insert(c);
		}

		output[coords] = pattern;
	}
	return output;
}

HashMap<Vector2i, CellSet::TerrainsPattern> CellMap::terrain_fill_connect(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains) {
	HashMap<Vector2i, CellSet::TerrainsPattern> output;
	ERR_FAIL_COND_V(!cell_set.is_valid(), output);
	ERR_FAIL_INDEX_V(p_terrain_set, cell_set->get_terrain_sets_count(), output);

	// Build list and set of cells that can be modified (painted and their surroundings)
	Vector<Vector2i> can_modify_list;
	RBSet<Vector2i> can_modify_set;
	RBSet<Vector2i> painted_set;
	for (int i = p_coords_array.size() - 1; i >= 0; i--) {
		const Vector2i &coords = p_coords_array[i];
		can_modify_list.push_back(coords);
		can_modify_set.insert(coords);
		painted_set.insert(coords);
	}
	for (Vector2i coords : p_coords_array) {
		// Find the adequate neighbor
		for (int j = 0; j < CellSet::CELL_NEIGHBOR_MAX; j++) {
			CellSet::CellNeighbor bit = CellSet::CellNeighbor(j);
			if (is_existing_neighbor(bit)) {
				Vector2i neighbor = get_neighbor_cell(coords, bit);
				if (!can_modify_set.has(neighbor)) {
					can_modify_list.push_back(neighbor);
					can_modify_set.insert(neighbor);
				}
			}
		}
	}

	// Build a set, out of the possibly modified cells, of the one with a center bit that is set (or will be) to the painted terrain
	RBSet<Vector2i> cells_with_terrain_center_bit;
	for (Vector2i coords : can_modify_set) {
		bool connect = false;
		if (painted_set.has(coords)) {
			connect = true;
		} else {
			// Get the center bit of the cell
			CellData *cell_data = nullptr;
			CellMapCell cell = get_cell(p_layer, coords);
			if (cell.source_id != CellSet::INVALID_SOURCE) {
				Ref<CellSetSource> source = cell_set->get_source(cell.source_id);
				Ref<CellSetAtlasSource> atlas_source = source;
				if (atlas_source.is_valid()) {
					cell_data = atlas_source->get_cell_data(cell.get_atlas_coords(), cell.alternative_cell);
				}
			}

			if (cell_data && cell_data->get_terrain_set() == p_terrain_set && cell_data->get_terrain() == p_terrain) {
				connect = true;
			}
		}
		if (connect) {
			cells_with_terrain_center_bit.insert(coords);
		}
	}

	RBSet<TerrainConstraint> constraints;

	// Add new constraints from the path drawn.
	for (Vector2i coords : p_coords_array) {
		// Constraints on the center bit.
		TerrainConstraint c = TerrainConstraint(this, coords, p_terrain);
		c.set_priority(10);
		constraints.insert(c);

		// Constraints on the connecting bits.
		for (int j = 0; j < CellSet::CELL_NEIGHBOR_MAX; j++) {
			CellSet::CellNeighbor bit = CellSet::CellNeighbor(j);
			if (cell_set->is_valid_terrain_peering_bit(p_terrain_set, bit)) {
				c = TerrainConstraint(this, coords, bit, p_terrain);
				c.set_priority(10);
				if ((int(bit) % 2) == 0) {
					// Side peering bits: add the constraint if the center is of the same terrain
					Vector2i neighbor = get_neighbor_cell(coords, bit);
					if (cells_with_terrain_center_bit.has(neighbor)) {
						constraints.insert(c);
					}
				} else {
					// Corner peering bits: add the constraint if all cells on the constraint has the same center bit
					HashMap<Vector2i, CellSet::CellNeighbor> overlapping_terrain_bits = c.get_overlapping_coords_and_peering_bits();
					bool valid = true;
					for (KeyValue<Vector2i, CellSet::CellNeighbor> kv : overlapping_terrain_bits) {
						if (!cells_with_terrain_center_bit.has(kv.key)) {
							valid = false;
							break;
						}
					}
					if (valid) {
						constraints.insert(c);
					}
				}
			}
		}
	}

	// Fills in the constraint list from existing cells.
	for (TerrainConstraint c : _get_terrain_constraints_from_painted_cells_list(p_layer, painted_set, p_terrain_set, p_ignore_empty_terrains)) {
		constraints.insert(c);
	}

	// Fill the terrains.
	output = terrain_fill_constraints(p_layer, can_modify_list, p_terrain_set, constraints);
	return output;
}

HashMap<Vector2i, CellSet::TerrainsPattern> CellMap::terrain_fill_path(int p_layer, const Vector<Vector2i> &p_path, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains) {
	HashMap<Vector2i, CellSet::TerrainsPattern> output;
	ERR_FAIL_COND_V(!cell_set.is_valid(), output);
	ERR_FAIL_INDEX_V(p_terrain_set, cell_set->get_terrain_sets_count(), output);

	// Make sure the path is correct and build the peering bit list while doing it.
	Vector<CellSet::CellNeighbor> neighbor_list;
	for (int i = 0; i < p_path.size() - 1; i++) {
		// Find the adequate neighbor
		CellSet::CellNeighbor found_bit = CellSet::CELL_NEIGHBOR_MAX;
		for (int j = 0; j < CellSet::CELL_NEIGHBOR_MAX; j++) {
			CellSet::CellNeighbor bit = CellSet::CellNeighbor(j);
			if (is_existing_neighbor(bit)) {
				if (get_neighbor_cell(p_path[i], bit) == p_path[i + 1]) {
					found_bit = bit;
					break;
				}
			}
		}
		ERR_FAIL_COND_V_MSG(found_bit == CellSet::CELL_NEIGHBOR_MAX, output, vformat("Invalid terrain path, %s is not a neighboring cell of %s", p_path[i + 1], p_path[i]));
		neighbor_list.push_back(found_bit);
	}

	// Build list and set of cells that can be modified (painted and their surroundings)
	Vector<Vector2i> can_modify_list;
	RBSet<Vector2i> can_modify_set;
	RBSet<Vector2i> painted_set;
	for (int i = p_path.size() - 1; i >= 0; i--) {
		const Vector2i &coords = p_path[i];
		can_modify_list.push_back(coords);
		can_modify_set.insert(coords);
		painted_set.insert(coords);
	}
	for (Vector2i coords : p_path) {
		// Find the adequate neighbor
		for (int j = 0; j < CellSet::CELL_NEIGHBOR_MAX; j++) {
			CellSet::CellNeighbor bit = CellSet::CellNeighbor(j);
			if (cell_set->is_valid_terrain_peering_bit(p_terrain_set, bit)) {
				Vector2i neighbor = get_neighbor_cell(coords, bit);
				if (!can_modify_set.has(neighbor)) {
					can_modify_list.push_back(neighbor);
					can_modify_set.insert(neighbor);
				}
			}
		}
	}

	RBSet<TerrainConstraint> constraints;

	// Add new constraints from the path drawn.
	for (Vector2i coords : p_path) {
		// Constraints on the center bit
		TerrainConstraint c = TerrainConstraint(this, coords, p_terrain);
		c.set_priority(10);
		constraints.insert(c);
	}
	for (int i = 0; i < p_path.size() - 1; i++) {
		// Constraints on the peering bits.
		TerrainConstraint c = TerrainConstraint(this, p_path[i], neighbor_list[i], p_terrain);
		c.set_priority(10);
		constraints.insert(c);
	}

	// Fills in the constraint list from existing cells.
	for (TerrainConstraint c : _get_terrain_constraints_from_painted_cells_list(p_layer, painted_set, p_terrain_set, p_ignore_empty_terrains)) {
		constraints.insert(c);
	}

	// Fill the terrains.
	output = terrain_fill_constraints(p_layer, can_modify_list, p_terrain_set, constraints);
	return output;
}

HashMap<Vector2i, CellSet::TerrainsPattern> CellMap::terrain_fill_pattern(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, CellSet::TerrainsPattern p_terrains_pattern, bool p_ignore_empty_terrains) {
	HashMap<Vector2i, CellSet::TerrainsPattern> output;
	ERR_FAIL_COND_V(!cell_set.is_valid(), output);
	ERR_FAIL_INDEX_V(p_terrain_set, cell_set->get_terrain_sets_count(), output);

	// Build list and set of cells that can be modified (painted and their surroundings).
	Vector<Vector2i> can_modify_list;
	RBSet<Vector2i> can_modify_set;
	RBSet<Vector2i> painted_set;
	for (int i = p_coords_array.size() - 1; i >= 0; i--) {
		const Vector2i &coords = p_coords_array[i];
		can_modify_list.push_back(coords);
		can_modify_set.insert(coords);
		painted_set.insert(coords);
	}
	for (Vector2i coords : p_coords_array) {
		// Find the adequate neighbor
		for (int j = 0; j < CellSet::CELL_NEIGHBOR_MAX; j++) {
			CellSet::CellNeighbor bit = CellSet::CellNeighbor(j);
			if (cell_set->is_valid_terrain_peering_bit(p_terrain_set, bit)) {
				Vector2i neighbor = get_neighbor_cell(coords, bit);
				if (!can_modify_set.has(neighbor)) {
					can_modify_list.push_back(neighbor);
					can_modify_set.insert(neighbor);
				}
			}
		}
	}

	// Add constraint by the new ones.
	RBSet<TerrainConstraint> constraints;

	// Add new constraints from the path drawn.
	for (Vector2i coords : p_coords_array) {
		// Constraints on the center bit
		RBSet<TerrainConstraint> added_constraints = _get_terrain_constraints_from_added_pattern(coords, p_terrain_set, p_terrains_pattern);
		for (TerrainConstraint c : added_constraints) {
			c.set_priority(10);
			constraints.insert(c);
		}
	}

	// Fills in the constraint list from modified cells border.
	for (TerrainConstraint c : _get_terrain_constraints_from_painted_cells_list(p_layer, painted_set, p_terrain_set, p_ignore_empty_terrains)) {
		constraints.insert(c);
	}

	// Fill the terrains.
	output = terrain_fill_constraints(p_layer, can_modify_list, p_terrain_set, constraints);
	return output;
}

void CellMap::set_cells_terrain_connect(int p_layer, TypedArray<Vector2i> p_cells, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains) {
	ERR_FAIL_COND(!cell_set.is_valid());
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	ERR_FAIL_INDEX(p_terrain_set, cell_set->get_terrain_sets_count());

	Vector<Vector2i> cells_vector;
	HashSet<Vector2i> painted_set;
	for (int i = 0; i < p_cells.size(); i++) {
		cells_vector.push_back(p_cells[i]);
		painted_set.insert(p_cells[i]);
	}
	HashMap<Vector2i, CellSet::TerrainsPattern> terrain_fill_output = terrain_fill_connect(p_layer, cells_vector, p_terrain_set, p_terrain, p_ignore_empty_terrains);
	for (const KeyValue<Vector2i, CellSet::TerrainsPattern> &kv : terrain_fill_output) {
		if (painted_set.has(kv.key)) {
			// Paint a random cell with the correct terrain for the painted path.
			CellMapCell c = cell_set->get_random_cell_from_terrains_pattern(p_terrain_set, kv.value);
			set_cell(p_layer, kv.key, c.source_id, c.get_atlas_coords(), c.alternative_cell);
		} else {
			// Avoids updating the painted path from the output if the new pattern is the same as before.
			CellSet::TerrainsPattern in_map_terrain_pattern = CellSet::TerrainsPattern(*cell_set, p_terrain_set);
			CellMapCell cell = get_cell(p_layer, kv.key);
			if (cell.source_id != CellSet::INVALID_SOURCE) {
				CellSetSource *source = *cell_set->get_source(cell.source_id);
				CellSetAtlasSource *atlas_source = Object::cast_to<CellSetAtlasSource>(source);
				if (atlas_source) {
					// Get cell data.
					CellData *cell_data = atlas_source->get_cell_data(cell.get_atlas_coords(), cell.alternative_cell);
					if (cell_data && cell_data->get_terrain_set() == p_terrain_set) {
						in_map_terrain_pattern = cell_data->get_terrains_pattern();
					}
				}
			}
			if (in_map_terrain_pattern != kv.value) {
				CellMapCell c = cell_set->get_random_cell_from_terrains_pattern(p_terrain_set, kv.value);
				set_cell(p_layer, kv.key, c.source_id, c.get_atlas_coords(), c.alternative_cell);
			}
		}
	}
}

void CellMap::set_cells_terrain_path(int p_layer, TypedArray<Vector2i> p_path, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains) {
	ERR_FAIL_COND(!cell_set.is_valid());
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	ERR_FAIL_INDEX(p_terrain_set, cell_set->get_terrain_sets_count());

	Vector<Vector2i> vector_path;
	HashSet<Vector2i> painted_set;
	for (int i = 0; i < p_path.size(); i++) {
		vector_path.push_back(p_path[i]);
		painted_set.insert(p_path[i]);
	}

	HashMap<Vector2i, CellSet::TerrainsPattern> terrain_fill_output = terrain_fill_path(p_layer, vector_path, p_terrain_set, p_terrain, p_ignore_empty_terrains);
	for (const KeyValue<Vector2i, CellSet::TerrainsPattern> &kv : terrain_fill_output) {
		if (painted_set.has(kv.key)) {
			// Paint a random cell with the correct terrain for the painted path.
			CellMapCell c = cell_set->get_random_cell_from_terrains_pattern(p_terrain_set, kv.value);
			set_cell(p_layer, kv.key, c.source_id, c.get_atlas_coords(), c.alternative_cell);
		} else {
			// Avoids updating the painted path from the output if the new pattern is the same as before.
			CellSet::TerrainsPattern in_map_terrain_pattern = CellSet::TerrainsPattern(*cell_set, p_terrain_set);
			CellMapCell cell = get_cell(p_layer, kv.key);
			if (cell.source_id != CellSet::INVALID_SOURCE) {
				CellSetSource *source = *cell_set->get_source(cell.source_id);
				CellSetAtlasSource *atlas_source = Object::cast_to<CellSetAtlasSource>(source);
				if (atlas_source) {
					// Get cell data.
					CellData *cell_data = atlas_source->get_cell_data(cell.get_atlas_coords(), cell.alternative_cell);
					if (cell_data && cell_data->get_terrain_set() == p_terrain_set) {
						in_map_terrain_pattern = cell_data->get_terrains_pattern();
					}
				}
			}
			if (in_map_terrain_pattern != kv.value) {
				CellMapCell c = cell_set->get_random_cell_from_terrains_pattern(p_terrain_set, kv.value);
				set_cell(p_layer, kv.key, c.source_id, c.get_atlas_coords(), c.alternative_cell);
			}
		}
	}
}

CellMapCell CellMap::get_cell(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), CellMapCell());
	const HashMap<Vector2i, CellMapCell> &cell_map = layers[p_layer].cell_map;
	if (!cell_map.has(p_coords)) {
		return CellMapCell();
	} else {
		CellMapCell c = cell_map.find(p_coords)->value;
		if (p_use_proxies && cell_set.is_valid()) {
			Array proxyed = cell_set->map_cell_proxy(c.source_id, c.get_atlas_coords(), c.alternative_cell);
			c.source_id = proxyed[0];
			c.set_atlas_coords(proxyed[1]);
			c.alternative_cell = proxyed[2];
		}
		return c;
	}
}

HashMap<Vector2i, CellMapQuadrant> *CellMap::get_quadrant_map(int p_layer) {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), nullptr);

	return &layers[p_layer].quadrant_map;
}

void CellMap::fix_invalid_cells() {
	ERR_FAIL_COND_MSG(cell_set.is_null(), "Cannot fix invalid cells if Cellset is not open.");

	for (unsigned int i = 0; i < layers.size(); i++) {
		const HashMap<Vector2i, CellMapCell> &cell_map = layers[i].cell_map;
		RBSet<Vector2i> coords;
		for (const KeyValue<Vector2i, CellMapCell> &E : cell_map) {
			CellSetSource *source = *cell_set->get_source(E.value.source_id);
			if (!source || !source->has_cell(E.value.get_atlas_coords()) || !source->has_alternative_cell(E.value.get_atlas_coords(), E.value.alternative_cell)) {
				coords.insert(E.key);
			}
		}
		for (const Vector2i &E : coords) {
			set_cell(i, E, CellSet::INVALID_SOURCE, CellSetSource::INVALID_ATLAS_COORDS, CellSetSource::INVALID_cell_ALTERNATIVE);
		}
	}
}

void CellMap::_build_runtime_update_cell_data(SelfList<CellMapQuadrant>::List &r_dirty_quadrant_list) {
	if (GDVIRTUAL_IS_OVERRIDDEN(_use_cell_data_runtime_update) && GDVIRTUAL_IS_OVERRIDDEN(_cell_data_runtime_update)) {
		SelfList<CellMapQuadrant> *q_list_element = r_dirty_quadrant_list.first();
		while (q_list_element) {
			CellMapQuadrant &q = *q_list_element->self();
			// Iterate over the cells of the quadrant.
			for (const KeyValue<Vector2, Vector2i> &E_cell : q.local_to_map) {
				CellMapCell c = get_cell(q.layer, E_cell.value, true);

				CellSetSource *source;
				if (cell_set->has_source(c.source_id)) {
					source = *cell_set->get_source(c.source_id);

					if (!source->has_cell(c.get_atlas_coords()) || !source->has_alternative_cell(c.get_atlas_coords(), c.alternative_cell)) {
						continue;
					}

					CellSetAtlasSource *atlas_source = Object::cast_to<CellSetAtlasSource>(source);
					if (atlas_source) {
						bool ret = false;
						if (GDVIRTUAL_CALL(_use_cell_data_runtime_update, q.layer, E_cell.value, ret) && ret) {
							CellData *cell_data = atlas_source->get_cell_data(c.get_atlas_coords(), c.alternative_cell);

							// Create the runtime CellData.
							CellData *cell_data_runtime_use = cell_data->duplicate();
							cell_data->set_allow_transform(true);
							q.runtime_cell_data_cache[E_cell.value] = cell_data_runtime_use;

							GDVIRTUAL_CALL(_cell_data_runtime_update, q.layer, E_cell.value, cell_data_runtime_use);
						}
					}
				}
			}
			q_list_element = q_list_element->next();
		}
	}
}

#ifdef TOOLS_ENABLED
Rect2 CellMap::_edit_get_rect() const {
	// Return the visible rect of the cellmap
	const_cast<CellMap *>(this)->_recompute_rect_cache();
	return rect_cache;
}
#endif

void CellMap::clear_layer(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	// Remove all cells.
	_clear_layer_internals(p_layer);
	layers[p_layer].cell_map.clear();
	_recreate_layer_internals(p_layer);
	used_rect_cache_dirty = true;
}

void CellMap::clear() {
	// Remove all cells.
	_clear_internals();
	for (CellMapLayer &layer : layers) {
		layer.cell_map.clear();
	}
	_recreate_internals();
	used_rect_cache_dirty = true;
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

void CellMap::_set_cell_data(int p_layer, const Vector<int> &p_data) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	ERR_FAIL_COND(format > FORMAT_3);

	// Set data for a given cell from raw data.

	int c = p_data.size();
	const int *r = p_data.ptr();

	int offset = (format >= FORMAT_2) ? 3 : 2;
	ERR_FAIL_COND_MSG(c % offset != 0, vformat("Corrupted cell data. Got size: %s. Expected modulo: %s", offset));

	clear_layer(p_layer);

#ifdef DISABLE_DEPRECATED
	ERR_FAIL_COND_MSG(format != FORMAT_3, vformat("Cannot handle deprecated CellMap data format version %d. This Godot version was compiled with no support for deprecated data.", format));
#endif

	for (int i = 0; i < c; i += offset) {
		const uint8_t *ptr = (const uint8_t *)&r[i];
		uint8_t local[12];
		for (int j = 0; j < ((format >= FORMAT_2) ? 12 : 8); j++) {
			local[j] = ptr[j];
		}

#ifdef BIG_ENDIAN_ENABLED

		SWAP(local[0], local[3]);
		SWAP(local[1], local[2]);
		SWAP(local[4], local[7]);
		SWAP(local[5], local[6]);
		//TODO: ask someone to check this...
		if (FORMAT >= FORMAT_2) {
			SWAP(local[8], local[11]);
			SWAP(local[9], local[10]);
		}
#endif
		// Extracts position in CellMap.
		int16_t x = decode_uint16(&local[0]);
		int16_t y = decode_uint16(&local[2]);

		if (format == FORMAT_3) {
			uint16_t source_id = decode_uint16(&local[4]);
			uint16_t atlas_coords_x = decode_uint16(&local[6]);
			uint16_t atlas_coords_y = decode_uint16(&local[8]);
			uint16_t alternative_cell = decode_uint16(&local[10]);
			set_cell(p_layer, Vector2i(x, y), source_id, Vector2i(atlas_coords_x, atlas_coords_y), alternative_cell);
		} else {
#ifndef DISABLE_DEPRECATED
			// Previous decated format.

			uint32_t v = decode_uint32(&local[4]);
			// Extract the transform flags that used to be in the cellmap.
			bool flip_h = v & (1UL << 29);
			bool flip_v = v & (1UL << 30);
			bool transpose = v & (1UL << 31);
			v &= (1UL << 29) - 1;

			// Extract autocell/atlas coords.
			int16_t coord_x = 0;
			int16_t coord_y = 0;
			if (format == FORMAT_2) {
				coord_x = decode_uint16(&local[8]);
				coord_y = decode_uint16(&local[10]);
			}

			if (cell_set.is_valid()) {
				Array a = cell_set->compatibility_cellmap_map(v, Vector2i(coord_x, coord_y), flip_h, flip_v, transpose);
				if (a.size() == 3) {
					set_cell(p_layer, Vector2i(x, y), a[0], a[1], a[2]);
				} else {
					ERR_PRINT(vformat("No valid cell in Cellset for: cell:%s coords:%s flip_h:%s flip_v:%s transpose:%s", v, Vector2i(coord_x, coord_y), flip_h, flip_v, transpose));
				}
			} else {
				int compatibility_alternative_cell = ((int)flip_h) + ((int)flip_v << 1) + ((int)transpose << 2);
				set_cell(p_layer, Vector2i(x, y), v, Vector2i(coord_x, coord_y), compatibility_alternative_cell);
			}
#endif
		}
	}
	emit_signal(SNAME("changed"));
}

Vector<int> CellMap::_get_cell_data(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), Vector<int>());

	// Export cell data to raw format
	const HashMap<Vector2i, CellMapCell> &cell_map = layers[p_layer].cell_map;
	Vector<int> cell_data;
	cell_data.resize(cell_map.size() * 3);
	int *w = cell_data.ptrw();

	// Save in highest format

	int idx = 0;
	for (const KeyValue<Vector2i, CellMapCell> &E : cell_map) {
		uint8_t *ptr = (uint8_t *)&w[idx];
		encode_uint16((int16_t)(E.key.x), &ptr[0]);
		encode_uint16((int16_t)(E.key.y), &ptr[2]);
		encode_uint16(E.value.source_id, &ptr[4]);
		encode_uint16(E.value.coord_x, &ptr[6]);
		encode_uint16(E.value.coord_y, &ptr[8]);
		encode_uint16(E.value.alternative_cell, &ptr[10]);
		idx += 3;
	}

	return cell_data;
}

bool CellMap::_set(const StringName &p_name, const Variant &p_value) {
	Vector<String> components = String(p_name).split("/", true, 2);
	if (p_name == "format") {
		if (p_value.get_type() == Variant::INT) {
			format = (DataFormat)(p_value.operator int64_t()); // Set format used for loading
			return true;
		}
	} else if (p_name == "cell_data") { // Kept for compatibility reasons.
		if (p_value.is_array()) {
			if (layers.size() < 1) {
				layers.resize(1);
			}
			_set_cell_data(0, p_value);
			return true;
		}
		return false;
	} else if (components.size() == 2 && components[0].begins_with("layer_") && components[0].trim_prefix("layer_").is_valid_int()) {
		int index = components[0].trim_prefix("layer_").to_int();
		if (index < 0) {
			return false;
		}

		if (index >= (int)layers.size()) {
			_clear_internals();
			while (index >= (int)layers.size()) {
				layers.push_back(CellMapLayer());
			}
			_recreate_internals();

			notify_property_list_changed();
			emit_signal(SNAME("changed"));
			update_configuration_warnings();
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
		} else if (components[1] == "cell_data") {
			_set_cell_data(index, p_value);
			return true;
		} else {
			return false;
		}
	}
	return false;
}

bool CellMap::_get(const StringName &p_name, Variant &r_ret) const {
	Vector<String> components = String(p_name).split("/", true, 2);
	if (p_name == "format") {
		r_ret = FORMAT_3; // When saving, always save highest format
		return true;
	} else if (components.size() == 2 && components[0].begins_with("layer_") && components[0].trim_prefix("layer_").is_valid_int()) {
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
		} else if (components[1] == "cell_data") {
			r_ret = _get_cell_data(index);
			return true;
		} else {
			return false;
		}
	}
	return false;
}

void CellMap::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "format", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
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

Vector2 CellMap::map_to_local(const Vector2i &p_pos) const {
	// SHOULD RETURN THE CENTER OF THE CELL
	ERR_FAIL_COND_V(!cell_set.is_valid(), Vector2());

	Vector2 ret = p_pos;
	CellSet::CellShape cell_shape = cell_set->get_cell_shape();
	CellSet::CellOffsetAxis cell_offset_axis = cell_set->get_cell_offset_axis();

	if (cell_shape == CellSet::CELL_SHAPE_HALF_OFFSET_SQUARE || cell_shape == CellSet::CELL_SHAPE_HEXAGON || cell_shape == CellSet::CELL_SHAPE_ISOMETRIC) {
		// Technically, those 3 shapes are equivalent, as they are basically half-offset, but with different levels or overlap.
		// square = no overlap, hexagon = 0.25 overlap, isometric = 0.5 overlap
		if (cell_offset_axis == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
			switch (cell_set->get_cell_layout()) {
				case CellSet::CELL_LAYOUT_STACKED:
					ret = Vector2(ret.x + (Math::posmod(ret.y, 2) == 0 ? 0.0 : 0.5), ret.y);
					break;
				case CellSet::CELL_LAYOUT_STACKED_OFFSET:
					ret = Vector2(ret.x + (Math::posmod(ret.y, 2) == 1 ? 0.0 : 0.5), ret.y);
					break;
				case CellSet::CELL_LAYOUT_STAIRS_RIGHT:
					ret = Vector2(ret.x + ret.y / 2, ret.y);
					break;
				case CellSet::CELL_LAYOUT_STAIRS_DOWN:
					ret = Vector2(ret.x / 2, ret.y * 2 + ret.x);
					break;
				case CellSet::CELL_LAYOUT_DIAMOND_RIGHT:
					ret = Vector2((ret.x + ret.y) / 2, ret.y - ret.x);
					break;
				case CellSet::CELL_LAYOUT_DIAMOND_DOWN:
					ret = Vector2((ret.x - ret.y) / 2, ret.y + ret.x);
					break;
			}
		} else { // CELL_OFFSET_AXIS_VERTICAL
			switch (cell_set->get_cell_layout()) {
				case CellSet::CELL_LAYOUT_STACKED:
					ret = Vector2(ret.x, ret.y + (Math::posmod(ret.x, 2) == 0 ? 0.0 : 0.5));
					break;
				case CellSet::CELL_LAYOUT_STACKED_OFFSET:
					ret = Vector2(ret.x, ret.y + (Math::posmod(ret.x, 2) == 1 ? 0.0 : 0.5));
					break;
				case CellSet::CELL_LAYOUT_STAIRS_RIGHT:
					ret = Vector2(ret.x * 2 + ret.y, ret.y / 2);
					break;
				case CellSet::CELL_LAYOUT_STAIRS_DOWN:
					ret = Vector2(ret.x, ret.y + ret.x / 2);
					break;
				case CellSet::CELL_LAYOUT_DIAMOND_RIGHT:
					ret = Vector2(ret.x + ret.y, (ret.y - ret.x) / 2);
					break;
				case CellSet::CELL_LAYOUT_DIAMOND_DOWN:
					ret = Vector2(ret.x - ret.y, (ret.y + ret.x) / 2);
					break;
			}
		}
	}

	// Multiply by the overlapping ratio
	double overlapping_ratio = 1.0;
	if (cell_offset_axis == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
		if (cell_shape == CellSet::CELL_SHAPE_ISOMETRIC) {
			overlapping_ratio = 0.5;
		} else if (cell_shape == CellSet::CELL_SHAPE_HEXAGON) {
			overlapping_ratio = 0.75;
		}
		ret.y *= overlapping_ratio;
	} else { // CELL_OFFSET_AXIS_VERTICAL
		if (cell_shape == CellSet::CELL_SHAPE_ISOMETRIC) {
			overlapping_ratio = 0.5;
		} else if (cell_shape == CellSet::CELL_SHAPE_HEXAGON) {
			overlapping_ratio = 0.75;
		}
		ret.x *= overlapping_ratio;
	}

	return (ret + Vector2(0.5, 0.5)) * cell_set->get_cell_size();
}

Vector2i CellMap::local_to_map(const Vector2 &p_local_position) const {
	ERR_FAIL_COND_V(!cell_set.is_valid(), Vector2i());

	Vector2 ret = p_local_position;
	ret /= cell_set->get_cell_size();

	CellSet::CellShape cell_shape = cell_set->get_cell_shape();
	CellSet::CellOffsetAxis cell_offset_axis = cell_set->get_cell_offset_axis();
	CellSet::CellLayout cell_layout = cell_set->get_cell_layout();

	// Divide by the overlapping ratio
	double overlapping_ratio = 1.0;
	if (cell_offset_axis == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
		if (cell_shape == CellSet::CELL_SHAPE_ISOMETRIC) {
			overlapping_ratio = 0.5;
		} else if (cell_shape == CellSet::CELL_SHAPE_HEXAGON) {
			overlapping_ratio = 0.75;
		}
		ret.y /= overlapping_ratio;
	} else { // CELL_OFFSET_AXIS_VERTICAL
		if (cell_shape == CellSet::CELL_SHAPE_ISOMETRIC) {
			overlapping_ratio = 0.5;
		} else if (cell_shape == CellSet::CELL_SHAPE_HEXAGON) {
			overlapping_ratio = 0.75;
		}
		ret.x /= overlapping_ratio;
	}

	// For each half-offset shape, we check if we are in the corner of the cell, and thus should correct the local position accordingly.
	if (cell_shape == CellSet::CELL_SHAPE_HALF_OFFSET_SQUARE || cell_shape == CellSet::CELL_SHAPE_HEXAGON || cell_shape == CellSet::CELL_SHAPE_ISOMETRIC) {
		// Technically, those 3 shapes are equivalent, as they are basically half-offset, but with different levels or overlap.
		// square = no overlap, hexagon = 0.25 overlap, isometric = 0.5 overlap
		if (cell_offset_axis == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
			// Smart floor of the position
			Vector2 raw_pos = ret;
			if (Math::posmod(Math::floor(ret.y), 2) ^ (cell_layout == CellSet::CELL_LAYOUT_STACKED_OFFSET)) {
				ret = Vector2(Math::floor(ret.x + 0.5) - 0.5, Math::floor(ret.y));
			} else {
				ret = ret.floor();
			}

			// Compute the cell offset, and if we might the output for a neighbor top cell
			Vector2 in_cell_pos = raw_pos - ret;
			bool in_top_left_triangle = (in_cell_pos - Vector2(0.5, 0.0)).cross(Vector2(-0.5, 1.0 / overlapping_ratio - 1)) <= 0;
			bool in_top_right_triangle = (in_cell_pos - Vector2(0.5, 0.0)).cross(Vector2(0.5, 1.0 / overlapping_ratio - 1)) > 0;

			switch (cell_layout) {
				case CellSet::CELL_LAYOUT_STACKED:
					ret = ret.floor();
					if (in_top_left_triangle) {
						ret += Vector2i(Math::posmod(Math::floor(ret.y), 2) ? 0 : -1, -1);
					} else if (in_top_right_triangle) {
						ret += Vector2i(Math::posmod(Math::floor(ret.y), 2) ? 1 : 0, -1);
					}
					break;
				case CellSet::CELL_LAYOUT_STACKED_OFFSET:
					ret = ret.floor();
					if (in_top_left_triangle) {
						ret += Vector2i(Math::posmod(Math::floor(ret.y), 2) ? -1 : 0, -1);
					} else if (in_top_right_triangle) {
						ret += Vector2i(Math::posmod(Math::floor(ret.y), 2) ? 0 : 1, -1);
					}
					break;
				case CellSet::CELL_LAYOUT_STAIRS_RIGHT:
					ret = Vector2(ret.x - ret.y / 2, ret.y).floor();
					if (in_top_left_triangle) {
						ret += Vector2i(0, -1);
					} else if (in_top_right_triangle) {
						ret += Vector2i(1, -1);
					}
					break;
				case CellSet::CELL_LAYOUT_STAIRS_DOWN:
					ret = Vector2(ret.x * 2, ret.y / 2 - ret.x).floor();
					if (in_top_left_triangle) {
						ret += Vector2i(-1, 0);
					} else if (in_top_right_triangle) {
						ret += Vector2i(1, -1);
					}
					break;
				case CellSet::CELL_LAYOUT_DIAMOND_RIGHT:
					ret = Vector2(ret.x - ret.y / 2, ret.y / 2 + ret.x).floor();
					if (in_top_left_triangle) {
						ret += Vector2i(0, -1);
					} else if (in_top_right_triangle) {
						ret += Vector2i(1, 0);
					}
					break;
				case CellSet::CELL_LAYOUT_DIAMOND_DOWN:
					ret = Vector2(ret.x + ret.y / 2, ret.y / 2 - ret.x).floor();
					if (in_top_left_triangle) {
						ret += Vector2i(-1, 0);
					} else if (in_top_right_triangle) {
						ret += Vector2i(0, -1);
					}
					break;
			}
		} else { // CELL_OFFSET_AXIS_VERTICAL
			// Smart floor of the position
			Vector2 raw_pos = ret;
			if (Math::posmod(Math::floor(ret.x), 2) ^ (cell_layout == CellSet::CELL_LAYOUT_STACKED_OFFSET)) {
				ret = Vector2(Math::floor(ret.x), Math::floor(ret.y + 0.5) - 0.5);
			} else {
				ret = ret.floor();
			}

			// Compute the cell offset, and if we might the output for a neighbor top cell
			Vector2 in_cell_pos = raw_pos - ret;
			bool in_top_left_triangle = (in_cell_pos - Vector2(0.0, 0.5)).cross(Vector2(1.0 / overlapping_ratio - 1, -0.5)) > 0;
			bool in_bottom_left_triangle = (in_cell_pos - Vector2(0.0, 0.5)).cross(Vector2(1.0 / overlapping_ratio - 1, 0.5)) <= 0;

			switch (cell_layout) {
				case CellSet::CELL_LAYOUT_STACKED:
					ret = ret.floor();
					if (in_top_left_triangle) {
						ret += Vector2i(-1, Math::posmod(Math::floor(ret.x), 2) ? 0 : -1);
					} else if (in_bottom_left_triangle) {
						ret += Vector2i(-1, Math::posmod(Math::floor(ret.x), 2) ? 1 : 0);
					}
					break;
				case CellSet::CELL_LAYOUT_STACKED_OFFSET:
					ret = ret.floor();
					if (in_top_left_triangle) {
						ret += Vector2i(-1, Math::posmod(Math::floor(ret.x), 2) ? -1 : 0);
					} else if (in_bottom_left_triangle) {
						ret += Vector2i(-1, Math::posmod(Math::floor(ret.x), 2) ? 0 : 1);
					}
					break;
				case CellSet::CELL_LAYOUT_STAIRS_RIGHT:
					ret = Vector2(ret.x / 2 - ret.y, ret.y * 2).floor();
					if (in_top_left_triangle) {
						ret += Vector2i(0, -1);
					} else if (in_bottom_left_triangle) {
						ret += Vector2i(-1, 1);
					}
					break;
				case CellSet::CELL_LAYOUT_STAIRS_DOWN:
					ret = Vector2(ret.x, ret.y - ret.x / 2).floor();
					if (in_top_left_triangle) {
						ret += Vector2i(-1, 0);
					} else if (in_bottom_left_triangle) {
						ret += Vector2i(-1, 1);
					}
					break;
				case CellSet::CELL_LAYOUT_DIAMOND_RIGHT:
					ret = Vector2(ret.x / 2 - ret.y, ret.y + ret.x / 2).floor();
					if (in_top_left_triangle) {
						ret += Vector2i(0, -1);
					} else if (in_bottom_left_triangle) {
						ret += Vector2i(-1, 0);
					}
					break;
				case CellSet::CELL_LAYOUT_DIAMOND_DOWN:
					ret = Vector2(ret.x / 2 + ret.y, ret.y - ret.x / 2).floor();
					if (in_top_left_triangle) {
						ret += Vector2i(-1, 0);
					} else if (in_bottom_left_triangle) {
						ret += Vector2i(0, 1);
					}
					break;
			}
		}
	} else {
		ret = (ret + Vector2(0.00005, 0.00005)).floor();
	}
	return Vector2i(ret);
}

bool CellMap::is_existing_neighbor(CellSet::CellNeighbor p_cell_neighbor) const {
	ERR_FAIL_COND_V(!cell_set.is_valid(), false);

	CellSet::CellShape shape = cell_set->get_cell_shape();
	if (shape == CellSet::CELL_SHAPE_SQUARE) {
		return p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_SIDE ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_SIDE ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_CORNER ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_SIDE ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_SIDE ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_CORNER;

	} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC) {
		return p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER ||
				p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE;
	} else {
		if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
			return p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE;
		} else {
			return p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_SIDE ||
					p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE;
		}
	}
}

Vector2i CellMap::get_neighbor_cell(const Vector2i &p_coords, CellSet::CellNeighbor p_cell_neighbor) const {
	ERR_FAIL_COND_V(!cell_set.is_valid(), p_coords);

	CellSet::CellShape shape = cell_set->get_cell_shape();
	if (shape == CellSet::CELL_SHAPE_SQUARE) {
		switch (p_cell_neighbor) {
			case CellSet::CELL_NEIGHBOR_RIGHT_SIDE:
				return p_coords + Vector2i(1, 0);
			case CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER:
				return p_coords + Vector2i(1, 1);
			case CellSet::CELL_NEIGHBOR_BOTTOM_SIDE:
				return p_coords + Vector2i(0, 1);
			case CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_CORNER:
				return p_coords + Vector2i(-1, 1);
			case CellSet::CELL_NEIGHBOR_LEFT_SIDE:
				return p_coords + Vector2i(-1, 0);
			case CellSet::CELL_NEIGHBOR_TOP_LEFT_CORNER:
				return p_coords + Vector2i(-1, -1);
			case CellSet::CELL_NEIGHBOR_TOP_SIDE:
				return p_coords + Vector2i(0, -1);
			case CellSet::CELL_NEIGHBOR_TOP_RIGHT_CORNER:
				return p_coords + Vector2i(1, -1);
			default:
				ERR_FAIL_V(p_coords);
		}
	} else { // Half-offset shapes (square and hexagon)
		switch (cell_set->get_cell_layout()) {
			case CellSet::CELL_LAYOUT_STACKED: {
				if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
					bool is_offset = p_coords.y % 2;
					if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) ||
							(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_SIDE)) {
						return p_coords + Vector2i(1, 0);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
						return p_coords + Vector2i(is_offset ? 1 : 0, 1);
					} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) {
						return p_coords + Vector2i(0, 2);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
						return p_coords + Vector2i(is_offset ? 0 : -1, 1);
					} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) ||
							(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_SIDE)) {
						return p_coords + Vector2i(-1, 0);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
						return p_coords + Vector2i(is_offset ? 0 : -1, -1);
					} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) {
						return p_coords + Vector2i(0, -2);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
						return p_coords + Vector2i(is_offset ? 1 : 0, -1);
					} else {
						ERR_FAIL_V(p_coords);
					}
				} else {
					bool is_offset = p_coords.x % 2;

					if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) ||
							(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)) {
						return p_coords + Vector2i(0, 1);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
						return p_coords + Vector2i(1, is_offset ? 1 : 0);
					} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) {
						return p_coords + Vector2i(2, 0);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
						return p_coords + Vector2i(1, is_offset ? 0 : -1);
					} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) ||
							(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_SIDE)) {
						return p_coords + Vector2i(0, -1);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
						return p_coords + Vector2i(-1, is_offset ? 0 : -1);
					} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) {
						return p_coords + Vector2i(-2, 0);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
						return p_coords + Vector2i(-1, is_offset ? 1 : 0);
					} else {
						ERR_FAIL_V(p_coords);
					}
				}
			} break;
			case CellSet::CELL_LAYOUT_STACKED_OFFSET: {
				if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
					bool is_offset = p_coords.y % 2;

					if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) ||
							(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_SIDE)) {
						return p_coords + Vector2i(1, 0);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
						return p_coords + Vector2i(is_offset ? 0 : 1, 1);
					} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) {
						return p_coords + Vector2i(0, 2);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
						return p_coords + Vector2i(is_offset ? -1 : 0, 1);
					} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) ||
							(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_SIDE)) {
						return p_coords + Vector2i(-1, 0);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
						return p_coords + Vector2i(is_offset ? -1 : 0, -1);
					} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) {
						return p_coords + Vector2i(0, -2);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
						return p_coords + Vector2i(is_offset ? 0 : 1, -1);
					} else {
						ERR_FAIL_V(p_coords);
					}
				} else {
					bool is_offset = p_coords.x % 2;

					if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) ||
							(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)) {
						return p_coords + Vector2i(0, 1);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
						return p_coords + Vector2i(1, is_offset ? 0 : 1);
					} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) {
						return p_coords + Vector2i(2, 0);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
						return p_coords + Vector2i(1, is_offset ? -1 : 0);
					} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) ||
							(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_SIDE)) {
						return p_coords + Vector2i(0, -1);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
						return p_coords + Vector2i(-1, is_offset ? -1 : 0);
					} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) {
						return p_coords + Vector2i(-2, 0);
					} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
						return p_coords + Vector2i(-1, is_offset ? 0 : 1);
					} else {
						ERR_FAIL_V(p_coords);
					}
				}
			} break;
			case CellSet::CELL_LAYOUT_STAIRS_RIGHT:
			case CellSet::CELL_LAYOUT_STAIRS_DOWN: {
				if ((cell_set->get_cell_layout() == CellSet::CELL_LAYOUT_STAIRS_RIGHT) ^ (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_VERTICAL)) {
					if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
						if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_SIDE)) {
							return p_coords + Vector2i(1, 0);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
							return p_coords + Vector2i(0, 1);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) {
							return p_coords + Vector2i(-1, 2);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 1);
						} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_SIDE)) {
							return p_coords + Vector2i(-1, 0);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
							return p_coords + Vector2i(0, -1);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) {
							return p_coords + Vector2i(1, -2);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
							return p_coords + Vector2i(1, -1);
						} else {
							ERR_FAIL_V(p_coords);
						}

					} else {
						if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)) {
							return p_coords + Vector2i(0, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
							return p_coords + Vector2i(1, 0);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) {
							return p_coords + Vector2i(2, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
							return p_coords + Vector2i(1, -1);
						} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_SIDE)) {
							return p_coords + Vector2i(0, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 0);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) {
							return p_coords + Vector2i(-2, 1);

						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 1);
						} else {
							ERR_FAIL_V(p_coords);
						}
					}
				} else {
					if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
						if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_SIDE)) {
							return p_coords + Vector2i(2, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
							return p_coords + Vector2i(1, 0);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) {
							return p_coords + Vector2i(0, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 1);
						} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_SIDE)) {
							return p_coords + Vector2i(-2, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 0);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) {
							return p_coords + Vector2i(0, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
							return p_coords + Vector2i(1, -1);
						} else {
							ERR_FAIL_V(p_coords);
						}

					} else {
						if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)) {
							return p_coords + Vector2i(-1, 2);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
							return p_coords + Vector2i(0, 1);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) {
							return p_coords + Vector2i(1, 0);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
							return p_coords + Vector2i(1, -1);
						} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_SIDE)) {
							return p_coords + Vector2i(1, -2);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
							return p_coords + Vector2i(0, -1);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) {
							return p_coords + Vector2i(-1, 0);

						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 1);
						} else {
							ERR_FAIL_V(p_coords);
						}
					}
				}
			} break;
			case CellSet::CELL_LAYOUT_DIAMOND_RIGHT:
			case CellSet::CELL_LAYOUT_DIAMOND_DOWN: {
				if ((cell_set->get_cell_layout() == CellSet::CELL_LAYOUT_DIAMOND_RIGHT) ^ (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_VERTICAL)) {
					if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
						if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_SIDE)) {
							return p_coords + Vector2i(1, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
							return p_coords + Vector2i(0, 1);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) {
							return p_coords + Vector2i(-1, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 0);
						} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_SIDE)) {
							return p_coords + Vector2i(-1, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
							return p_coords + Vector2i(0, -1);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) {
							return p_coords + Vector2i(1, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
							return p_coords + Vector2i(1, 0);
						} else {
							ERR_FAIL_V(p_coords);
						}

					} else {
						if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)) {
							return p_coords + Vector2i(1, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
							return p_coords + Vector2i(1, 0);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) {
							return p_coords + Vector2i(1, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
							return p_coords + Vector2i(0, -1);
						} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_SIDE)) {
							return p_coords + Vector2i(-1, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 0);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) {
							return p_coords + Vector2i(-1, 1);

						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
							return p_coords + Vector2i(0, 1);
						} else {
							ERR_FAIL_V(p_coords);
						}
					}
				} else {
					if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
						if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_SIDE)) {
							return p_coords + Vector2i(1, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
							return p_coords + Vector2i(1, 0);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) {
							return p_coords + Vector2i(1, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
							return p_coords + Vector2i(0, 1);
						} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_SIDE)) {
							return p_coords + Vector2i(-1, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 0);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) {
							return p_coords + Vector2i(-1, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
							return p_coords + Vector2i(0, -1);
						} else {
							ERR_FAIL_V(p_coords);
						}

					} else {
						if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_SIDE)) {
							return p_coords + Vector2i(-1, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE) {
							return p_coords + Vector2i(0, 1);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_RIGHT_CORNER) {
							return p_coords + Vector2i(1, 1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE) {
							return p_coords + Vector2i(1, 0);
						} else if ((shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_CORNER) ||
								(shape != CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_SIDE)) {
							return p_coords + Vector2i(1, -1);
						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE) {
							return p_coords + Vector2i(0, -1);
						} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC && p_cell_neighbor == CellSet::CELL_NEIGHBOR_LEFT_CORNER) {
							return p_coords + Vector2i(-1, -1);

						} else if (p_cell_neighbor == CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE) {
							return p_coords + Vector2i(-1, 0);
						} else {
							ERR_FAIL_V(p_coords);
						}
					}
				}
			} break;
			default:
				ERR_FAIL_V(p_coords);
		}
	}
}

TypedArray<Vector2i> CellMap::get_used_cells(int p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), TypedArray<Vector2i>());

	// Returns the cells used in the cellmap.
	TypedArray<Vector2i> a;
	a.resize(layers[p_layer].cell_map.size());
	int i = 0;
	for (const KeyValue<Vector2i, CellMapCell> &E : layers[p_layer].cell_map) {
		Vector2i p(E.key.x, E.key.y);
		a[i++] = p;
	}

	return a;
}

TypedArray<Vector2i> CellMap::get_used_cells_by_id(int p_layer, int p_source_id, const Vector2i p_atlas_coords, int p_alternative_cell) const {
	ERR_FAIL_INDEX_V(p_layer, (int)layers.size(), TypedArray<Vector2i>());

	// Returns the cells used in the cellmap.
	TypedArray<Vector2i> a;
	for (const KeyValue<Vector2i, CellMapCell> &E : layers[p_layer].cell_map) {
		if ((p_source_id == CellSet::INVALID_SOURCE || p_source_id == E.value.source_id) &&
				(p_atlas_coords == CellSetSource::INVALID_ATLAS_COORDS || p_atlas_coords == E.value.get_atlas_coords()) &&
				(p_alternative_cell == CellSetSource::INVALID_cell_ALTERNATIVE || p_alternative_cell == E.value.alternative_cell)) {
			a.push_back(E.key);
		}
	}

	return a;
}

Rect2i CellMap::get_used_rect() { // Not const because of cache
	// Return the rect of the currently used area
	if (used_rect_cache_dirty) {
		bool first = true;
		used_rect_cache = Rect2i();

		for (unsigned int i = 0; i < layers.size(); i++) {
			const HashMap<Vector2i, CellMapCell> &cell_map = layers[i].cell_map;
			if (cell_map.size() > 0) {
				if (first) {
					used_rect_cache = Rect2i(cell_map.begin()->key.x, cell_map.begin()->key.y, 0, 0);
					first = false;
				}

				for (const KeyValue<Vector2i, CellMapCell> &E : cell_map) {
					used_rect_cache.expand_to(Vector2i(E.key.x, E.key.y));
				}
			}
		}

		if (!first) { // first is true if every layer is empty.
			used_rect_cache.size += Vector2i(1, 1); // The cache expands to top-left coordinate, so we add one full cell.
		}
		used_rect_cache_dirty = false;
	}

	return used_rect_cache;
}

TypedArray<Vector2i> CellMap::get_surrounding_cells(const Vector2i &coords) {
	if (!cell_set.is_valid()) {
		return TypedArray<Vector2i>();
	}

	TypedArray<Vector2i> around;
	CellSet::CellShape shape = cell_set->get_cell_shape();
	if (shape == CellSet::CELL_SHAPE_SQUARE) {
		around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_RIGHT_SIDE));
		around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_BOTTOM_SIDE));
		around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_LEFT_SIDE));
		around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_TOP_SIDE));
	} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC) {
		around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE));
		around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE));
		around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE));
		around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE));
	} else {
		if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_RIGHT_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_LEFT_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE));
		} else {
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_BOTTOM_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_TOP_SIDE));
			around.push_back(get_neighbor_cell(coords, CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE));
		}
	}

	return around;
}

void CellMap::draw_cells_outline(Control *p_control, const RBSet<Vector2i> &p_cells, Color p_color, Transform2D p_transform) {
	if (!cell_set.is_valid()) {
		return;
	}

	// Create a set.
	Vector2i cell_size = cell_set->get_cell_size();
	Vector<Vector2> polygon = cell_set->get_cell_shape_polygon();
	CellSet::CellShape shape = cell_set->get_cell_shape();

	for (const Vector2i &E : p_cells) {
		Vector2 center = map_to_local(E);

#define DRAW_SIDE_IF_NEEDED(side, polygon_index_from, polygon_index_to)                     \
	if (!p_cells.has(get_neighbor_cell(E, side))) {                                         \
		Vector2 from = p_transform.xform(center + polygon[polygon_index_from] * cell_size); \
		Vector2 to = p_transform.xform(center + polygon[polygon_index_to] * cell_size);     \
		p_control->draw_line(from, to, p_color);                                            \
	}

		if (shape == CellSet::CELL_SHAPE_SQUARE) {
			DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_RIGHT_SIDE, 1, 2);
			DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_BOTTOM_SIDE, 2, 3);
			DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_LEFT_SIDE, 3, 0);
			DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_TOP_SIDE, 0, 1);
		} else if (shape == CellSet::CELL_SHAPE_ISOMETRIC) {
			DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE, 2, 3);
			DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE, 1, 2);
			DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE, 0, 1);
			DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE, 3, 0);
		} else {
			if (cell_set->get_cell_offset_axis() == CellSet::CELL_OFFSET_AXIS_HORIZONTAL) {
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE, 3, 4);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE, 2, 3);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_LEFT_SIDE, 1, 2);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE, 0, 1);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE, 5, 0);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_RIGHT_SIDE, 4, 5);
			} else {
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE, 3, 4);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_BOTTOM_SIDE, 4, 5);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_BOTTOM_LEFT_SIDE, 5, 0);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_TOP_LEFT_SIDE, 0, 1);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_TOP_SIDE, 1, 2);
				DRAW_SIDE_IF_NEEDED(CellSet::CELL_NEIGHBOR_TOP_RIGHT_SIDE, 2, 3);
			}
		}
	}
#undef DRAW_SIDE_IF_NEEDED
}

PackedStringArray CellMap::get_configuration_warnings() const {
	PackedStringArray warnings = Node::get_configuration_warnings();

	// Retrieve the set of Z index values with a Y-sorted layer.
	RBSet<int> y_sorted_z_index;
	for (const CellMapLayer &layer : layers) {
		if (layer.y_sort_enabled) {
			y_sorted_z_index.insert(layer.z_index);
		}
	}

	// Check if we have a non-sorted layer in a Z-index with a Y-sorted layer.
	for (const CellMapLayer &layer : layers) {
		if (!layer.y_sort_enabled && y_sorted_z_index.has(layer.z_index)) {
			warnings.push_back(RTR("A Y-sorted layer has the same Z-index value as a not Y-sorted layer.\nThis may lead to unwanted behaviors, as a layer that is not Y-sorted will be Y-sorted as a whole with cells from Y-sorted layers."));
			break;
		}
	}

	// Check if Y-sort is enabled on a layer but not on the node.
	if (!is_y_sort_enabled()) {
		for (const CellMapLayer &layer : layers) {
			if (layer.y_sort_enabled) {
				warnings.push_back(RTR("A CellMap layer is set as Y-sorted, but Y-sort is not enabled on the CellMap node itself."));
				break;
			}
		}
	}

	// Check if we are in isometric mode without Y-sort enabled.
	if (cell_set.is_valid() && cell_set->get_cell_shape() == CellSet::CELL_SHAPE_ISOMETRIC) {
		bool warn = !is_y_sort_enabled();
		if (!warn) {
			for (const CellMapLayer &layer : layers) {
				if (!layer.y_sort_enabled) {
					warn = true;
					break;
				}
			}
		}

		if (warn) {
			warnings.push_back(RTR("Isometric CellSet will likely not look as intended without Y-sort enabled for the CellMap and all of its layers."));
		}
	}

	return warnings;
}

void CellMap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_cellset", "cellset"), &CellMap::set_cellset);
	ClassDB::bind_method(D_METHOD("get_cellset"), &CellMap::get_cellset);

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

	ClassDB::bind_method(D_METHOD("set_cell", "layer", "coords", "source_id", "atlas_coords", "alternative_cell"), &CellMap::set_cell, DEFVAL(CellSet::INVALID_SOURCE), DEFVAL(CellSetSource::INVALID_ATLAS_COORDS), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("erase_cell", "layer", "coords"), &CellMap::erase_cell);
	ClassDB::bind_method(D_METHOD("get_cell_source_id", "layer", "coords", "use_proxies"), &CellMap::get_cell_source_id, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_cell_atlas_coords", "layer", "coords", "use_proxies"), &CellMap::get_cell_atlas_coords, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_cell_alternative_cell", "layer", "coords", "use_proxies"), &CellMap::get_cell_alternative_cell, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_cell_data", "layer", "coords", "use_proxies"), &CellMap::get_cell_data, DEFVAL(false));

	ClassDB::bind_method(D_METHOD("map_pattern", "position_in_cellmap", "coords_in_pattern", "pattern"), &CellMap::map_pattern);
	ClassDB::bind_method(D_METHOD("set_pattern", "layer", "position", "pattern"), &CellMap::set_pattern);

	ClassDB::bind_method(D_METHOD("set_cells_terrain_connect", "layer", "cells", "terrain_set", "terrain", "ignore_empty_terrains"), &CellMap::set_cells_terrain_connect, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("set_cells_terrain_path", "layer", "path", "terrain_set", "terrain", "ignore_empty_terrains"), &CellMap::set_cells_terrain_path, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("fix_invalid_cells"), &CellMap::fix_invalid_cells);
	ClassDB::bind_method(D_METHOD("clear_layer", "layer"), &CellMap::clear_layer);
	ClassDB::bind_method(D_METHOD("clear"), &CellMap::clear);

	ClassDB::bind_method(D_METHOD("force_update", "layer"), &CellMap::force_update, DEFVAL(-1));

	ClassDB::bind_method(D_METHOD("get_surrounding_cells", "coords"), &CellMap::get_surrounding_cells);

	ClassDB::bind_method(D_METHOD("get_used_cells", "layer"), &CellMap::get_used_cells);
	ClassDB::bind_method(D_METHOD("get_used_cells_by_id", "layer", "source_id", "atlas_coords", "alternative_cell"), &CellMap::get_used_cells_by_id, DEFVAL(CellSet::INVALID_SOURCE), DEFVAL(CellSetSource::INVALID_ATLAS_COORDS), DEFVAL(CellSetSource::INVALID_cell_ALTERNATIVE));
	ClassDB::bind_method(D_METHOD("get_used_rect"), &CellMap::get_used_rect);

	ClassDB::bind_method(D_METHOD("map_to_local", "map_position"), &CellMap::map_to_local);
	ClassDB::bind_method(D_METHOD("local_to_map", "local_position"), &CellMap::local_to_map);

	ClassDB::bind_method(D_METHOD("get_neighbor_cell", "coords", "neighbor"), &CellMap::get_neighbor_cell);

	ClassDB::bind_method(D_METHOD("_cell_set_changed_deferred_update"), &CellMap::_cell_set_changed_deferred_update);

	GDVIRTUAL_BIND(_use_cell_data_runtime_update, "layer", "coords");
	GDVIRTUAL_BIND(_cell_data_runtime_update, "layer", "coords", "cell_data");

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "cell_set", PROPERTY_HINT_RESOURCE_TYPE, "CellSet"), "set_cellset", "get_cellset");

	ADD_ARRAY("layers", "layer_");

	ADD_PROPERTY_DEFAULT("format", FORMAT_1);

	ADD_SIGNAL(MethodInfo("changed"));

}

void CellMap::_cell_set_changed() {
	emit_signal(SNAME("changed"));
	_cell_set_changed_deferred_update_needed = true;
}

void CellMap::_cell_set_changed_deferred_update() {
	if (_cell_set_changed_deferred_update_needed) {
		_clear_internals();
		_recreate_internals();
		_cell_set_changed_deferred_update_needed = false;
	}
}

CellMap::CellMap() {
	set_notify_transform(true);
	set_notify_local_transform(false);

	layers.resize(1);
}

CellMap::~CellMap() {
	if (cell_set.is_valid()) {
		cell_set->disconnect("changed", callable_mp(this, &CellMap::_cell_set_changed));
	}

	_clear_internals();
}
