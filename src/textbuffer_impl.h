/* Copyright (C) 2018 G.P. Halkes
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
#ifndef T3_WIDGET_TEXTBUFFER_IMPL_H
#define T3_WIDGET_TEXTBUFFER_IMPL_H

#ifndef _T3_WIDGET_INTERNAL
#error This header file is for internal use _only_!!
#endif

#include <t3widget/textbuffer.h>
#include <t3widget/undo.h>

namespace t3widget {

struct text_buffer_t::implementation_t {
  std::vector<std::unique_ptr<text_line_t>> lines;
  text_coordinate_t selection_start;
  text_coordinate_t selection_end;
  selection_mode_t selection_mode;

  undo_list_t undo_list;
  text_coordinate_t last_undo_position;
  undo_type_t last_undo_type;
  undo_t *last_undo;

  text_line_factory_t *line_factory;
  signal_t<rewrap_type_t, text_pos_t, text_pos_t> rewrap_required;
  text_coordinate_t cursor;

  implementation_t(text_line_factory_t *_line_factory)
      : selection_start(-1, 0),
        selection_end(-1, 0),
        selection_mode(selection_mode_t::NONE),
        last_undo_type(UNDO_NONE),
        last_undo(nullptr),
        line_factory(_line_factory == nullptr ? &default_text_line_factory : _line_factory),
        cursor(0, 0) {
    // Allocate a new, empty line
    lines.push_back(line_factory->new_text_line_t());
  }

  text_pos_t size() const { return lines.size(); }
  text_pos_t get_line_size(text_pos_t line) const { return lines[line]->size(); }
  text_pos_t calculate_line_pos(text_pos_t line, text_pos_t pos, int tabsize) const;
  bool insert_char(key_t c);
  bool overwrite_char(key_t c);
  bool delete_char();
  bool backspace_char();
  bool backspace_word();
  bool merge_internal(text_pos_t line);
  bool insert_block_internal(text_coordinate_t insert_at, std::unique_ptr<text_line_t> block);
  void delete_block_internal(text_coordinate_t start, text_coordinate_t end, undo_t *undo);
  bool break_line_internal(const std::string &indent = nullptr);
  bool append_text(string_view text);
  bool break_line(const std::string &indent);
  bool merge(bool backspace);
  bool insert_block(const std::string &block);
  bool replace_block(text_coordinate_t start, text_coordinate_t end, const std::string &block);
  std::unique_ptr<std::string> convert_block(text_coordinate_t start, text_coordinate_t end);
  void goto_next_word();
  void goto_previous_word();
  void goto_next_word_boundary();
  void goto_previous_word_boundary();
  void adjust_position(int adjust);
  int width_at_cursor() const;
  void set_selection_mode(selection_mode_t mode);
  bool selection_empty() const { return selection_start == selection_end; }
  void set_selection_end(bool update_primary);
  undo_t *get_undo(undo_type_t type);
  undo_t *get_undo(undo_type_t type, text_coordinate_t coord);
  void start_undo_block() { get_undo(UNDO_BLOCK_START); }
  void end_undo_block() { get_undo(UNDO_BLOCK_END); }
  void set_undo_mark();
  void apply_undo_redo(undo_type_t type, undo_t *current);
  void set_selection_from_find(const find_result_t &result);
  bool find(finder_t *finder, find_result_t *result, bool reverse) const;
  bool find_limited(finder_t *finder, text_coordinate_t start, text_coordinate_t end,
                    find_result_t *result) const;
  bool indent_block(text_coordinate_t &start, text_coordinate_t &end, int tabsize, bool tab_spaces);
  bool indent_selection(int tabsize, bool tab_spaces);
  bool undo_indent_selection(undo_t *undo, undo_type_t type);
  bool unindent_selection(int tabsize);
  bool unindent_block(text_coordinate_t &start, text_coordinate_t &end, int tabsize);
  bool unindent_line(int tabsize);
  void goto_pos(text_pos_t line, text_pos_t pos);
};

}  // namespace t3widget

#endif
