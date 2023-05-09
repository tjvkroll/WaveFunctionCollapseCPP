#pragma once

#include <cmath>
#include <iostream>
#include <string>
#include <time.h>
#include <iomanip>

#include "block.h"
#include "pgm.h"

class WFC {
  
  private:
    int threads;
	int height; 
	int width;   
    vector<vector<WFCBlock>> tilemap;
  
    // HELPFUL STRUCTS: 
    struct location {
    	location() {
      	  x = -1;
        	y = -1;
    	}
    	location(int x, int y) : x(x), y(y) {}
    	int x;
    	int y;
    };

    struct Neighbor {
    	Neighbor(DIRECTION d, location l) : dir(d), loc(l) {}
    	DIRECTION dir;
    	location loc;
    };

	vector<Neighbor> get_neighbors(location loc, int height, int width);
	location get_location_with_fewest_choices();
	vector<location> get_locations_requiring_updates();
	bool updatesRequired();
	void resetUpdateValues();
	bool add_constraint(Neighbor neighbor, vector<BuildingBlock> sourceSuperPositions);

  public: 
	// variable height/width
	WFC(int height, int width, WFCBlock choices, int num_threads) 
		:threads(num_threads), height(height), width(width){
		  tilemap = vector<vector<WFCBlock>>(height, vector<WFCBlock>(width, choices));
		};
	
	vector<vector<WFCBlock>> GetTilemap();
	int GetNumThreads();
	void SetTilemap(vector<vector<WFCBlock>> map);
    void DisplayTilemap();
	double Density();
	void Propogate(location next_location);
	void Iterate();
	void IterateSpecific(int x, int y, string name);
	PGMDAT Tilemap_To_Pgmdat();

	void MakeMaze(); 
};