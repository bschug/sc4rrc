#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

void blurImage (SDL_Surface* image, int blur_amount);
void adjustMinMax (SDL_Surface* image, int min, int max);
void adjustWaterPercentage (SDL_Surface* image, float percentage);
void adjustLevels (SDL_Surface* image);

#endif