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

#include "ImageFilter.h"


void ImageFilter::ImageCleanTransparent(eastl::shared_ptr<Texture2> const& src, unsigned int threshold)
{
	// Walk each pixel looking for fully transparent ones.
	// Note: loop y around x for better cache locality.
    unsigned int const numLevels = src->GetNumLevels();
    for (unsigned int level = 0; level < numLevels; ++level)
    {
        unsigned int uSize = src->GetDimensionFor(level, 0);
        unsigned int vSize = src->GetDimensionFor(level, 1);
        auto srcData = reinterpret_cast<unsigned int*>(src->GetDataFor(level));
        for (unsigned int v = 0; v < vSize; ++v)
        {
            for (unsigned int u = 0; u < uSize; ++u)
            {
                // Ignore opaque pixels.
                SColor srcColor = SColor(srcData[u + uSize * v]);
                if (srcColor.GetAlpha() > threshold)
                    continue;

                // Sample size and total weighted r, g, b values.
                unsigned int ss = 0, sr = 0, sg = 0, sb = 0;

                // Walk each neighbor pixel (clipped to image bounds).
                for (unsigned int sv = (v < 1) ? 0 : (v - 1); sv <= (v + 1) && sv < vSize; sv++)
                {
                    for (unsigned int su = (u < 1) ? 0 : (u - 1); su <= (u + 1) && su < uSize; su++)
                    {
                        // Ignore transparent pixels.
                        SColor destColor = SColor(srcData[su + uSize * sv]);
                        if (destColor.GetAlpha() <= threshold)
                            continue;

                        // Add RGB values weighted by alpha.
                        unsigned int alpha = destColor.GetAlpha();
                        ss += alpha;
                        sr += alpha * destColor.GetRed();
                        sg += alpha * destColor.GetGreen();
                        sb += alpha * destColor.GetBlue();
                    }
                }

                // If we found any neighbor RGB data, set pixel to average
                // weighted by alpha.
                if (ss > 0)
                {
                    srcColor.SetRed(sr / ss);
                    srcColor.SetGreen(sg / ss);
                    srcColor.SetBlue(sb / ss);
                    srcColor.GetData(&srcData[u + uSize * v], DF_R32G32B32A32_UINT);
                }
            }
        }
    }
}

void ImageFilter::ImageScaleNNAA(eastl::shared_ptr<Texture2> const& src,
    const RectangleShape<2, int>& srcRect, eastl::shared_ptr<Texture2> const& dest)
{
    double sx, sy, minsx, maxsx, minsy, maxsy, area, ra, ga, ba, aa, pw, ph, pa;
    unsigned int dy, dx;
    SColor colorPixel;

    // Cache rectangle boundaries.
    double sox = srcRect.mCenter[0] - (srcRect.mExtent[0] / 2);
    double soy = srcRect.mCenter[1] - (srcRect.mExtent[1] / 2);
    double sw = srcRect.mExtent[0];
    double sh = srcRect.mExtent[1];

    // Walk each destination image pixel.
    // Note: loop y around x for better cache locality.
    unsigned int const numLevels = dest->GetNumLevels();
    for (unsigned int level = 0; level < numLevels; ++level)
    {
        unsigned int dimWidth = dest->GetDimensionFor(level, 0);
        unsigned int dimHeight = dest->GetDimensionFor(level, 1);
        auto destData = reinterpret_cast<unsigned int*>(dest->GetDataFor(level));
        for (dy = 0; dy < dimHeight; ++dy)
        {
            for (dx = 0; dx < dimWidth; ++dx)
            {
                // Calculate floating-point source rectangle bounds.
                // Do some basic clipping, and for mirrored/flipped rects,
                // make sure min/max are in the right order.
                minsx = sox + (dx * sw / dimWidth);
                minsx = eastl::clamp(minsx, (double)0, sox + sw);
                maxsx = minsx + sw / dimWidth;
                maxsx = eastl::clamp(maxsx, (double)0, sox + sw);
                if (minsx > maxsx)
                    eastl::swap(minsx, maxsx);
                minsy = soy + (dy * sh / dimHeight);
                minsy = eastl::clamp(minsy, (double)0, soy + sh);
                maxsy = minsy + sh / dimHeight;
                maxsy = eastl::clamp(maxsy, (double)0, soy + sh);
                if (minsy > maxsy)
                    eastl::swap(minsy, maxsy);

                // Total area, and integral of r, g, b values over that area,
                // initialized to zero, to be summed up in next loops.
                area = 0;
                ra = 0;
                ga = 0;
                ba = 0;
                aa = 0;

                // Loop over the integral pixel positions described by those bounds.
                for (sy = floor(minsy); sy < maxsy; sy++)
                {
                    for (sx = floor(minsx); sx < maxsx; sx++)
                    {
                        // Calculate width, height, then area of dest pixel
                        // that's covered by this source pixel.
                        pw = 1;
                        if (minsx > sx)
                            pw += sx - minsx;
                        if (maxsx < (sx + 1))
                            pw += maxsx - sx - 1;
                        ph = 1;
                        if (minsy > sy)
                            ph += sy - minsy;
                        if (maxsy < (sy + 1))
                            ph += maxsy - sy - 1;
                        pa = pw * ph;

                        // Get source pixel and add it to totals, weighted
                        // by covered area and alpha.
                        colorPixel = SColor(destData[(unsigned int)sx + dimWidth * (unsigned int)sy]);
                        area += pa;
                        ra += pa * colorPixel.GetRed();
                        ga += pa * colorPixel.GetGreen();
                        ba += pa * colorPixel.GetBlue();
                        aa += pa * colorPixel.GetAlpha();
                    }

                    // Set the destination image pixel to the average color.
                    if (area > 0) 
                    {
                        colorPixel.SetRed(ra / area + 0.5);
                        colorPixel.SetGreen(ga / area + 0.5);
                        colorPixel.SetBlue(ba / area + 0.5);
                        colorPixel.SetAlpha(aa / area + 0.5);
                    }
                    else 
                    {
                        colorPixel.SetRed(0);
                        colorPixel.SetGreen(0);
                        colorPixel.SetBlue(0);
                        colorPixel.SetAlpha(0);
                    }

                    colorPixel.GetData(&destData[dx + dimWidth * dy], DF_R32G32B32A32_UINT);
                }
            }
        }
    }
}