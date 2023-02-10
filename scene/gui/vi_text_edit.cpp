void TextEdit::set_vi_enabled(const bool p_enabled) {
	if (vi_enabled == p_enabled) {
		return;
	}
	vi_enabled = p_enabled;
	queue_redraw();
}

bool TextEdit::is_vi_enabled() const {
	return vi_enabled;
}

void TextEdit::set_vi_mode(const int p_mode) {
	if (vi_mode == p_mode) {
		return;
	}
	vi_mode = p_mode;
	switch (vi_mode) {
		case ViMode::MODE_INSERT:
			set_editable(true);
			break;
		case ViMode::MODE_NORMAL:
			set_editable(false);
			break;
		case ViMode::MODE_VISUAL:
			break;
		case ViMode::MODE_COMAND:
			break;
	}
	queue_redraw();
}

int TextEdit::get_vi_mode() const {
	return vi_mode;
}

void TextEdit::handle_vi(const uint32_t p_unicode, const Vector<String> &p_text) {
/*
 * Create a buffer to hold input commands.
 * until a combination can be fulfilled, keep feeding the buffer
 * Then do a for loop even if the count is 0, if it is, assume it's one.
 * Jeff Venancius
 *
 *
 *
 *
 */
	current_op.text += p_text;
	current_op.to_column = retchar;
	current_op.to_line = retline;
	current_op.version = op.version;
	current_op.end_carets = carets;
}
void TextEdit::gui_input(const Ref<InputEvent> &p_gui_input) {
	ERR_FAIL_COND(p_gui_input.is_null());

	double prev_v_scroll = v_scroll->get_value();
	double prev_h_scroll = h_scroll->get_value();

	Ref<InputEventMouseButton> mb = p_gui_input;

	if (mb.is_valid()) {
		Vector2i mpos = mb->get_position();
		if (is_layout_rtl()) {
			mpos.x = get_size().x - mpos.x;
		}
		if (ime_text.length() != 0) {
			// Ignore mouse clicks in IME input mode.
			return;
		}

		if (mb->is_pressed()) {
			if (mb->get_button_index() == MouseButton::WHEEL_UP && !mb->is_command_or_control_pressed()) {
				if (mb->is_shift_pressed()) {
					h_scroll->set_value(h_scroll->get_value() - (100 * mb->get_factor()));
				} else if (mb->is_alt_pressed()) {
					// Scroll 5 times as fast as normal (like in Visual Studio Code).
					_scroll_up(15 * mb->get_factor());
				} else if (v_scroll->is_visible()) {
					// Scroll 3 lines.
					_scroll_up(3 * mb->get_factor());
				}
			}
			if (mb->get_button_index() == MouseButton::WHEEL_DOWN && !mb->is_command_or_control_pressed()) {
				if (mb->is_shift_pressed()) {
					h_scroll->set_value(h_scroll->get_value() + (100 * mb->get_factor()));
				} else if (mb->is_alt_pressed()) {
					// Scroll 5 times as fast as normal (like in Visual Studio Code).
					_scroll_down(15 * mb->get_factor());
				} else if (v_scroll->is_visible()) {
					// Scroll 3 lines.
					_scroll_down(3 * mb->get_factor());
				}
			}
			if (mb->get_button_index() == MouseButton::WHEEL_LEFT) {
				h_scroll->set_value(h_scroll->get_value() - (100 * mb->get_factor()));
			}
			if (mb->get_button_index() == MouseButton::WHEEL_RIGHT) {
				h_scroll->set_value(h_scroll->get_value() + (100 * mb->get_factor()));
			}
			if (mb->get_button_index() == MouseButton::LEFT) {
				_reset_caret_blink_timer();

				Point2i pos = get_line_column_at_pos(mpos);
				int row = pos.y;
				int col = pos.x;

				int left_margin = style_normal->get_margin(SIDE_LEFT);
				for (int i = 0; i < gutters.size(); i++) {
					if (!gutters[i].draw || gutters[i].width <= 0) {
						continue;
					}

					if (mpos.x >= left_margin && mpos.x <= left_margin + gutters[i].width) {
						emit_signal(SNAME("gutter_clicked"), row, i);
						return;
					}

					left_margin += gutters[i].width;
				}

				// minimap
				if (draw_minimap) {
					_update_minimap_click();
					if (dragging_minimap) {
						return;
					}
				}

				int caret = carets.size() - 1;
				int prev_col = get_caret_column(caret);
				int prev_line = get_caret_line(caret);

				const int triple_click_timeout = 600;
				const int triple_click_tolerance = 5;
				bool is_triple_click = (!mb->is_double_click() && (OS::get_singleton()->get_ticks_msec() - last_dblclk) < triple_click_timeout && mb->get_position().distance_to(last_dblclk_pos) < triple_click_tolerance);

				if (!is_mouse_over_selection() && !mb->is_double_click() && !is_triple_click) {
					if (mb->is_alt_pressed()) {
						prev_line = row;
						prev_col = col;

						caret = add_caret(row, col);
						if (caret == -1) {
							return;
						}

						carets.write[caret].selection.selecting_line = row;
						carets.write[caret].selection.selecting_column = col;

						last_dblclk = 0;
					} else if (!mb->is_shift_pressed()) {
						caret = 0;
						remove_secondary_carets();
					}
				}

				set_caret_line(row, false, true, 0, caret);
				set_caret_column(col, false, caret);
				selection_drag_attempt = false;

				if (selecting_enabled && mb->is_shift_pressed() && (get_caret_column(caret) != prev_col || get_caret_line(caret) != prev_line)) {
					if (!has_selection(caret)) {
						carets.write[caret].selection.active = true;
						selecting_mode = SelectionMode::SELECTION_MODE_POINTER;
						carets.write[caret].selection.from_column = prev_col;
						carets.write[caret].selection.from_line = prev_line;
						carets.write[caret].selection.to_column = carets[caret].column;
						carets.write[caret].selection.to_line = carets[caret].line;

						if (carets[caret].selection.from_line > carets[caret].selection.to_line || (carets[caret].selection.from_line == carets[caret].selection.to_line && carets[caret].selection.from_column > carets[caret].selection.to_column)) {
							SWAP(carets.write[caret].selection.from_column, carets.write[caret].selection.to_column);
							SWAP(carets.write[caret].selection.from_line, carets.write[caret].selection.to_line);
							carets.write[caret].selection.shiftclick_left = false;
						} else {
							carets.write[caret].selection.shiftclick_left = true;
						}
						carets.write[caret].selection.selecting_line = prev_line;
						carets.write[caret].selection.selecting_column = prev_col;
						caret_index_edit_dirty = true;
						merge_overlapping_carets();
						queue_redraw();
					} else {
						if (carets[caret].line < carets[caret].selection.selecting_line || (carets[caret].line == carets[caret].selection.selecting_line && carets[caret].column < carets[caret].selection.selecting_column)) {
							if (carets[caret].selection.shiftclick_left) {
								carets.write[caret].selection.shiftclick_left = !carets[caret].selection.shiftclick_left;
							}
							carets.write[caret].selection.from_column = carets[caret].column;
							carets.write[caret].selection.from_line = carets[caret].line;

						} else if (carets[caret].line > carets[caret].selection.selecting_line || (carets[caret].line == carets[caret].selection.selecting_line && carets[caret].column > carets[caret].selection.selecting_column)) {
							if (!carets[caret].selection.shiftclick_left) {
								SWAP(carets.write[caret].selection.from_column, carets.write[caret].selection.to_column);
								SWAP(carets.write[caret].selection.from_line, carets.write[caret].selection.to_line);
								carets.write[caret].selection.shiftclick_left = !carets[caret].selection.shiftclick_left;
							}
							carets.write[caret].selection.to_column = carets[caret].column;
							carets.write[caret].selection.to_line = carets[caret].line;

						} else {
							deselect(caret);
						}
						caret_index_edit_dirty = true;
						merge_overlapping_carets();
						queue_redraw();
					}
				} else if (drag_and_drop_selection_enabled && is_mouse_over_selection()) {
					set_selection_mode(SelectionMode::SELECTION_MODE_NONE, get_selection_line(caret), get_selection_column(caret), caret);
					// We use the main caret for dragging, so reset this one.
					set_caret_line(prev_line, false, true, 0, caret);
					set_caret_column(prev_col, false, caret);
					selection_drag_attempt = true;
				} else if (caret == 0) {
					deselect();
					set_selection_mode(SelectionMode::SELECTION_MODE_POINTER, row, col);
				}

				if (is_triple_click) {
					// Triple-click select line.
					selecting_mode = SelectionMode::SELECTION_MODE_LINE;
					selection_drag_attempt = false;
					_update_selection_mode_line();
					last_dblclk = 0;
				} else if (mb->is_double_click() && text[get_caret_line(caret)].length()) {
					// Double-click select word.
					selecting_mode = SelectionMode::SELECTION_MODE_WORD;
					_update_selection_mode_word();
					last_dblclk = OS::get_singleton()->get_ticks_msec();
					last_dblclk_pos = mb->get_position();
				}
				queue_redraw();
			}

			if (is_middle_mouse_paste_enabled() && mb->get_button_index() == MouseButton::MIDDLE && DisplayServer::get_singleton()->has_feature(DisplayServer::FEATURE_CLIPBOARD_PRIMARY)) {
				paste_primary_clipboard();
			}

			if (mb->get_button_index() == MouseButton::RIGHT && (context_menu_enabled || is_move_caret_on_right_click_enabled())) {
				_reset_caret_blink_timer();

				Point2i pos = get_line_column_at_pos(mpos);
				int row = pos.y;
				int col = pos.x;
				int caret = carets.size() - 1;

				if (is_move_caret_on_right_click_enabled()) {
					if (has_selection(caret)) {
						int from_line = get_selection_from_line(caret);
						int to_line = get_selection_to_line(caret);
						int from_column = get_selection_from_column(caret);
						int to_column = get_selection_to_column(caret);

						if (row < from_line || row > to_line || (row == from_line && col < from_column) || (row == to_line && col > to_column)) {
							// Right click is outside the selected text.
							deselect(caret);
						}
					}
					if (!has_selection(caret)) {
						set_caret_line(row, true, false, 0, caret);
						set_caret_column(col, true, caret);
					}
					merge_overlapping_carets();
				}

				if (context_menu_enabled) {
					_update_context_menu();
					menu->set_position(get_screen_position() + mpos);
					menu->reset_size();
					menu->popup();
					grab_focus();
				}
			}
		} else {
			if (mb->get_button_index() == MouseButton::LEFT) {
				if (selection_drag_attempt && is_mouse_over_selection()) {
					remove_secondary_carets();

					Point2i pos = get_line_column_at_pos(get_local_mouse_pos());
					set_caret_line(pos.y, false, true, 0, 0);
					set_caret_column(pos.x, true, 0);

					deselect();
				}
				dragging_minimap = false;
				dragging_selection = false;
				can_drag_minimap = false;
				click_select_held->stop();
				if (!drag_action) {
					selection_drag_attempt = false;
				}
				if (DisplayServer::get_singleton()->has_feature(DisplayServer::FEATURE_CLIPBOARD_PRIMARY)) {
					DisplayServer::get_singleton()->clipboard_set_primary(get_selected_text());
				}
			}

			// Notify to show soft keyboard.
			notification(NOTIFICATION_FOCUS_ENTER);
		}
	}

	const Ref<InputEventPanGesture> pan_gesture = p_gui_input;
	if (pan_gesture.is_valid()) {
		const real_t delta = pan_gesture->get_delta().y;
		if (delta < 0) {
			_scroll_up(-delta);
		} else {
			_scroll_down(delta);
		}
		h_scroll->set_value(h_scroll->get_value() + pan_gesture->get_delta().x * 100);
		if (v_scroll->get_value() != prev_v_scroll || h_scroll->get_value() != prev_h_scroll) {
			accept_event(); // Accept event if scroll changed.
		}

		return;
	}

	Ref<InputEventMouseMotion> mm = p_gui_input;

	if (mm.is_valid()) {
		Vector2i mpos = mm->get_position();
		if (is_layout_rtl()) {
			mpos.x = get_size().x - mpos.x;
		}

		if (mm->get_button_mask().has_flag(MouseButtonMask::LEFT) && get_viewport()->gui_get_drag_data() == Variant()) { // Ignore if dragging.
			_reset_caret_blink_timer();

			if (draw_minimap && !dragging_selection) {
				_update_minimap_drag();
			}

			if (!dragging_minimap) {
				switch (selecting_mode) {
					case SelectionMode::SELECTION_MODE_POINTER: {
						_update_selection_mode_pointer();
					} break;
					case SelectionMode::SELECTION_MODE_WORD: {
						_update_selection_mode_word();
					} break;
					case SelectionMode::SELECTION_MODE_LINE: {
						_update_selection_mode_line();
					} break;
					default: {
						break;
					}
				}
			}
		}

		// Check if user is hovering a different gutter, and update if yes.
		Vector2i current_hovered_gutter = Vector2i(-1, -1);

		int left_margin = style_normal->get_margin(SIDE_LEFT);
		if (mpos.x <= left_margin + gutters_width + gutter_padding) {
			int hovered_row = get_line_column_at_pos(mpos).y;
			for (int i = 0; i < gutters.size(); i++) {
				if (!gutters[i].draw || gutters[i].width <= 0) {
					continue;
				}

				if (mpos.x >= left_margin && mpos.x < left_margin + gutters[i].width) {
					// We are in this gutter i's horizontal area.
					current_hovered_gutter = Vector2i(i, hovered_row);
					break;
				}

				left_margin += gutters[i].width;
			}
		}

		if (current_hovered_gutter != hovered_gutter) {
			hovered_gutter = current_hovered_gutter;
			queue_redraw();
		}

		if (drag_action && can_drop_data(mpos, get_viewport()->gui_get_drag_data())) {
			drag_caret_force_displayed = true;
			Point2i pos = get_line_column_at_pos(get_local_mouse_pos());
			set_caret_line(pos.y, false, true, 0, 0);
			set_caret_column(pos.x, true, 0);
			dragging_selection = true;
		}
	}

	if (draw_minimap && !dragging_selection) {
		_update_minimap_hover();
	}

	if (v_scroll->get_value() != prev_v_scroll || h_scroll->get_value() != prev_h_scroll) {
		accept_event(); // Accept event if scroll changed.
	}

	Ref<InputEventKey> k = p_gui_input;

	if (k.is_valid()) {
		if (alt_input(p_gui_input)) {
			accept_event();
			return;
		}
		if (!k->is_pressed()) {
			return;
		}

		// If a modifier has been pressed, and nothing else, return.
		if (k->get_keycode() == Key::CTRL || k->get_keycode() == Key::ALT || k->get_keycode() == Key::SHIFT || k->get_keycode() == Key::META) {
			return;
		}

		_reset_caret_blink_timer();

		// Allow unicode handling if:
		// * No Modifiers are pressed (except shift)
		bool allow_unicode_handling = !(k->is_command_or_control_pressed() || k->is_ctrl_pressed() || k->is_alt_pressed() || k->is_meta_pressed());

		// Check and handle all built in shortcuts.
		
		if (vi_mode) { // Which means not on insert mode.
			handle_vi(k->get_unicode());
			return;
		}

		// NEWLINES.
		if (k->is_action("ui_text_newline_above", true)) {
			_new_line(false, true);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_newline_blank", true)) {
			_new_line(false);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_newline", true)) {
			_new_line();
			accept_event();
			return;
		}

		// BACKSPACE AND DELETE.
		if (k->is_action("ui_text_backspace_all_to_left", true)) {
			_do_backspace(false, true);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_backspace_word", true)) {
			_do_backspace(true);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_backspace", true)) {
			_do_backspace();
			accept_event();
			return;
		}
		if (k->is_action("ui_text_delete_all_to_right", true)) {
			_delete(false, true);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_delete_word", true)) {
			_delete(true);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_delete", true)) {
			_delete();
			accept_event();
			return;
		}

		// SCROLLING.
		if (k->is_action("ui_text_scroll_up", true)) {
			_scroll_lines_up();
			accept_event();
			return;
		}
		if (k->is_action("ui_text_scroll_down", true)) {
			_scroll_lines_down();
			accept_event();
			return;
		}

		if (is_shortcut_keys_enabled()) {
			// SELECT ALL, SELECT WORD UNDER CARET, ADD SELECTION FOR NEXT OCCURRENCE,
			// CLEAR CARETS AND SELECTIONS, CUT, COPY, PASTE.
			if (k->is_action("ui_text_select_all", true)) {
				select_all();
				accept_event();
				return;
			}
			if (k->is_action("ui_text_select_word_under_caret", true)) {
				select_word_under_caret();
				accept_event();
				return;
			}
			if (k->is_action("ui_text_add_selection_for_next_occurrence", true)) {
				add_selection_for_next_occurrence();
				accept_event();
				return;
			}
			if (k->is_action("ui_text_clear_carets_and_selection", true)) {
				if (vi_enabled) {
					set_vi_mode(ViMode::MODE_NORMAL);
					accept_event();
				}
				// Since the default shortcut is ESC, accepts the event only if it's actually performed.
				if (_clear_carets_and_selection()) {
					accept_event();
					return;
				}
			}
			if (k->is_action("ui_cut", true)) {
				cut();
				accept_event();
				return;
			}
			if (k->is_action("ui_copy", true)) {
				copy();
				accept_event();
				return;
			}
			if (k->is_action("ui_paste", true)) {
				paste();
				accept_event();
				return;
			}

			// UNDO/REDO.
			if (k->is_action("ui_undo", true)) {
				undo();
				accept_event();
				return;
			}
			if (k->is_action("ui_redo", true)) {
				redo();
				accept_event();
				return;
			}

			if (k->is_action("ui_text_caret_add_below", true)) {
				add_caret_at_carets(true);
				accept_event();
				return;
			}
			if (k->is_action("ui_text_caret_add_above", true)) {
				add_caret_at_carets(false);
				accept_event();
				return;
			}
		}

		// MISC.
		if (k->is_action("ui_menu", true)) {
			if (context_menu_enabled) {
				_update_context_menu();
				adjust_viewport_to_caret();
				menu->set_position(get_screen_position() + get_caret_draw_pos());
				menu->reset_size();
				menu->popup();
				menu->grab_focus();
			}
			accept_event();
			return;
		}
		if (k->is_action("ui_text_toggle_insert_mode", true)) {
			set_overtype_mode_enabled(!overtype_mode);
			accept_event();
			return;
		}
		if (k->is_action("ui_swap_input_direction", true)) {
			_swap_current_input_direction();
			accept_event();
			return;
		}

		// CARET MOVEMENT

		k = k->duplicate();
		bool shift_pressed = k->is_shift_pressed();
		// Remove shift or else actions will not match. Use above variable for selection.
		k->set_shift_pressed(false);

		// CARET MOVEMENT - LEFT, RIGHT.
		if (k->is_action("ui_text_caret_word_left", true) {
			_move_caret_left(shift_pressed, true);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_caret_left", true) {
			_move_caret_left(shift_pressed, false);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_caret_word_right", true) {
			_move_caret_right(shift_pressed, true);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_caret_right", true) {
			_move_caret_right(shift_pressed, false);
			accept_event();
			return;
		}

		// CARET MOVEMENT - UP, DOWN.
		if (k->is_action("ui_text_caret_up", true) {
			_move_caret_up(shift_pressed);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_caret_down", true) {
			_move_caret_down(shift_pressed);
			accept_event();
			return;
		}

		// CARET MOVEMENT - DOCUMENT START/END.
		if (k->is_action("ui_text_caret_document_start", true) {
			_move_caret_document_start(shift_pressed);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_caret_document_end", true) {
			_move_caret_document_end(shift_pressed);
			accept_event();
			return;
		}

		// CARET MOVEMENT - LINE START/END.
		if (k->is_action("ui_text_caret_line_start", true) {
			_move_caret_to_line_start(shift_pressed);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_caret_line_end", true) {
			_move_caret_to_line_end(shift_pressed);
			accept_event();
			return;
		}

		// CARET MOVEMENT - PAGE UP/DOWN.
		if (k->is_action("ui_text_caret_page_up", true) {
			_move_caret_page_up(shift_pressed);
			accept_event();
			return;
		}
		if (k->is_action("ui_text_caret_page_down", true) {
			_move_caret_page_down(shift_pressed);
			accept_event();
			return;
		}

		// Handle tab as it has no set unicode value.
		if (k->is_action("ui_text_indent", true)) {
			if (editable) {
				insert_text_at_caret("\t");
			}
			accept_event();
			return;
		}

		// Handle Unicode (if no modifiers active).
		if (allow_unicode_handling && editable && k->get_unicode() >= 32) {
			handle_unicode_input(k->get_unicode());
			accept_event();
			return;
		}
	}
}

/* Input actions. */
void TextEdit::_swap_current_input_direction() {
	if (input_direction == TEXT_DIRECTION_LTR) {
		input_direction = TEXT_DIRECTION_RTL;
	} else {
		input_direction = TEXT_DIRECTION_LTR;
	}
	for (int i = 0; i < carets.size(); i++) {
		set_caret_column(get_caret_column(i), i == 0, i);
	}
	queue_redraw();
}

void TextEdit::_new_line(bool p_split_current_line, bool p_above) {
	if (!editable) {
		return;
	}

	begin_complex_operation();
	Vector<int> caret_edit_order = get_caret_index_edit_order();
	for (const int &i : caret_edit_order) {
		bool first_line = false;
		if (!p_split_current_line) {
			deselect(i);
			if (p_above) {
				if (get_caret_line(i) > 0) {
					set_caret_line(get_caret_line(i) - 1, false, true, 0, i);
					set_caret_column(text[get_caret_line(i)].length(), i == 0, i);
				} else {
					set_caret_column(0, i == 0, i);
					first_line = true;
				}
			} else {
				set_caret_column(text[get_caret_line(i)].length(), i == 0, i);
			}
		}

		insert_text_at_caret("\n", i);

		if (first_line) {
			set_caret_line(0, i == 0, true, 0, i);
		}
	}
	end_complex_operation();
}

void TextEdit::_move_caret_left(bool p_select, bool p_move_by_word) {
	for (int i = 0; i < carets.size(); i++) {
		// Handle selection.
		if (p_select) {
			_pre_shift_selection(i);
		} else if (has_selection(i) && !p_move_by_word) {
			// If a selection is active, move caret to start of selection.
			set_caret_line(get_selection_from_line(i), false, true, 0, i);
			set_caret_column(get_selection_from_column(i), i == 0, i);
			deselect(i);
			continue;
		} else {
			deselect(i);
		}

		if (p_move_by_word) {
			int cc = get_caret_column(i);
			// If the caret is at the start of the line, and not on the first line, move it up to the end of the previous line.
			if (cc == 0 && get_caret_line(i) > 0) {
				set_caret_line(get_caret_line(i) - 1, false, true, 0, i);
				set_caret_column(text[get_caret_line(i)].length(), i == 0, i);
			} else {
				PackedInt32Array words = TS->shaped_text_get_word_breaks(text.get_line_data(get_caret_line(i))->get_rid());
				if (words.is_empty() || cc <= words[0]) {
					// This solves the scenario where there are no words but glyfs that can be ignored.
					cc = 0;
				} else {
					for (int j = words.size() - 2; j >= 0; j = j - 2) {
						if (words[j] < cc) {
							cc = words[j];
							break;
						}
					}
				}
				set_caret_column(cc, i == 0, i);
			}
		} else {
			// If the caret is at the start of the line, and not on the first line, move it up to the end of the previous line.
			// not on vi mode <jeff venancius>
			if (get_caret_column(i) == 0) {
				if (get_caret_line(i) > 0) {
					set_caret_line(get_caret_line(i) - get_next_visible_line_offset_from(CLAMP(get_caret_line(i) - 1, 0, text.size() - 1), -1), false, true, 0, i);
					set_caret_column(text[get_caret_line(i)].length(), i == 0, i);
				}
			} else {
				if (caret_mid_grapheme_enabled) {
					set_caret_column(get_caret_column(i) - 1, i == 0, i);
				} else {
					set_caret_column(TS->shaped_text_prev_grapheme_pos(text.get_line_data(get_caret_line(i))->get_rid(), get_caret_column(i)), i == 0, i);
				}
			}
		}

		if (p_select) {
			_post_shift_selection(i);
		}
	}
	merge_overlapping_carets();
}

void TextEdit::_move_caret_right(bool p_select, bool p_move_by_word) {
	for (int i = 0; i < carets.size(); i++) {
		// Handle selection
		if (p_select) {
			_pre_shift_selection(i);
		} else if (has_selection(i) && !p_move_by_word) {
			// If a selection is active, move caret to end of selection
			set_caret_line(get_selection_to_line(i), false, true, 0, i);
			set_caret_column(get_selection_to_column(i), i == 0, i);
			deselect(i);
			continue;
		} else {
			deselect(i);
		}

		if (p_move_by_word) {
			int cc = get_caret_column(i);
			// If the caret is at the end of the line, and not on the last line, move it down to the beginning of the next line.
			if (cc == text[get_caret_line(i)].length() && get_caret_line(i) < text.size() - 1) {
				set_caret_line(get_caret_line(i) + 1, false, true, 0, i);
				set_caret_column(0, i == 0, i);
			} else {
				PackedInt32Array words = TS->shaped_text_get_word_breaks(text.get_line_data(get_caret_line(i))->get_rid());
				if (words.is_empty() || cc >= words[words.size() - 1]) {
					// This solves the scenario where there are no words but glyfs that can be ignored.
					cc = text[get_caret_line(i)].length();
				} else {
					for (int j = 1; j < words.size(); j = j + 2) {
						if (words[j] > cc) {
							cc = words[j];
							break;
						}
					}
				}
				set_caret_column(cc, i == 0, i);
			}
		} else {
			// If we are at the end of the line, move the caret to the next line down.
			// not on vi mode
			if (get_caret_column(i) == text[get_caret_line(i)].length()) {
				if (get_caret_line(i) < text.size() - 1) {
					set_caret_line(get_caret_line(i) + get_next_visible_line_offset_from(CLAMP(get_caret_line(i) + 1, 0, text.size() - 1), 1), false, false, 0, i);
					set_caret_column(0, i == 0, i);
				}
			} else {
				if (caret_mid_grapheme_enabled) {
					set_caret_column(get_caret_column(i) + 1, i == 0, i);
				} else {
					set_caret_column(TS->shaped_text_next_grapheme_pos(text.get_line_data(get_caret_line(i))->get_rid(), get_caret_column(i)), i == 0, i);
				}
			}
		}

		if (p_select) {
			_post_shift_selection(i);
		}
	}
	merge_overlapping_carets();
}

void TextEdit::_move_caret_up(bool p_select) {
	for (int i = 0; i < carets.size(); i++) {
		if (p_select) {
			_pre_shift_selection(i);
		} else {
			deselect(i);
		}

		int cur_wrap_index = get_caret_wrap_index(i);
		if (cur_wrap_index > 0) {
			set_caret_line(get_caret_line(i), true, false, cur_wrap_index - 1, i);
		} else if (get_caret_line(i) == 0) {
			set_caret_column(0, i == 0, i);
		} else {
			int new_line = get_caret_line(i) - get_next_visible_line_offset_from(get_caret_line(i) - 1, -1);
			if (is_line_wrapped(new_line)) {
				set_caret_line(new_line, i == 0, false, get_line_wrap_count(new_line), i);
			} else {
				set_caret_line(new_line, i == 0, false, 0, i);
			}
		}

		if (p_select) {
			_post_shift_selection(i);
		}
	}
	merge_overlapping_carets();
}
void TextEdit::_handle_unicode_input_internal(const uint32_t p_unicode, int p_caret) {
	ERR_FAIL_COND(p_caret > carets.size());
	if (!editable && !vi_mode) {
		return;
	}

	start_action(EditAction::ACTION_TYPING);
	Vector<int> caret_edit_order = get_caret_index_edit_order();
	for (const int &i : caret_edit_order) {
		if (p_caret != -1 && p_caret != i) {
			continue;
		}

		/* Remove the old character if in insert mode and no selection. */
		if (overtype_mode && !has_selection(i)) {
			/* Make sure we don't try and remove empty space. */
			int cl = get_caret_line(i);
			int cc = get_caret_column(i);
			if (cc < get_line(cl).length()) {
				_remove_text(cl, cc, cl, cc + 1);
			}
		}

		const char32_t chr[2] = { (char32_t)p_unicode, 0 };
		insert_text_at_caret(chr, i);
	}
	end_action();
}

void TextEdit::_backspace_internal(int p_caret) {
	ERR_FAIL_COND(p_caret > carets.size());
	if (!editable) {
		return;
	}

	if (has_selection(p_caret)) {
		delete_selection(p_caret);
		return;
	}

	begin_complex_operation();
	Vector<int> caret_edit_order = get_caret_index_edit_order();
	for (const int &i : caret_edit_order) {
		if (p_caret != -1 && p_caret != i) {
			continue;
		}

		int cc = get_caret_column(i);
		int cl = get_caret_line(i);

		if (cc == 0 && cl == 0) {
			continue;
		}

		int prev_line = cc ? cl : cl - 1;
		int prev_column = cc ? (cc - 1) : (text[cl - 1].length());

		merge_gutters(prev_line, cl);

		if (_is_line_hidden(cl)) {
			_set_line_as_hidden(prev_line, true);
		}
		_remove_text(prev_line, prev_column, cl, cc);

		set_caret_line(prev_line, false, true, 0, i);
		set_caret_column(prev_column, i == 0, i);

		adjust_carets_after_edit(i, prev_line, prev_column, cl, cc);
	}
	merge_overlapping_carets();
	end_complex_operation();
}
