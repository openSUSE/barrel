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
    output(std::ostream& s, const Table::Row& row, const vector<size_t>& widths, const vector<Align>& aligns,
	   bool indent, bool last)
    {
	if (indent)
	    s << (last ? "└─" : "├─");

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
		    s << std::string(widths[i] - column.size() - extra, ' ');
	    }

	    s << column;

	    if (last)
		break;

	    if (aligns[i] == Align::LEFT)
	    {
		if (column.size() < widths[i] - extra)
		    s << std::string(widths[i] - column.size() - extra, ' ');
	    }

	    s << " │";
	}

	s << '\n';
    }


    void
    output(std::ostream& s, const vector<size_t>& widths)
    {
	for (size_t i = 0; i < widths.size(); ++i)
	{
	    for (size_t j = 0; j < widths[i]; ++j)
		s << "─";

	    if (i == widths.size() - 1)
		break;

	    s << "─┼─";
	}

	s << '\n';
    }


    string&
    Table::Row::operator[](Id id)
    {
	for (size_t i = 0; i < table.ids.size(); ++i)
	{
	    if (table.ids[i] == id)
	    {
		if (columns.size() < i + 1)
		    columns.resize(i + 1);

		return columns[i];
	    }
	}

	throw runtime_error("id not found");
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


    std::ostream&
    operator<<(std::ostream& s, const Table& table)
    {
	vector<size_t> widths;

	// calculate widths

	calculate_widths(widths, table.header, false);
	for (const Table::Row& row : table.rows)
	{
	    calculate_widths(widths, row, false);

	    const vector<Table::Row>& subrows = row.get_subrows();
	    for (const Table::Row& subrow : subrows)
		calculate_widths(widths, subrow, true);
	}

	// output header and rows

	output(s, table.header, widths, table.aligns, 0, false);
	output(s, widths);
	for (const Table::Row& row : table.rows)
	{
	    output(s, row, widths, table.aligns, 0, false);

	    const vector<Table::Row>& subrows = row.get_subrows();
	    for (size_t i = 0; i < subrows.size(); ++i)
		output(s, subrows[i], widths, table.aligns, true, i == subrows.size() - 1);
	}

	return s;
    }

}
