Sim City 4 Random Region Creator
================================

This tool allows you to quickly create whole regions
(heightmaps, actually) for Sim City 4. So if you have already filled all of the default
regions and if you are too lazy to build your own, this is just the right thing for you!

How to use:
-----------

Just start sc4rrc.exe and choose the options you like. The program will then create
a heightmap that you can import as a region in SC4. It will also create a preview
image that gives you a better impression of how the region will look like in the game
because you can already see water and mountains on it.

General Options:
----------------

#### width	
The desired width of the region in kilometers. 1 km = 1 small city.

Don't make the region too large! The SC4RRC might not be impressed by high
region sizes, but you'll still have to import the heightmap in the game and
that takes some time, even on fast machines. You must enter an integer number
here.

Example: 10

#### height
The desired height of the region. (This is not the same as the height you
specify with level. Width and height define the size of your region.) You must
enter an integer number here.

Example: 10

#### level
The average terrain height. The terrain height must always be between 0 and 255.
Sea level is at 83. You can use this setting to move the whole landscape up or
down and create or remove lakes this way. This must also be an integer number.

Example: 100

#### blur
After the terrain generation is complete, you can tell the program to apply a
blur filter to the heightmap. This is especially recommended when using the
Triangle Grid terrain generator (see below) to get rid of the edges. If you
increase this number, the filter will be applied multiple times, resulting in
an ever smoother heightmap. You have to enter an integer number here. Note 
that you can enter 0 here to disable blurring altogether.

Example: 3

#### seed
The seed is used to initialize the random number generator. If you use the same
seed twice, you'll also get the same terrain. This way, if you have found a
terrain whose general layout you like, you can write down the seed and then
play with the other settings (level, blur, detail level) to further refine it.
When asked for the seed, you can also type in "r" (without the quotes) to let
the program choose a random seed. The seed must be an integer number between 1 and 4,294,967,295.

Example: 12345


Terrain Generators
------------------
Currently, there are two different terrain generators available: The Triangle Grid
generator and the Perlin Noise generator. They produce quite different looking results
but the best thing is to just try and see for yourself. They also have a set of options
on their own which you only need to enter if you chose the respective generator.


### Triangle Grid Options

#### steepness
This defines how strong the height differences in your terrain may be. Just play
around with it a little and you'll get a feel for it. This value can be a floating point
number, and actually you should choose a value between 0 and 1.

Example: 0.5

#### detail level
This defines how many details you want in the heightmap. If you choose very
small numbers here, the result will look really stupid (although you'll get an
idea of how the algorithm works ;-), if you choose some insanely high value, it
will take ages to compute. The best is to start with a small value (5 or so) and
then slowly increase it. You should be aware that the computation time increases
exponentially with the detail level, so don't be surprised if level 10 takes 
MUCH longer than level 9. This value must be an integer number.

Example: 6



### Perlin Noise Options

#### roughness
The higher this is, the steeper will the slopes on the terrain be. 
Increasing this value makes the mountains higher and the valleys deeper.

Example: 0.5

#### detail level
Detail of computation. The higher this is, the more detailed the resulting 
landscape will be, but it will also take longer to compute. Also, more detail 
is not always what you want. Keep in mind that you still want to build a city 
on the map.

Example: 6

#### peak
The highest value the algorithm is allowed to generate. Allowed values are
between 0 and 255. Keep in mind that sea level is at 83.

Example: 220

#### bottom
The lowest value the algorithm is allowed to generate. See peak.

Example: 30

#### water percentage
How much of the map should be covered in water. Even though this says percentage,
the program actually expects a number between 0 and 1.

Example: 0.2