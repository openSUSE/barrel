/*
 * Copyright (c) [2021-2022] SUSE LLC
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


#ifndef BARREL_TABLE_H
#define BARREL_TABLE_H


#include <string>
#include <vector>
#include <iostream>


namespace barrel
{

    using namespace std;


    enum class Id
    {
	NONE, NAME, SIZE, USAGE, POOL, USED, NUMBER, STRIPES, LABEL, MOUNT_POINT, PROFILES
    };


    enum class Align
    {
	LEFT, RIGHT
    };


    enum class Visibility
    {
	ON, AUTO, OFF
    };


    enum class Style
    {
	STANDARD, DOUBLE, ASCII, NONE
    };


    struct Cell
    {
	Cell(const char* name) : name(name) {}
	Cell(const char* name, Id id) : name(name), id(id) {}
	Cell(const char* name, Align align) : name(name), align(align) {}
	Cell(const char* name, Id id, Align align) : name(name), id(id), align(align) {}

	const char* name;
	const Id id = Id::NONE;
	const Align align = Align::LEFT;
    };


    class Table
    {

    public:

	explicit Table(std::initializer_list<Cell> init);

	class Row
	{
	public:

	    explicit Row(const Table& table)
		: table(table) {}

	    explicit Row(const Table& table, std::initializer_list<string> init)
		: table(table), columns(init) {}

	    const Table& get_table() const { return table; }

	    void add(const string& s) { columns.push_back(s); }

	    string& operator[](Id id);

	    const vector<string>& get_columns() const { return columns; }

	    void add_subrow(const Row& row) { subrows.push_back(row); }

	    const vector<Row>& get_subrows() const { return subrows; }

	private:

	    // backref, used when accessing columns by id
	    const Table& table;

	    vector<string> columns;

	    vector<Row> subrows;

	};

	class Header : public Row
	{
	public:

	    explicit Header(const Table& table) : Row(table) {}

	    explicit Header(const Table& table, std::initializer_list<string> init)
		: Row(table, init) {}

	};

	void add(const Row& row) { rows.push_back(row); }

	void add(std::initializer_list<string> init) { rows.emplace_back(Row(*this, init)); }

	const vector<Row> get_rows() const { return rows; }

	void set_style(enum Style style) { Table::style = style; }
	void set_global_indent(size_t global_indent) { Table::global_indent = global_indent; }
	void set_min_width(Id id, size_t min_width);
	void set_visibility(Id id, Visibility visibility);
	void set_tree_id(Id id);

	friend std::ostream& operator<<(std::ostream& s, const Table& Table);

    private:

	Header header;
	vector<Row> rows;

	Style style = Style::STANDARD;
	size_t global_indent = 0;

	vector<Align> aligns;
	vector<Id> ids;
	vector<size_t> min_widths;
	vector<Visibility> visibilities;
	size_t tree_index = 0;

	size_t id_to_index(Id id) const;

	struct OutputInfo
	{
	    OutputInfo(const Table& table);

	    void calculate_hidden(const Table& table, const Table::Row& row);
	    void calculate_widths(const Table& table, const Table::Row& row, unsigned indent);

	    vector<bool> hidden;
	    vector<size_t> widths;
	};

	void output(std::ostream& s, const Table::Row& row, const OutputInfo& output_info, const vector<bool>& lasts) const;

	void output(std::ostream& s, const OutputInfo& output_info) const;

	const char* glyph(unsigned int i) const;

    };

}

#endif
