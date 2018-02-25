/* Copyright (C) 2011-2013,2018 G.P. Halkes
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
#include "dialogbase.h"
#include "colorscheme.h"
#include "dialogs/dialog.h"
#include "dialogs/mainwindow.h"
#include "internal.h"
#include "main.h"

namespace t3_widget {

dialog_base_list_t dialog_base_t::dialog_base_list;

dummy_widget_t *dialog_base_t::dummy;
signals::connection dialog_base_t::init_connected = connect_on_init(dialog_base_t::init);

void dialog_base_t::init(bool _init) {
  if (_init) {
    if (dummy == nullptr) {
      dummy = new dummy_widget_t();
    }
  } else {
    if (dummy != nullptr) {
      delete dummy;
    }
  }
}

dialog_base_t::dialog_base_t(int height, int width, bool has_shadow) : redraw(true) {
  window.alloc(nullptr, height, width, 0, 0, 0);
  if (has_shadow) {
    shadow_window.alloc(nullptr, height, width, 1, 1, 1);
    shadow_window.set_anchor(&window, 0);
  }
  dialog_base_list.push_back(this);
  window.set_restrict(nullptr);
  current_widget = widgets.begin();
}

/** Create a new ::dialog_base_t.

    This constructor should only be called by ::main_window_base_t (through ::dialog_t).
*/
dialog_base_t::dialog_base_t() : redraw(false) { dialog_base_list.push_back(this); }

dialog_base_t::~dialog_base_t() {
  for (dialog_base_list_t::iterator iter = dialog_base_list.begin(); iter != dialog_base_list.end();
       iter++) {
    if ((*iter) == this) {
      dialog_base_list.erase(iter);
      break;
    }
  }
  for (widget_t *widget : widgets) {
    if (widget != dummy) {
      delete widget;
    }
  }
}

void dialog_base_t::set_position(optint top, optint left) {
  if (!top.is_valid()) {
    top = window.get_y();
  }
  if (!left.is_valid()) {
    left = window.get_x();
  }

  window.move(top, left);
}

bool dialog_base_t::set_size(optint height, optint width) {
  bool result = true;

  redraw = true;
  if (!height.is_valid()) {
    height = window.get_height();
  }
  if (!width.is_valid()) {
    width = window.get_width();
  }

  result &= (window.resize(height, width) == 0);
  if (shadow_window != nullptr) {
    result &= (shadow_window.resize(height, width) == 0);
  }
  return result;
}

void dialog_base_t::update_contents() {
  if (redraw) {
    int i, x;

    redraw = false;
    window.set_default_attrs(attributes.dialog);

    /* Just clear the whole thing and redraw */
    window.set_paint(0, 0);
    window.clrtobot();

    window.box(0, 0, window.get_height(), window.get_width(), 0);

    if (shadow_window != nullptr) {
      shadow_window.set_default_attrs(attributes.shadow);
      x = shadow_window.get_width() - 1;
      for (i = shadow_window.get_height() - 1; i > 0; i--) {
        shadow_window.set_paint(i - 1, x);
        shadow_window.addch(' ', 0);
      }
      shadow_window.set_paint(shadow_window.get_height() - 1, 0);
      shadow_window.addchrep(' ', 0, shadow_window.get_width());
    }
  }

  for (widget_t *widget : widgets) {
    widget->update_contents();
  }
}

void dialog_base_t::set_focus(focus_t focus) {
  if (current_widget != widgets.end()) {
    (*current_widget)->set_focus(focus);
  }
}

void dialog_base_t::show() {
  for (current_widget = widgets.begin();
       current_widget != widgets.end() && !(*current_widget)->accepts_focus(); current_widget++) {
  }

  if (current_widget == widgets.end()) {
    widgets.push_front(dummy);
    current_widget = widgets.begin();
  }

  window.show();
  if (shadow_window != nullptr) {
    shadow_window.show();
  }
}

void dialog_base_t::hide() {
  window.hide();
  if (shadow_window != nullptr) {
    shadow_window.hide();
  }
  if (widgets.front() == dummy) {
    widgets.pop_front();
  }
}

void dialog_base_t::focus_next() {
  (*current_widget)->set_focus(window_component_t::FOCUS_OUT);
  do {
    current_widget++;
    if (current_widget == widgets.end()) {
      current_widget = widgets.begin();
    }
  } while (!(*current_widget)->accepts_focus());

  (*current_widget)->set_focus(window_component_t::FOCUS_IN_FWD);
}

void dialog_base_t::focus_previous() {
  (*current_widget)->set_focus(window_component_t::FOCUS_OUT);

  do {
    if (current_widget == widgets.begin()) {
      current_widget = widgets.end();
    }

    current_widget--;
  } while (!(*current_widget)->accepts_focus());

  (*current_widget)->set_focus(window_component_t::FOCUS_IN_BCK);
}

void dialog_base_t::set_child_focus(window_component_t *target) {
  widget_t *target_widget = dynamic_cast<widget_t *>(target);
  if (target_widget == nullptr || !target_widget->accepts_focus()) {
    return;
  }

  for (widgets_t::iterator iter = widgets.begin(); iter != widgets.end(); iter++) {
    if (*iter == target) {
      if (*current_widget != *iter) {
        (*current_widget)->set_focus(window_component_t::FOCUS_OUT);
        current_widget = iter;
        (*current_widget)->set_focus(window_component_t::FOCUS_SET);
      }
      return;
    } else {
      container_t *container = dynamic_cast<container_t *>(*iter);
      if (container != nullptr && container->is_child(target)) {
        if (*current_widget != *iter) {
          (*current_widget)->set_focus(window_component_t::FOCUS_OUT);
          current_widget = iter;
        }
        container->set_child_focus(target);
        return;
      }
    }
  }
}

bool dialog_base_t::is_child(window_component_t *widget) {
  for (widget_t *iter : widgets) {
    if (iter == widget) {
      return true;
    } else {
      container_t *container = dynamic_cast<container_t *>(iter);
      if (container != nullptr && container->is_child(widget)) {
        return true;
      }
    }
  }
  return false;
}

void dialog_base_t::push_back(widget_t *widget) {
  if (!set_widget_parent(widget)) {
    return;
  }
  if (widgets.size() > 0 && widgets.front() == dummy) {
    widgets.pop_front();
  }
  widgets.push_back(widget);
}

void dialog_base_t::force_redraw() {
  redraw = true;
  for (widget_t *widget : widgets) {
    widget->force_redraw();
  }
}

void dialog_base_t::center_over(window_component_t *center) {
  window.set_anchor(center->get_base_window(),
                    T3_PARENT(T3_ANCHOR_CENTER) | T3_CHILD(T3_ANCHOR_CENTER));
  window.move(0, 0);
}

void dialog_base_t::force_redraw_all() {
  for (dialog_base_t *dialog : dialog_base_list) {
    dialog->force_redraw();
  }
}

}  // namespace
