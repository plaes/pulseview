/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "cursorpair.h"

#include "ruler.h"
#include "view.h"

#include <algorithm>

using boost::shared_ptr;
using std::max;
using std::make_pair;
using std::min;
using std::pair;

namespace pv {
namespace view {

const int CursorPair::DeltaPadding = 8;

CursorPair::CursorPair(View &view) :
	_first(new Cursor(view, 0.0)),
	_second(new Cursor(view, 1.0)),
	_view(view)
{
}

shared_ptr<Cursor> CursorPair::first() const
{
	return _first;
}

shared_ptr<Cursor> CursorPair::second() const
{
	return _second;
}

QRectF CursorPair::get_label_rect(const QRect &rect) const
{
	const QSizeF label_size(
		_text_size.width() + View::LabelPadding.width() * 2,
		_text_size.height() + View::LabelPadding.height() * 2);
	const pair<float, float> offsets(get_cursor_offsets());
	const pair<float, float> normal_offsets(
		(offsets.first < offsets.second) ? offsets :
		make_pair(offsets.second, offsets.first));

	const float height = label_size.height();
	const float left = max(normal_offsets.first + DeltaPadding, -height);
	const float right = min(normal_offsets.second - DeltaPadding,
		(float)rect.width() + height);

	return QRectF(left, rect.height() - label_size.height() -
		Cursor::ArrowSize - Cursor::Offset - 0.5f,
		right - left, height);
}

void CursorPair::draw_markers(QPainter &p,
	const QRect &rect, unsigned int prefix)
{
	assert(_first);
	assert(_second);

	compute_text_size(p, prefix);
	QRectF delta_rect(get_label_rect(rect));

	const int radius = delta_rect.height() / 2;
	const QRectF text_rect(delta_rect.intersected(
		rect).adjusted(radius, 0, -radius, 0));
	if(text_rect.width() >= _text_size.width())
	{
		const int highlight_radius = delta_rect.height() / 2 - 2;

		p.setBrush(Cursor::FillColour);
		p.setPen(Cursor::LineColour);
		p.drawRoundedRect(delta_rect, radius, radius);

		delta_rect.adjust(1, 1, -1, -1);
		p.setPen(Cursor::HighlightColour);
		p.drawRoundedRect(delta_rect, highlight_radius, highlight_radius);

		p.setPen(Cursor::TextColour);
		p.drawText(text_rect, Qt::AlignCenter | Qt::AlignVCenter,
			Ruler::format_time(_second->time() - _first->time(), prefix, 2));
	}

	// Paint the cursor markers
	_first->paint_label(p, rect, prefix);
	_second->paint_label(p, rect, prefix);
}

void CursorPair::draw_viewport_background(QPainter &p,
	const QRect &rect)
{
	p.setPen(Qt::NoPen);
	p.setBrush(QBrush(View::CursorAreaColour));

	const pair<float, float> offsets(get_cursor_offsets());
	const int l = (int)max(min(
		offsets.first, offsets.second), 0.0f);
	const int r = (int)min(max(
		offsets.first, offsets.second), (float)rect.width());

	p.drawRect(l, 0, r - l, rect.height());
}

void CursorPair::draw_viewport_foreground(QPainter &p,
	const QRect &rect)
{
	assert(_first);
	assert(_second);

	_first->paint(p, rect);
	_second->paint(p, rect);
}

void CursorPair::compute_text_size(QPainter &p, unsigned int prefix)
{
	assert(_first);
	assert(_second);

	_text_size = p.boundingRect(QRectF(), 0, Ruler::format_time(
		_second->time() - _first->time(), prefix, 2)).size();
}

pair<float, float> CursorPair::get_cursor_offsets() const
{
	assert(_first);
	assert(_second);

	return pair<float, float>(
		(_first->time() - _view.offset()) / _view.scale(),
		(_second->time() - _view.offset()) / _view.scale());
}

} // namespace view
} // namespace pv
