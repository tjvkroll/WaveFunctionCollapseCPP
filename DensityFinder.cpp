#include "wfc.h"

using namespace std; 

// Blocks used to build image
vector<BuildingBlock> BUILDINGBLOCKS{
    BuildingBlock("Blank", vector<uint8_t>{0, 0, 0, 0, 0, 0, 0, 0, 0},
                  new bool[4]{false, false, false, false}, 10, 0.0),
        BuildingBlock("Intersection", vector<uint8_t>{0, 1, 0, 1, 1, 1, 0, 1, 0},
                  new bool[4]{true, true, true, true}, 10, 5.0),              
    BuildingBlock("vertPipe", vector<uint8_t>{0, 1, 0, 0, 1, 0, 0, 1, 0},
                  new bool[4]{true, true, false, false}, 5, 3.0),
    BuildingBlock("HoriPipe", vector<uint8_t>{0, 0, 0, 1, 1, 1, 0, 0, 0},
                  new bool[4]{false, false, true, true}, 5, 3.0),
    BuildingBlock("TBottom", vector<uint8_t>{0, 0, 0, 1, 1, 1, 0, 1, 0},
                  new bool[4]{false, true, true, true}, 3, 4.0),
    BuildingBlock("TRight", vector<uint8_t>{0, 1, 0, 0, 1, 1, 0, 1, 0},
                  new bool[4]{true, true, false, true}, 3, 4.0),
    BuildingBlock("TLeft", vector<uint8_t>{0, 1, 0, 1, 1, 0, 0, 1, 0},
                  new bool[4]{true, true, true, false}, 3, 4.0),
    BuildingBlock("TUp", vector<uint8_t>{0, 1, 0, 1, 1, 1, 0, 0, 0},
                  new bool[4]{true, false, true, true}, 3, 4.0),
    BuildingBlock("CornerBottomRight",
                  vector<uint8_t>{0, 0, 0, 0, 1, 1, 0, 1, 0},
                  new bool[4]{false, true, false, true}, 3, 3.0),
    BuildingBlock("CornerBottomLeft",
                  vector<uint8_t>{0, 0, 0, 1, 1, 0, 0, 1, 0},
                  new bool[4]{false, true, true, false}, 3, 3.0),
    BuildingBlock("CornerTopRight", vector<uint8_t>{0, 1, 0, 0, 1, 1, 0, 0, 0},
                  new bool[4]{true, false, false, true}, 3, 3.0),
    BuildingBlock("CornerTopLeft", vector<uint8_t>{0, 1, 0, 1, 1, 0, 0, 0, 0},
                  new bool[4]{true, false, true, false}, 3, 3.0),
};






int main (int argc, char *argv[]){
    srand(time(0));
  struct timespec start_t, wfc_end_t, pix_end_t, dens_end_t; 
  double start, wfc_end, pix_end, dens_end, min = 2.0, max = 0.0; 
	int size, num_threads, ndim, min_max; 
	string file, outfile = "result.pgm"; 
  

	if (argc < 4 && argc > 1) {
    cout << "Invalid args: <exe> <outfile> <Dim> <num_threads>\n";
    exit(1);
  }

	if(argc > 1){
		min_max = stoi(argv[1]);
		ndim = stoi(argv[2]);
		num_threads = stoi(argv[3]);
		printf("Running with mode: %d ndim: %d num_threads: %d\n", min_max, ndim, num_threads);
	}
	else{
		cout << "Would you it to be small, medium, or large?\n1) Small (40x40)\n2) Medium (100x100)\n3) Large(200x200)\n";
		cin >> size;
		cout << "Would you like to find the minimum or maximum density:\n1)minimum\n2)maximum\n";
		cin >> min_max;
		cout << "Please enter the a name for your output file (no extension)\n";
		cin >> file; 
		outfile = file + ".pgm"; 

		cout << "How many threads would you like to utilize(4 is recommended on most systems):\n";
		cin >> num_threads; 
		
		if(size < 1 || size > 3){
			cout << "Incorrect size passed, defaulting to small.\n";
			ndim = 40; 
		}
		if (size == 1) ndim = 40;
		else if(size ==2) ndim = 100;
		else if(size == 3) ndim = 200; 
	}

	vector<vector<WFCBlock>> placeholder; 

  clock_gettime(CLOCK_MONOTONIC, &start_t);
#pragma omp parallel for num_threads(num_threads)
	for(int i = 0; i < 100; i++){
	WFC wfc(ndim, ndim, WFCBlock(BUILDINGBLOCKS), num_threads);

		wfc.MakeMaze();
		while (1) {
			try {
				wfc.Iterate();
			} catch (std::runtime_error const& e) {
					// searching for min/max 
#pragma omp critical 
{
					double tmp = wfc.Density();
					if(min_max == 2){
						if(tmp > max){
							max = tmp;
							placeholder = wfc.GetTilemap();
						}
					}
					else{
						if(tmp < min){
							min = tmp; 
							placeholder = wfc.GetTilemap();
						}
					}
}
				break;
			}
		}
	}

	WFC wfc_final(ndim, ndim, WFCBlock(BUILDINGBLOCKS), num_threads);
	wfc_final.SetTilemap(placeholder);

  clock_gettime(CLOCK_MONOTONIC,& wfc_end_t);
  cout << "Density: " << to_string(wfc_final.Density()) << endl;
  clock_gettime(CLOCK_MONOTONIC,& dens_end_t);

  cout << "Generating pixmap: " << outfile << endl << endl;
  PGMDAT d = wfc_final.Tilemap_To_Pgmdat();
  PGM::WritePGM(outfile, d);
  clock_gettime(CLOCK_MONOTONIC,& pix_end_t);


  // Timing for analysis
  start = start_t.tv_sec + (start_t.tv_nsec / 1000000000.0);
  wfc_end = wfc_end_t.tv_sec + (wfc_end_t.tv_nsec / 1000000000.0);
  pix_end = pix_end_t.tv_sec + (pix_end_t.tv_nsec / 1000000000.0);
  dens_end = dens_end_t.tv_sec + (dens_end_t.tv_nsec / 1000000000.0);

  cout << "~~~ Statistics ~~~ \n";
  cout << "Time in seconds for each section:\n"<< setprecision(15); 
  cout << "WFC: " << (wfc_end-start)<< endl;
  cout << "Density: " << (dens_end-wfc_end)<< endl;
  cout << "PGM generation and write to file: " << (pix_end-dens_end)<< endl;
  cout << "TOTAL: " << (pix_end-start)<< endl << endl;

    return 0; 
}