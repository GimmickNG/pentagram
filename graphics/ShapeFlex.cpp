/*
 *  Copyright (C) 2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

#include "ShapeFlex.h"
#include "Shape.h"
#include "Palette.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ShapeFlex,Flex);

ShapeFlex::ShapeFlex(IDataSource* ds, const uint16 ident, Pentagram::Palette* pal,
					 const ConvertShapeFormat *fmt)
	: Flex(ds), id(ident), format(fmt), palette(pal)
{
	shapes.resize(get_count()); // shape pointers are initialized to 0 by this
}


ShapeFlex::~ShapeFlex()
{
	for (unsigned int i = 0; i < shapes.size(); i++)
	{
		delete shapes[i];
	}
	shapes.clear();
}

Shape* ShapeFlex::getShape(uint32 shapenum)
{
	if (shapenum >= shapes.size()) return 0;

	cache(shapenum);

	return shapes[shapenum];
}

void ShapeFlex::cache(uint32 shapenum)
{
	if (shapenum >= shapes.size()) return;
	if (shapes[shapenum]) return;

	uint8 *data = get_object(shapenum);
	uint32 shpsize = get_size(shapenum);

	if (!data || shpsize == 0) return;

	// Auto detect format
	if (!format) format = Shape::DetectShapeFormat(data,shpsize);
	
	if (!format)
	{
		delete [] data;
		perr << "Error: Unable to detect shape format for flex." << std::endl;
		return;
	}

	Shape* shape = new Shape(data, shpsize, format, id, shapenum);
	if (palette) shape->setPalette(palette);

	shapes[shapenum] = shape;
}

void ShapeFlex::cache()
{
	for (unsigned int i = 0; i < shapes.size(); i++)
		cache(i);
}

void ShapeFlex::uncache(uint32 shapenum)
{
	if (shapenum >= shapes.size()) return;

	delete shapes[shapenum];
	shapes[shapenum] = 0;
}
