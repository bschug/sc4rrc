#define SC4RRC_LIB
#include "LogManager.h"

#include <SDL/SDL.h>

#include <algorithm>
#include <vector>

#include <limits>
using std::numeric_limits;

struct HeightValue
{
	Uint8 value;
	int pos;

	HeightValue(Uint8 value, int pos) : value(value),pos(pos) { }

	bool operator<(const HeightValue& v) const
	{
		return value < v.value;
	}
};

__inline float randf () { return float(rand()) / float(RAND_MAX); }
__inline int rand (int min, int max) { return min + int(randf() * max); }


/**	Blurs the image.
 *	This function assigns to each pixel the average of all surrounding pixels.
 *	This is repeated <blur_amount> times.
 */
void blurImage(SDL_Surface* image, int blur_amount)
{
	LogManager::log("blurring image",true);

	for (int i=0; i < blur_amount; i++)
    {
		for (int y=1; y < image->h - 1; y++)
		for (int x=1; x < image->w - 1; x++)
		{
			int sum = 0;
			int nr_of_samples = 0;

			for(int yy = y-1; yy <= y+1; yy++)
			for(int xx = x-1; xx <= x+1; xx++)
			{
				int ofs = xx + yy * image->pitch;
				sum += ((Uint8*)image->pixels)[ofs];
				nr_of_samples++;
			}

			int ofs = x+y*image->pitch;
			((Uint8*)image->pixels)[ofs] = (sum/nr_of_samples);
		}
    }
}


void adjustMinMax(SDL_Surface* image, Uint8 bottom, Uint8 peak)
{
    Uint8 *pixels = static_cast <Uint8*> (image->pixels);
    Uint16 pitch = image->pitch;

	// find min and max
	Uint8 min = numeric_limits<Uint8>::max();
	Uint8 max = numeric_limits<Uint8>::min();

	for (int y=0; y < image->h; y++)
    for (int x=0; x < image->w; x++)
	{
        int ofs = x + y * pitch;
		min = pixels[ofs] < min ? pixels[ofs] : min;
		max = pixels[ofs] > max ? pixels[ofs] : max;
	}

	// bring values to desired range
	float factor = float(peak-bottom) / float(max-min);

    for(int y=0; y < image->h; y++)
    for(int x=0; x < image->w; x++)
	{
        int ofs = x + y * pitch;
		pixels[ofs] -= min;
		pixels[ofs] *= factor;
		pixels[ofs] += bottom;
	}
}


/** Adjusts the water level.
 *	First, the values on the heightmap are sorted.
 *	Then the height value at the desired water percentage is retrieved.
 *	Then the values are adjusted in such a way that the value at the
 *	desired position is just at sea level.
 *	This is done by appling a third-grade polynomial on the heightfield
 *	such that the min and max points are preserved and the water level
 *	is at the desired height.
 */
void adjustWaterPercentage (SDL_Surface *image, float percentage)
{
	LogManager::log("Building heightlist");

    Uint8* pixels = static_cast <Uint8*> (image->pixels);
    Uint16 pitch = image->pitch;

	// sort height values in a list
	std::vector<HeightValue> heightlist;

    for(int y=0; y < image->h; y++)
    for(int x=0; x < image->w; x++)
	{
        int ofs = x + y * pitch;
		heightlist.push_back (HeightValue (pixels[ofs], ofs));
	}

	LogManager::log("Sorting heightlist");
	std::sort (heightlist.begin(), heightlist.end());
	
	LogManager::log("Finding current water value");
	// find current value at desired water percentage position
	int wpos = int(float(image->w * image->h) * percentage);
    float w = heightlist[wpos].value;

	// Compute coefficients for adjusting polynomial.
	// The polynomial is of the form ax²+bx+c and it must be 0 for x=0,
    // 255 for x=255 and 83 for x=w.
    // Since c must be 0, we ignore it.
    float w2 = w*w;
	float A = (83 - w) / (w2 - 255*w);
	float B = (w2 - 21165) / (w2 - 255*w);

    std::ostringstream o;
    o << "water value at position " << wpos << " is " << w << ", coefficients: A=" << A << ", B=" << B;
    LogManager::log (o.str());

	LogManager::log("adjusting height values");

    for(int y=0; y < image->h; y++)
    for(int x=0; x < image->w; x++)
	{
        int ofs = x + y * pitch;
        float h = pixels[ofs];
        float v = A * h * h + B * h;
        pixels[ofs] = v < 0 ? 0 : v > 255 ? 255 : Uint8(v);
    }
}



void adjustLevels (SDL_Surface* image)
{
    const Uint8 MIN_LEVEL_HEIGHT = 20;
    const Uint8 MAX_LEVEL_HEIGHT = 100;
    const Uint8 MIN_LEVEL_DIST = 2;
    const Uint8 MAX_LEVEL_DIST = 10;

    Uint8* pixels = static_cast <Uint8*> (image->pixels);
    Uint16 pitch = image->pitch;

    typedef std::pair<Uint8, Uint8> LevelRegion;
    std::vector<LevelRegion> levels;
    
    Uint8 levelStart = 83 + rand (0, 10);
    Uint8 cutOff = 0;

    std::ostringstream oss;
    oss << "Creating Levels at: ";

/*
    while (levelStart < 255)
    {
        Uint8 height = rand (MIN_LEVEL_HEIGHT, MAX_LEVEL_HEIGHT);
        if (levelStart > 255 - height) break;

        Uint8 levelEnd = levelStart + height;
        levels.push_back (LevelRegion (levelStart, levelEnd));
        oss << int(levelStart) << "-" << int(levelEnd);
        
        cutOff += levelEnd - levelStart;

        Uint8 dist = rand (MIN_LEVEL_DIST, MAX_LEVEL_DIST);
        if (levelEnd > 255 - dist * dist) break;

        levelStart = levelEnd + dist * dist;
        oss << ", ";
    }
*/

    levels.push_back (LevelRegion (85, 255));
    //levels.push_back (LevelRegion (175, 210));

    // termination, to make things easier in the loop below
    levels.push_back (LevelRegion (255, 255));

    oss << ", total cutoff: " << int(cutOff);
    LogManager::log (oss.str(), true);

	// sort height values in a list
	std::vector<HeightValue> heightlist;
    for(int y=0; y < image->h; y++)
    for(int x=0; x < image->w; x++)
	{
        int ofs = x + y * pitch;
		heightlist.push_back (HeightValue (pixels[ofs], ofs));
    }
    std::sort (heightlist.begin(), heightlist.end());

    typedef std::vector<LevelRegion>::const_iterator LevelIterator;
    typedef std::vector<HeightValue>::const_iterator HeightListIterator;

    // scale factor for maintaining the peak
    float scale = float (heightlist.back().value) / float (heightlist.back().value - cutOff);
    SC4_LOG ("scale factor: " << scale);

    // base cutoff for maintaining water line
    cutOff = Uint8(83 * scale) - 83;
    SC4_LOG ("base cutoff: " << cutOff);

    LevelIterator level = levels.begin();
    cutOff = 0;

    for (HeightListIterator it = heightlist.begin(); it != heightlist.end(); ++it)
    {
        int pos = it->pos;
        Uint16 height = Uint16 (it->value * scale);

        if (height > level->second)
        {
            cutOff += level->second - level->first;
            ++level;
            SC4_LOG ("reaching end of level at " << int(height) << ", cutOff = " << int(cutOff) << ", next level: " << int(level->first) << "-" << int(level->second));
        }

        if ((height > level->first) && (height < level->second))
            height = level->first;

        pixels [pos] = height - cutOff;
    }

}