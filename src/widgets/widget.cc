/* Copyright (C) 2011-2012,2018 G.P. Halkes
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
#include "widgets/widget.h"
#include "colorscheme.h"
#include "log.h"
#include "main.h"

namespace t3_widget {

/* The default_parent must exist before any widgets are created. Thus using the
   #on_init method won't work. Instead we use a cleanup_t3_window.
*/
t3_window::window_t widget_t::default_parent(nullptr, 1, 1, 0, 0, 0, false);

bool widget_t::is_hotkey(key_t key) {
  (void)key;
  return false;
}

bool widget_t::accepts_focus() { return enabled && shown; }

widget_t::widget_t(int height, int width, bool register_as_mouse_target)
    : redraw(true), enabled(true), shown(true) {
  init_window(height, width, register_as_mouse_target);
}

widget_t::widget_t() : redraw(true), enabled(true), shown(true) {}

void widget_t::init_window(int height, int width, bool register_as_mouse_target) {
  window.alloc(&default_parent, height, width, 0, 0, 0);
  window.show();
  if (register_as_mouse_target) {
    register_mouse_target(&window);
  }
}

void widget_t::init_unbacked_window(int height, int width, bool register_as_mouse_target) {
  window.alloc_unbacked(&default_parent, height, width, 0, 0, 0);
  window.show();
  if (register_as_mouse_target) {
    register_mouse_target(&window);
  }
}

void widget_t::set_anchor(window_component_t *anchor, int relation) {
  window.set_anchor(anchor->get_base_window(), relation);
}

void widget_t::set_position(optint top, optint left) {
  if (!top.is_valid()) {
    top = window.get_y();
  }
  if (!left.is_valid()) {
    left = window.get_x();
  }

  window.move(top, left);
}

void widget_t::show() {
  window.show();
  shown = true;
}

void widget_t::hide() {
  window.hide();
  shown = false;
}

void widget_t::force_redraw() { redraw = true; }

void widget_t::set_enabled(bool enable) { enabled = enable; }

bool widget_t::is_enabled() { return enabled; }

bool widget_t::is_shown() { return shown; }

void widget_t::set_focus(focus_t focus) {
  (void)focus;
  return;
}

bool widget_t::process_mouse_event(mouse_event_t event) {
  lprintf("Default mouse handling for %s (%d)\n", typeid(*this).name(), accepts_focus());
  return accepts_focus() && (event.button_state & EMOUSE_CLICK_BUTTONS);
}

}  // namespace
