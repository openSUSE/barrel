/*
 * Copyright (c) 2021 SUSE LLC
 *
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, contact SUSE LLC.
 *
 * To contact SUSE LLC about this file by physical or electronic mail, you may
 * find current contact information at www.suse.com.
 */


#include "Table.h"
#include "Text.h"


namespace barrel
{

    using namespace std;


    void
    Table::calculate_widths(const Table::Row& row, vector<size_t>& widths, unsigned indent) const
    {
	const vector<string>& columns = row.get_columns();

	if (columns.size() > widths.size())
	    widths.resize(columns.size());

	for (size_t i = 0; i < columns.size(); ++i)
	{
	    size_t width = mbs_width(columns[i]);

	    if (i == tree_index)
		width += 2 * indent;

	    widths[i] = max(widths[i], width);
	}

	for (const Table::Row& subrow : row.get_subrows())
	    calculate_widths(subrow, widths, indent + 1);
    }


    void
    Table::output(std::ostream& s, const Table::Row& row, const vector<size_t>& widths,
		  const vector<bool>& lasts) const
    {
	s << string(global_indent, ' ');

	const vector<string>& columns = row.get_columns();

	for (size_t i = 0; i < widths.size(); ++i)
	{
	    string column = i < columns.size() ? columns[i] : "";

	    bool first = i == 0;
	    bool last = i == widths.size() - 1;

	    size_t extra = (i == tree_index) ? 2 * lasts.size() : 0;

	    if (last && column.empty())
		break;

	    if (!first)
		s << " ";

	    if (i == tree_index)
	    {
		for (size_t tl = 0; tl < lasts.size(); ++tl)
		{
		    if (tl == lasts.size() - 1)
			s << (lasts[tl] ? glyph(4) : glyph(3));
		    else
			s << (lasts[tl] ? glyph(6) : glyph(5));
		}
	    }

	    if (aligns[i] == Align::RIGHT)
	    {
		size_t width = mbs_width(column);

		if (width < widths[i] - extra)
		    s << string(widths[i] - width - extra, ' ');
	    }

	    s << column;

	    if (last)
		break;

	    if (aligns[i] == Align::LEFT)
	    {
		size_t width = mbs_width(column);

		if (width < widths[i] - extra)
		    s << string(widths[i] - width - extra, ' ');
	    }

	    s << " " << glyph(0);
	}

	s << '\n';

	const vector<Table::Row>& subrows = row.get_subrows();
	for (size_t i = 0; i < subrows.size(); ++i)
	{
	    vector<bool> sub_lasts = lasts;
	    sub_lasts.push_back(i == subrows.size() - 1);
	    output(s, subrows[i], widths, sub_lasts);
	}
    }


    void
    Table::output(std::ostream& s, const vector<size_t>& widths) const
    {
	s << string(global_indent, ' ');

	for (size_t i = 0; i < widths.size(); ++i)
	{
	    for (size_t j = 0; j < widths[i]; ++j)
		s << glyph(1);

	    if (i == widths.size() - 1)
		break;

	    s << glyph(1) << glyph(2) << glyph(1);
	}

	s << '\n';
    }


    size_t
    Table::id_to_index(Id id) const
    {
	for (size_t i = 0; i < ids.size(); ++i)
	    if (ids[i] == id)
		return i;

	throw runtime_error("id not found");
    }


    string&
    Table::Row::operator[](Id id)
    {
	size_t i = table.id_to_index(id);

	if (columns.size() < i + 1)
	    columns.resize(i + 1);

	return columns[i];
    }


    Table::Table(std::initializer_list<Cell> init)
	: header(*this)
    {
	for (const Cell& cell : init)
	{
	    header.add(cell.name);
	    ids.push_back(cell.id);
	    aligns.push_back(cell.align);
	}
    }


    void
    Table::set_min_width(Id id, size_t min_width)
    {
	size_t i = id_to_index(id);

	if (min_widths.size() < i + 1)
	    min_widths.resize(i + 1);

	min_widths[i] = min_width;
    }


    void
    Table::set_tree_id(Id id)
    {
	tree_index = id_to_index(id);
    }


    std::ostream&
    operator<<(std::ostream& s, const Table& table)
    {
	vector<size_t> widths = table.min_widths;

	// calculate widths

	if (table.style != Style::NONE)
	    table.calculate_widths(table.header, widths, 0);

	for (const Table::Row& row : table.rows)
	    table.calculate_widths(row, widths, 0);

	// output header and rows

	if (table.style != Style::NONE)
	{
	    table.output(s, table.header, widths, {});
	    table.output(s, widths);
	}

	for (const Table::Row& row : table.rows)
	    table.output(s, row, widths, {});

	return s;
    }


    const char*
    Table::glyph(unsigned int i) const
    {
	const char* glyphs[][7] = {
	    { "│", "─", "┼", "├─", "└─", "│ ", "  " },  // STANDARD
	    { "║", "═", "╬", "├─", "└─", "│ ", "  " },  // DOUBLE
	    { "|", "-", "+", "+-", "+-", "| ", "  " },  // ASCII
	    { "",  "",  "",  "  ", "  ", "  ", "  " }   // NONE
	};

	return glyphs[(unsigned int)(style)][i];
    }

}
