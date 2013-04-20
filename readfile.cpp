#include <iostream>

#include "world.h"

using std::cout;
using std::endl;

int main()
{
    World w("maps/simple.map");

    cout << "Loaded simple.map" << endl;
    cout << "Size: " << w.getsizeX() << "," << w.getsizeY() << endl;
    cout << "Start: " << w.getstartX() << "," << w.getstartY() << endl;
    cout << "Goal: " << w.getgoalX() << "," << w.getgoalY() << endl;
}
