#ifndef VIM_CODE_EDIT_H
#define VIM_CODE_EDIT_H

#include "scene/gui/code_edit.h.h"

class VimCodeEdit : public CodeEdit {
	GDCLASS(VimCodeEdit, CodeEdit)

		enum VimMode {
			MODE_INSERT,
			MODE_NORMAL,
			MODE_VISUAL,
			MODE_COMAND,
		};

	public:
	VimMode mode = VimMode::MODE_INSERT;

	void set_mode(const int p_mode);
	int get_mode() const;

	void handle_vim_input(const Ref<InputEvent> &p_key_input);

#endif // VIM_TEXT_EDIT_H
