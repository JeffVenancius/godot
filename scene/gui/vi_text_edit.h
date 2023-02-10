
	/* Vi mode */
	bool vi_enabled = false; // So esc can leave insert mode.
	ViMode vi_mode = ViMode::MODE_INSERT;

	/* Vi mode */
	void set_vi_enabled(const bool p_enabled);
	bool is_vi_enabled() const;

	void set_vi_mode(const int p_mode);
	int get_vi_mode() const;

	void handle_vi(const uint32_t p_unicode, const Vector<String> &p_text);
