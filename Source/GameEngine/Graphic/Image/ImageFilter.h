/*
Copyright (C) 2015 Aaron Suen <warr1024@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include "Graphic/GraphicStd.h"

#include "Graphic/Resource/Color.h"
#include "Graphic/Resource/Texture/Texture2.h"

#include "Mathematic/Geometric/Rectangle.h"

class GRAPHIC_ITEM ImageFilter
{
public:

    /* Fill in RGB values for transparent pixels, to correct for odd colors
     * appearing at borders when blending.  This is because many PNG optimizers
     * like to discard RGB values of transparent pixels, but when blending then
     * with non-transparent neighbors, their RGB values will shpw up nonetheless.
     *
     * This function modifies the original image in-place.
     *
     * Parameter "threshold" is the alpha level below which pixels are considered
     * transparent.  Should be 127 for 3d where alpha is threshold, but 0 for
     * 2d where alpha is blended.
     */
    static void ImageCleanTransparent(eastl::shared_ptr<Texture2> const& src, unsigned int threshold);

    /* Scale a region of an image into another image, using nearest-neighbor with
     * anti-aliasing; treat pixels as crisp rectangles, but blend them at boundaries
     * to prevent non-integer scaling ratio artifacts.  Note that this may cause
     * some blending at the edges where pixels don't line up perfectly, but this
     * filter is designed to produce the most accurate results for both upscaling
     * and downscaling.
     */
    static void ImageScaleNNAA(eastl::shared_ptr<Texture2> const& src, 
        const RectangleShape<2, int>& srcRect, eastl::shared_ptr<Texture2> const& dest);
};

#endif