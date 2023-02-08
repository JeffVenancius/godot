
static const _BuiltinActionDisplayName _normal_mode_display_names[] = {
	/* clang-format off */
    { "nvi_accept",                                     TTRC("Accept") },
    { "nvi_select",                                     TTRC("Select") },
    { "nvi_cancel",                                     TTRC("Cancel") },
    { "nvi_focus_next",                                 TTRC("Focus Next") },
    { "nvi_focus_prev",                                 TTRC("Focus Prev") },
    { "nvi_left",                                       TTRC("Left") },
    { "nvi_right",                                      TTRC("Right") },
    { "nvi_up",                                         TTRC("Up") },
    { "nvi_down",                                       TTRC("Down") },
    { "nvi_page_up",                                    TTRC("Page Up") },
    { "nvi_page_down",                                  TTRC("Page Down") },
    { "nvi_home",                                       TTRC("Home") },
    { "nvi_end",                                        TTRC("End") },
    { "nvi_cut",                                        TTRC("Cut") },
    { "nvi_copy",                                       TTRC("Copy") },
    { "nvi_paste",                                      TTRC("Paste") },
    { "nvi_undo",                                       TTRC("Undo") },
    { "nvi_redo",                                       TTRC("Redo") },
    { "nvi_text_completion_query",                      TTRC("Completion Query") },
    { "nvi_text_newline",                               TTRC("New Line") },
    { "nvi_text_newline_blank",                         TTRC("New Blank Line") },
    { "nvi_text_newline_above",                         TTRC("New Line Above") },
    { "nvi_text_indent",                                TTRC("Indent") },
    { "nvi_text_dedent",                                TTRC("Dedent") },
    { "nvi_text_backspace",                             TTRC("Backspace") },
    { "nvi_text_backspace_word",                        TTRC("Backspace Word") },
    { "nvi_text_backspace_word.macos",                  TTRC("Backspace Word") },
    { "nvi_text_backspace_all_to_left",                 TTRC("Backspace all to Left") },
    { "nvi_text_backspace_all_to_left.macos",           TTRC("Backspace all to Left") },
    { "nvi_text_delete",                                TTRC("Delete") },
    { "nvi_text_delete_word",                           TTRC("Delete Word") },
    { "nvi_text_delete_word.macos",                     TTRC("Delete Word") },
    { "nvi_text_delete_all_to_right",                   TTRC("Delete all to Right") },
    { "nvi_text_delete_all_to_right.macos",             TTRC("Delete all to Right") },
    { "nvi_text_caret_left",                            TTRC("Caret Left") },
    { "nvi_text_caret_word_left",                       TTRC("Caret Word Left") },
    { "nvi_text_caret_word_left.macos",                 TTRC("Caret Word Left") },
    { "nvi_text_caret_right",                           TTRC("Caret Right") },
    { "nvi_text_caret_word_right",                      TTRC("Caret Word Right") },
    { "nvi_text_caret_word_right.macos",                TTRC("Caret Word Right") },
    { "nvi_text_caret_up",                              TTRC("Caret Up") },
    { "nvi_text_caret_down",                            TTRC("Caret Down") },
    { "nvi_text_caret_line_start",                      TTRC("Caret Line Start") },
    { "nvi_text_caret_line_start.macos",                TTRC("Caret Line Start") },
    { "nvi_text_caret_line_end",                        TTRC("Caret Line End") },
    { "nvi_text_caret_line_end.macos",                  TTRC("Caret Line End") },
    { "nvi_text_caret_page_up",                         TTRC("Caret Page Up") },
    { "nvi_text_caret_page_down",                       TTRC("Caret Page Down") },
    { "nvi_text_caret_document_start",                  TTRC("Caret Document Start") },
    { "nvi_text_caret_document_start.macos",            TTRC("Caret Document Start") },
    { "nvi_text_caret_document_end",                    TTRC("Caret Document End") },
    { "nvi_text_caret_document_end.macos",              TTRC("Caret Document End") },
    { "nvi_text_caret_add_below",                       TTRC("Caret Add Below") },
    { "nvi_text_caret_add_below.macos",                 TTRC("Caret Add Below") },
    { "nvi_text_caret_add_above",                       TTRC("Caret Add Above") },
    { "nvi_text_caret_add_above.macos",                 TTRC("Caret Add Above") },
    { "nvi_text_scroll_up",                             TTRC("Scroll Up") },
    { "nvi_text_scroll_up.macos",                       TTRC("Scroll Up") },
    { "nvi_text_scroll_down",                           TTRC("Scroll Down") },
    { "nvi_text_scroll_down.macos",                     TTRC("Scroll Down") },
    { "nvi_text_select_all",                            TTRC("Select All") },
    { "nvi_text_select_word_under_caret",               TTRC("Select Word Under Caret") },
    { "nvi_text_add_selection_for_next_occurrence",     TTRC("Add Selection for Next Occurrence") },
    { "nvi_text_clear_carets_and_selection",            TTRC("Clear Carets and Selection") },
    { "nvi_text_toggle_insert_mode",                    TTRC("Toggle Insert Mode") },
    { "nvi_text_submit",                                TTRC("Text Submitted") },
    { "nvi_graph_duplicate",                            TTRC("Duplicate Nodes") },
    { "nvi_graph_delete",                               TTRC("Delete Nodes") },
    { "nvi_filedialog_up_one_level",                    TTRC("Go Up One Level") },
    { "nvi_filedialog_refresh",                         TTRC("Refresh") },
    { "nvi_filedialog_show_hidden",                     TTRC("Show Hidden") },
    { "nvi_swap_input_direction ",                      TTRC("Swap Input Direction") },
    { "",                                              ""}
	/* clang-format on */
};


