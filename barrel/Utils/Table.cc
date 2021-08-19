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


namespace barrel
{

    using namespace std;


    void
    calculate_widths(vector<size_t>& widths, const Table::Row& row, bool indent)
    {
	// TODO utf-8 and wide chars

	const vector<string>& columns = row.get_columns();

	if (columns.size() > widths.size())
	    widths.resize(columns.size());

	for (size_t i = 0; i < columns.size(); ++i)
	{
	    size_t w = columns[i].size();

	    if (i == 0 && indent)
		w += 2;

	    widths[i] = max(widths[i], w);
	}
    }


    void
    Table::output(std::ostream& s, const Table::Row& row, const vector<size_t>& widths,
		  const vector<Align>& aligns, bool indent, bool last) const
    {
	s << string(global_indent, ' ');

	if (indent)
	    s << (last ? glyph(4) : glyph(3));

	const vector<string>& columns = row.get_columns();

	for (size_t i = 0; i < widths.size(); ++i)
	{
	    string column = i < columns.size() ? columns[i] : "";

	    bool first = i == 0;
	    bool last = i == widths.size() - 1;

	    size_t extra = first && indent ? 2 : 0;

	    if (last && column.empty())
		break;

	    if (!first)
		s << " ";

	    if (aligns[i] == Align::RIGHT)
	    {
		if (column.size() < widths[i] - extra)
		    s << string(widths[i] - column.size() - extra, ' ');
	    }

	    s << column;

	    if (last)
		break;

	    if (aligns[i] == Align::LEFT)
	    {
		if (column.size() < widths[i] - extra)
		    s << string(widths[i] - column.size() - extra, ' ');
	    }

	    s << " " << glyph(0);
	}

	s << '\n';
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


    std::ostream&
    operator<<(std::ostream& s, const Table& table)
    {
	vector<size_t> widths = table.min_widths;

	// calculate widths

	if (table.style != Style::NONE)
	{
	    calculate_widths(widths, table.header, false);
	}

	for (const Table::Row& row : table.rows)
	{
	    calculate_widths(widths, row, false);

	    const vector<Table::Row>& subrows = row.get_subrows();
	    for (const Table::Row& subrow : subrows)
		calculate_widths(widths, subrow, true);
	}

	// output header and rows

	if (table.style != Style::NONE)
	{
	    table.output(s, table.header, widths, table.aligns, 0, false);
	    table.output(s, widths);
	}

	for (const Table::Row& row : table.rows)
	{
	    table.output(s, row, widths, table.aligns, 0, false);

	    const vector<Table::Row>& subrows = row.get_subrows();
	    for (size_t i = 0; i < subrows.size(); ++i)
		table.output(s, subrows[i], widths, table.aligns, true, i == subrows.size() - 1);
	}

	return s;
    }


    const char*
    Table::glyph(unsigned int i) const
    {
	const char* glyphs[][5] = {
	    { "│", "─", "┼", "├─", "└─" },
	    { "|", "-", "+", " -", " -" },
	    { "", "", "", "  ", "  " }
	};

	return glyphs[(unsigned int)(style)][i];
    }

}
