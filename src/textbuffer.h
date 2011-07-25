/* Copyright (C) 2010 G.P. Halkes
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3, as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef T3_WIDGET_TEXTBUFFER_H
#define T3_WIDGET_TEXTBUFFER_H

#include <cstdio>
#include <vector>
#include <string>

#include <t3widget/key.h>
#include <t3widget/textline.h>
#include <t3widget/undo.h>
#include <t3widget/main.h>
#include <t3widget/interfaces.h>

namespace t3_widget {

typedef std::vector<text_line_t *> lines_t;

struct find_result_t;
class finder_t;
class wrap_info_t;

class T3_WIDGET_API text_buffer_t : public bad_draw_recheck_t {
	friend class wrap_info_t;
	protected:
		lines_t lines;
		text_coordinate_t selection_start;
		text_coordinate_t selection_end;
		undo_list_t undo_list;
		text_coordinate_t last_undo_position;
		undo_type_t last_undo_type;
		undo_t *last_undo;
		char *name;
		text_line_t name_line;

		selection_mode_t selection_mode;
		text_line_factory_t *line_factory;

		undo_t *get_undo(undo_type_t type);
		undo_t *get_undo(undo_type_t type, text_coordinate_t coord);
		void locate_pos(void);
		void locate_pos(text_coordinate_t *coord) const;
		void delete_block_internal(text_coordinate_t start, text_coordinate_t end, undo_t *undo);
		bool insert_block_internal(text_coordinate_t insert_at, text_line_t *block);
		int apply_undo_redo(undo_type_t type, undo_t *current);
		bool merge_internal(int line);
		bool break_line_internal(void);

		void set_selection_from_find(int line, find_result_t *result);
		bool undo_indent_selection(undo_t *undo, undo_type_t type);

	public:
		text_buffer_t(const char *_name = NULL, text_line_factory_t *_line_factory = NULL);
		virtual ~text_buffer_t(void);
		virtual void bad_draw_recheck(void);

		int size(void) const;
		const std::string *get_line_data(int idx) const;

		bool insert_char(key_t c);
		bool overwrite_char(key_t c);
		bool delete_char(void);
		bool backspace_char(void);
		bool merge(bool backspace);
		bool break_line(void);
		bool insert_block(const std::string *block);

		bool append_text(const char *text);
		bool append_text(const char *text, size_t size);
		bool append_text(const std::string *text);

		int get_line_max(int line) const;
		void adjust_position(int adjust);
		int width_at_cursor(void) const;

		void paint_line(t3_window_t *win, int line, text_line_t::paint_info_t *info) const;
		void goto_next_word(void);
		void goto_previous_word(void);

		int calculate_screen_pos(const text_coordinate_t *where, int tabsize) const;
		int calculate_line_pos(int line, int pos, int tabsize) const;

		const text_line_t *get_name_line(void) const;

		text_coordinate_t get_selection_start(void) const;
		text_coordinate_t get_selection_end(void) const;
		void set_selection_end(void);
		void set_selection_mode(selection_mode_t mode);
		selection_mode_t get_selection_mode(void) const;
		bool selection_empty(void) const;
		void delete_selection(void);
		bool replace_selection(const std::string *block);
		bool indent_selection(int tabsize, bool tab_spaces);
		bool unindent_selection(int tabsize);
		bool unindent_line(int tabsize);

		bool find(finder_t *finder, bool reverse = false);
		void replace(finder_t *finder);

		bool is_modified(void) const;
		std::string *convert_selection(void);
		int apply_undo(void);
		int apply_redo(void);

		const char *get_name(void) const;

		//FIXME: make these members private again
		/* These are public members, because they store data that is not used
		   by the text_buffer_t class (except cursor), but are stored here
		   because the data are associated with a text bufffer and not with
		   a window displaying the text buffer. */
		text_coordinate_t cursor;
		int ins_mode, last_set_pos;

	T3_WIDGET_SIGNAL(rewrap_required, void, rewrap_type_t, int, int);
};

}; // namespace
#endif