const HashMap<String, List<Ref<InputEvent>>> &InputMap::get_builtins() {
	// Return cache if it has already been built.
	if (default_builtin_cache.size()) {
		return default_builtin_cache;
	}

	List<Ref<InputEvent>> inputs;
	inputs.push_back(InputEventKey::create_reference(Key::H));
	default_builtin_cache.insert("nvi_left", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::L));
	default_builtin_cache.insert("nvi_right", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::K));
	default_builtin_cache.insert("nvi_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::J));
	default_builtin_cache.insert("nvi_down", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::B));
	default_builtin_cache.insert("nvi_page_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::F));
	default_builtin_cache.insert("nvi_page_down", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::G));
	default_builtin_cache.insert("nvi_end", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::D));
	default_builtin_cache.insert("nvi_cut", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::Y));
	default_builtin_cache.insert("nvi_copy", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::P));
	default_builtin_cache.insert("nvi_paste", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::U));
	default_builtin_cache.insert("nvi_undo", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::R));
	default_builtin_cache.insert("nvi_redo", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::TAB));
	default_builtin_cache.insert("nvi_text_completion_query", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::O));
	default_builtin_cache.insert("nvi_text_newline", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::O));
	default_builtin_cache.insert("nvi_text_newline_blank", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::O));
	default_builtin_cache.insert("nvi_text_newline_above", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE)); // >
	default_builtin_cache.insert("nvi_text_indent", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE)); // <
	default_builtin_cache.insert("nvi_text_dedent", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_backspace_word", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_backspace_word.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_backspace_all_to_left", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_backspace_all_to_left.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_delete", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_delete_word", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_delete_word.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_delete_all_to_right", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_delete_all_to_right.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_left", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_word_left", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_word_left.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_right", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_word_right", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_word_right.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_down", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_line_start", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_line_start.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_line_end", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_line_end.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_page_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_page_down", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_document_start", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_document_start.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_document_end", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_document_end.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_add_below", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_add_below.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_add_above", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_caret_add_above.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_scroll_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_scroll_up.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_scroll_down", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_scroll_down.macos", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_select_all", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_select_word_under_caret", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_add_selection_for_next_occurrence", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_clear_carets_and_selection", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_toggle_insert_mode", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_text_submit", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_graph_duplicate", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_graph_delete", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_filedialog_up_one_level", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_filedialog_refresh", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_filedialog_show_hidden", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("nvi_swap_input_direction", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(Key::SPACE));
	default_builtin_cache.insert("", inputs);
