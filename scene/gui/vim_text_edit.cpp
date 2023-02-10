
void VimCodeEdit::set_vi_mode(const int p_mode) {
	if (vi_mode == p_mode) {
		return;
	}
	vi_mode = p_mode;
	switch (vi_mode) {
		case ViMode::MODE_INSERT:
			break;
		case ViMode::MODE_NORMAL:
			break;
		case ViMode::MODE_VISUAL:
			break;
		case ViMode::MODE_COMAND:
			break;
	}
	queue_redraw();
}

int VimCodeEdit::get_vi_mode() const {
	return vi_mode;
}

void handle_vim_input(const Ref<InputEvent> &p_key_input) {
	/* TODO - Create the stack - it could be a String but it must be dynamic somehow since we don't know how musch text the user will put.
	 * add key to the stack.
	 * compare the stack to see if a function can be done with it.
	 * if so, do it, save to the command line window and clean the stack.
	 */


}

void VimCodeEdit::gui_input(const Ref<InputEvent> &p_gui_input) {
	Ref<InputEventKey> k = p_gui_input;
	if (TextEdit::alt_input(p_gui_input)) {
		accept_event();
		return;
	}

	bool update_code_completion = false;
	if (!k.is_valid()) {
		TextEdit::gui_input(p_gui_input);
		return;
	}

	/* Ctrl + Hover symbols */
#ifdef MACOS_ENABLED
	if (k->get_keycode() == Key::META) {
#else
	if (k->get_keycode() == Key::CTRL) {
#endif
	/* If a modifier has been pressed, and nothing else, return. */
	if (!k->is_pressed() || k->get_keycode() == Key::CTRL || k->get_keycode() == Key::ALT || k->get_keycode() == Key::SHIFT || k->get_keycode() == Key::META) {
		return;
	}
	
	/* Allow unicode handling if:              */
	/* No Modifiers are pressed (except shift) */
	bool allow_unicode_handling = !(k->is_command_or_control_pressed() || k->is_ctrl_pressed() || k->is_alt_pressed() || k->is_meta_pressed());

	if (allow_unicode_handling && get_mode()) {
		handle_vim_input(k);
			return;
	}


	/* AUTO-COMPLETE */
	if (code_completion_enabled && k->is_action("ui_text_completion_query", true)) {
		request_code_completion(true);
		accept_event();
		return;
	}

	if (code_completion_active) {
		if (k->is_action("ui_up", true)) {
			if (code_completion_current_selected > 0) {
				code_completion_current_selected--;
			} else {
				code_completion_current_selected = code_completion_options.size() - 1;
			}
			code_completion_force_item_center = -1;
			queue_redraw();
			accept_event();
			return;
		}
		if (k->is_action("ui_down", true)) {
			if (code_completion_current_selected < code_completion_options.size() - 1) {
				code_completion_current_selected++;
			} else {
				code_completion_current_selected = 0;
			}
			code_completion_force_item_center = -1;
			queue_redraw();
			accept_event();
			return;
		}
		if (k->is_action("ui_page_up", true)) {
			code_completion_current_selected = MAX(0, code_completion_current_selected - code_completion_max_lines);
			code_completion_force_item_center = -1;
			queue_redraw();
			accept_event();
			return;
		}
		if (k->is_action("ui_page_down", true)) {
			code_completion_current_selected = MIN(code_completion_options.size() - 1, code_completion_current_selected + code_completion_max_lines);
			code_completion_force_item_center = -1;
			queue_redraw();
			accept_event();
			return;
		}
		if (k->is_action("ui_home", true)) {
			code_completion_current_selected = 0;
			code_completion_force_item_center = -1;
			queue_redraw();
			accept_event();
			return;
		}
		if (k->is_action("ui_end", true)) {
			code_completion_current_selected = code_completion_options.size() - 1;
			code_completion_force_item_center = -1;
			queue_redraw();
			accept_event();
			return;
		}
		if (k->is_action("ui_text_completion_replace", true) || k->is_action("ui_text_completion_accept", true)) {
			confirm_code_completion(k->is_action("ui_text_completion_replace", true));
			accept_event();
			return;
		}
		if (k->is_action("ui_cancel", true)) {
			cancel_code_completion();
			accept_event();
			return;
		}
		if (k->is_action("ui_text_backspace", true)) {
			backspace();
			_filter_code_completion_candidates_impl();
			accept_event();
			return;
		}

		if (k->is_action("ui_left", true) || k->is_action("ui_right", true)) {
			update_code_completion = true;
		} else {
			update_code_completion = (allow_unicode_handling && k->get_unicode() >= 32);
		}

		if (!update_code_completion) {
			cancel_code_completion();
		}
	}

	/* MISC */
	if (!code_hint.is_empty() && k->is_action("ui_cancel", true)) {
		set_code_hint("");
		accept_event();
		return;
	}
	if (allow_unicode_handling && k->get_unicode() == ')') {
		set_code_hint("");
	}

	/* Indentation */
	if (k->is_action("ui_text_indent", true)) {
		do_indent();
		accept_event();
		return;
	}

	if (k->is_action("ui_text_dedent", true)) {
		unindent_lines();
		accept_event();
		return;
	}

	// Override new line actions, for auto indent
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

	/* Remove shift otherwise actions will not match. */
	k = k->duplicate();
	k->set_shift_pressed(false);

	if (k->is_action("ui_text_caret_up", true) ||
			k->is_action("ui_text_caret_down", true) ||
			k->is_action("ui_text_caret_line_start", true) ||
			k->is_action("ui_text_caret_line_end", true) ||
			k->is_action("ui_text_caret_page_up", true) ||
			k->is_action("ui_text_caret_page_down", true)) {
		set_code_hint("");
	}

	TextEdit::gui_input(p_gui_input);

	if (update_code_completion) {
		_filter_code_completion_candidates_impl();
	}
}

/* General overrides */
Control::CursorShape CodeEdit::get_cursor_shape(const Point2 &p_pos) const {
	if (!symbol_lookup_word.is_empty()) {
		return CURSOR_POINTING_HAND;
	}

	if ((code_completion_active && code_completion_rect.has_point(p_pos)) || (!is_editable() && (!is_selecting_enabled() || get_line_count() == 0))) {
		return CURSOR_ARROW;
	}

	if (code_completion_active && code_completion_scroll_rect.has_point(p_pos)) {
		return CURSOR_ARROW;
	}

	Point2i pos = get_line_column_at_pos(p_pos, false);
	int line = pos.y;
	int col = pos.x;

	if (line != -1 && is_line_folded(line)) {
		int wrap_index = get_line_wrap_index_at_column(line, col);
		if (wrap_index == get_line_wrap_count(line)) {
			int eol_icon_width = folded_eol_icon->get_width();
			int left_margin = get_total_gutter_width() + eol_icon_width + get_line_width(line, wrap_index) - get_h_scroll();
			if (p_pos.x > left_margin && p_pos.x <= left_margin + eol_icon_width + 3) {
				return CURSOR_POINTING_HAND;
			}
		}
	}

	return TextEdit::get_cursor_shape(p_pos);
}

/* Text manipulation */

// Overridable actions
void CodeEdit::_handle_unicode_input_internal(const uint32_t p_unicode, int p_caret) {
	start_action(EditAction::ACTION_TYPING);
	Vector<int> caret_edit_order = get_caret_index_edit_order();
	for (const int &i : caret_edit_order) {
		if (p_caret != -1 && p_caret != i) {
			continue;
		}

		bool had_selection = has_selection(i);
		String selection_text = (had_selection ? get_selected_text(i) : "");

		if (had_selection) {
			delete_selection(i);
		}

		// Remove the old character if in overtype mode and no selection.
		if (is_overtype_mode_enabled() && !had_selection) {
			// Make sure we don't try and remove empty space.
			if (get_caret_column(i) < get_line(get_caret_line(i)).length()) {
				remove_text(get_caret_line(i), get_caret_column(i), get_caret_line(i), get_caret_column(i) + 1);
			}
		}

		const char32_t chr[2] = { (char32_t)p_unicode, 0 };

		if (auto_brace_completion_enabled) {
			int cl = get_caret_line(i);
			int cc = get_caret_column(i);

			if (had_selection) {
				insert_text_at_caret(chr, i);

				String close_key = get_auto_brace_completion_close_key(chr);
				if (!close_key.is_empty()) {
					insert_text_at_caret(selection_text + close_key, i);
					set_caret_column(get_caret_column(i) - 1, i == 0, i);
				}
			} else {
				int caret_move_offset = 1;

				int post_brace_pair = cc < get_line(cl).length() ? _get_auto_brace_pair_close_at_pos(cl, cc) : -1;

				if (has_string_delimiter(chr) && cc > 0 && !is_symbol(get_line(cl)[cc - 1]) && post_brace_pair == -1) {
					insert_text_at_caret(chr, i);
				} else if (cc < get_line(cl).length() && !is_symbol(get_line(cl)[cc])) {
					insert_text_at_caret(chr, i);
				} else if (post_brace_pair != -1 && auto_brace_completion_pairs[post_brace_pair].close_key[0] == chr[0]) {
					caret_move_offset = auto_brace_completion_pairs[post_brace_pair].close_key.length();
				} else if (is_in_comment(cl, cc) != -1 || (is_in_string(cl, cc) != -1 && has_string_delimiter(chr))) {
					insert_text_at_caret(chr, i);
				} else {
					insert_text_at_caret(chr, i);

					int pre_brace_pair = _get_auto_brace_pair_open_at_pos(cl, cc + 1);
					if (pre_brace_pair != -1) {
						insert_text_at_caret(auto_brace_completion_pairs[pre_brace_pair].close_key, i);
					}
				}
				set_caret_column(cc + caret_move_offset, i == 0, i);
			}
		} else {
			insert_text_at_caret(chr, i);
		}
	}
	end_action();
}
