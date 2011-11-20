/* Copyright (C) 2011 G.P. Halkes
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
#include <algorithm>
#include "widgets/split.h"

using namespace std;

namespace t3_widget {

split_t::split_t(widget_t *widget) : horizontal(true) {
	init_unbacked_window(3, 3);
	set_widget_parent(widget);
	widget->set_anchor(this, 0);
	widget->show();
	widgets.push_back(widget);
	current = widgets.begin();
}

split_t::~split_t(void) {
	for (widgets_t::iterator iter = widgets.begin(); iter != widgets.end(); iter++)
		delete (*iter);
}

bool split_t::process_key(key_t key) {
	if (widgets.empty())
		return false;

	switch (key) {
		case EKEY_F8:
		case EKEY_META | '8':
			next();
			break;
		case EKEY_F8 | EKEY_SHIFT:
			previous();
			break;
		default:
			return (*current)->process_key(key);
	}
	return true;
}

bool split_t::set_size(optint height, optint width) {
	bool result;

	if (!height.is_valid())
		height = t3_win_get_height(window);
	if (!width.is_valid())
		width = t3_win_get_width(window);

	result = t3_win_resize(window, height, width);

	if (horizontal) {
		int idx;
		int step = height / widgets.size();
		int left_over = height % widgets.size();
		widgets_t::iterator iter;

		for (iter = widgets.begin(), idx = 0; iter != widgets.end(); iter++, idx++) {
			result &= (*iter)->set_size(step + (idx < left_over), width);
			(*iter)->set_position(idx * step + min(idx, left_over), 0);
		}
	} else {
		int idx;
		int step = width / widgets.size();
		int left_over = width % widgets.size();
		widgets_t::iterator iter;

		for (iter = widgets.begin(), idx = 0; iter != widgets.end(); iter++, idx++) {
			result &= (*iter)->set_size(height, step + (idx < left_over));
			(*iter)->set_position(0, idx * step + min(idx, left_over));
		}
	}
	return result;
}

void split_t::update_contents(void) {
	for (widgets_t::iterator iter = widgets.begin(); iter != widgets.end(); iter++)
		(*iter)->update_contents();
}

void split_t::set_focus(bool _focus) {
	focus = _focus;
	(*current)->set_focus(focus);
}

void split_t::force_redraw(void) {
	for (widgets_t::iterator iter = widgets.begin(); iter != widgets.end(); iter++)
		(*iter)->force_redraw();
}

void split_t::split(widget_t *widget, bool _horizontal) {
	split_t *current_window = dynamic_cast<split_t *>(*current);

	if (current_window != NULL) {
		current_window->split(widget, _horizontal);
	} else if (widgets.size() == 1 || _horizontal == horizontal) {
		horizontal = _horizontal;
		set_widget_parent(widget);
		widget->set_anchor(this, 0);
		widget->show();
		if (focus)
			(*current)->set_focus(false);
		current++;
		current = widgets.insert(current, widget);
		set_size(None, None);
		if (focus)
			(*current)->set_focus(true);
	} else {
		/* Create a new split_t with the current widget as its contents. Then
		   add split that split_t to splice in the requested widget. */
		(*current)->set_focus(false);
		current_window = new split_t(*current);
		set_widget_parent(current_window);
		current_window->set_focus(focus);
		current_window->split(widget, _horizontal);
		*current = current_window;
		set_size(None, None);
	}
}

bool split_t::unsplit(widget_t **widget) {
	split_t *current_window = dynamic_cast<split_t *>(*current);

	if (current_window == NULL) {
		/* This should not happen for previously split windows. However, for
		   the first split_t instance this may be the case, so we have to
		   handle it. */
		if (widgets.size() == 1)
			return true;
		*widget = *current;
		current = widgets.erase(current);
		if (current == widgets.end()) {
			current--;
			if ((current_window = dynamic_cast<split_t *>(*current)) != NULL)
				current_window->set_to_end();
		} else {
			if ((current_window = dynamic_cast<split_t *>(*current)) != NULL)
				current_window->set_to_begin();
		}
		if (focus)
			(*current)->set_focus(true);
		set_size(None, None);
		if (widgets.size() == 1)
			return true;
	} else {
		if (current_window->unsplit(widget)) {
			*current = current_window->widgets.front();
			set_widget_parent(*current);
			(*current)->set_anchor(this, 0);
			current_window->widgets.clear();
			delete current_window;
			if (focus)
				(*current)->set_focus(true);
			set_size(None, None);
		}
	}
	return false;
}

widget_t *split_t::unsplit(void) {
	widget_t *result = NULL;
	unsplit(&result);
	return result;
}

bool split_t::next_recurse(void) {
	split_t *current_window = dynamic_cast<split_t *>(*current);
	if (current_window == NULL || !current_window->next_recurse()) {
		(*current)->set_focus(false);
		current++;
		if (current != widgets.end()) {
			if ((current_window = dynamic_cast<split_t *>(*current)) != NULL)
				current_window->set_to_begin();
			(*current)->set_focus(focus);
			return true;
		} else {
			current--;
			return false;
		}
	}
	return true;
}

bool split_t::previous_recurse(void) {
	split_t *current_window = dynamic_cast<split_t *>(*current);
	if (current_window == NULL || !current_window->previous_recurse()) {
		(*current)->set_focus(false);
		if (current == widgets.begin())
			return false;
		current--;

		if ((current_window = dynamic_cast<split_t *>(*current)) != NULL)
			current_window->set_to_end();
		(*current)->set_focus(true);
	}
	return true;
}

void split_t::next(void) {
	split_t *current_window = dynamic_cast<split_t *>(*current);
	if (current_window == NULL || !current_window->next_recurse()) {
		(*current)->set_focus(false);
		current++;
		if (current == widgets.end())
			current = widgets.begin();

		if ((current_window = dynamic_cast<split_t *>(*current)) != NULL)
			current_window->set_to_begin();
		(*current)->set_focus(focus);
	}
}

void split_t::previous(void) {
	split_t *current_window = dynamic_cast<split_t *>(*current);
	if (current_window == NULL || !current_window->previous_recurse()) {
		(*current)->set_focus(false);
		if (current == widgets.begin())
			current = widgets.end();
		current--;

		if ((current_window = dynamic_cast<split_t *>(*current)) != NULL)
			current_window->set_to_end();
		(*current)->set_focus(focus);
	}
}

widget_t *split_t::get_current(void) {
	split_t *current_window = dynamic_cast<split_t *>(*current);
	if (current_window == NULL)
		return *current;
	else
		return current_window->get_current();
}

void split_t::set_to_begin(void) {
	split_t *current_window;

	current = widgets.begin();
	if ((current_window = dynamic_cast<split_t *>(*current)) != NULL)
		current_window->set_to_begin();
}

void split_t::set_to_end(void) {
	split_t *current_window;

	current = widgets.end();
	current--;
	if ((current_window = dynamic_cast<split_t *>(*current)) != NULL)
		current_window->set_to_end();
}

}; // namespace
