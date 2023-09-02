#include "wfc.h"

#include <set>

using namespace std;

// Example building blocks: used to create maze/image
// BuildingBlock(string name, vector<int> bitmap, bool* sides, float weight,
// float density) UP DOWN LEFT RIGHT EVERY TILE IS 3x3 bitmap

// vector<BuildingBlock> BUILDINGBLOCKS{
//     BuildingBlock("Blank", vector<uint8_t>{0, 0, 0, 0, 0, 0, 0, 0, 0},
//                   new bool[4]{false, false, false, false}, 19, 0.0),
//         BuildingBlock("Intersection", vector<uint8_t>{0, 1, 0, 1, 1, 1, 0, 1,
//         0},
//                   new bool[4]{true, true, true, true}, 10, 5.0),
//     BuildingBlock("vertPipe", vector<uint8_t>{0, 1, 0, 0, 1, 0, 0, 1, 0},
//                   new bool[4]{true, true, false, false}, 5, 3.0),
//     BuildingBlock("HoriPipe", vector<uint8_t>{0, 0, 0, 1, 1, 1, 0, 0, 0},
//                   new bool[4]{false, false, true, true}, 5, 3.0),
//     BuildingBlock("TBottom", vector<uint8_t>{0, 0, 0, 1, 1, 1, 0, 1, 0},
//                   new bool[4]{false, true, true, true}, 3, 4.0),
//     BuildingBlock("TRight", vector<uint8_t>{0, 1, 0, 0, 1, 1, 0, 1, 0},
//                   new bool[4]{true, true, false, true}, 3, 4.0),
//     BuildingBlock("TLeft", vector<uint8_t>{0, 1, 0, 1, 1, 0, 0, 1, 0},
//                   new bool[4]{true, true, true, false}, 3, 4.0),
//     BuildingBlock("TUp", vector<uint8_t>{0, 1, 0, 1, 1, 1, 0, 0, 0},
//                   new bool[4]{true, false, true, true}, 3, 4.0),
//     BuildingBlock("CornerBottomRight",
//                   vector<uint8_t>{0, 0, 0, 0, 1, 1, 0, 1, 0},
//                   new bool[4]{false, true, false, true}, 3, 3.0),
//     BuildingBlock("CornerBottomLeft",
//                   vector<uint8_t>{0, 0, 0, 1, 1, 0, 0, 1, 0},
//                   new bool[4]{false, true, true, false}, 3, 3.0),
//     BuildingBlock("CornerTopRight", vector<uint8_t>{0, 1, 0, 0, 1, 1, 0, 0,
//     0},
//                   new bool[4]{true, false, false, true}, 3, 3.0),
//     BuildingBlock("CornerTopLeft", vector<uint8_t>{0, 1, 0, 1, 1, 0, 0, 0,
//     0},
//                   new bool[4]{true, false, true, false}, 3, 3.0),
// };

// Getters
int WFC::GetNumThreads() { return threads; }

vector<vector<WFCBlock>> WFC::GetTilemap() { return tilemap; }

void WFC::SetTilemap(vector<vector<WFCBlock>> map) { tilemap = map; }

// Debugging helper mostly : NOT CURRENTLY BEING CALLED
void WFC::DisplayTilemap() {
  for (auto& row : tilemap) {
    cout << "[ ";
    for (auto& tile : row) {
      cout << tile.superPositions.size() << " ";
    }
    cout << " ]" << endl;
  }
  cout << endl;
}

// Calculating the density based on pixels in final pgm
double WFC::Density() {
  double pix_dim = (double)(height * width) * 9.0;
  double pix_cnt = 0;

// PRAGMA CANDIDATE
#pragma omp parallel for num_threads(threads) reduction(+ : pix_cnt)
  for (auto& row : tilemap) {
#pragma omp parallel for num_threads(threads) reduction(+ : pix_cnt)
    for (auto& tile : row) {
      pix_cnt += tile.superPositions[0].density;
    }
  }
  double dens = pix_cnt / pix_dim;
  // cout << "pix_cnt: " << pix_cnt << "  pix_dim: " << pix_dim << "  density: "
  // << dens << endl;
  return dens;
}

// Returns a vector of neighbor tuples {Direction, location} for all the
// neighbors of any tile
vector<WFC::Neighbor> WFC::get_neighbors(location loc, int height, int width) {
  vector<Neighbor> neighborTup;
  if (loc.x != 0)
    neighborTup.push_back(
        Neighbor(DIRECTION::RIGHT, location(loc.x - 1, loc.y)));
  if (loc.y != 0)
    neighborTup.push_back(
        Neighbor(DIRECTION::DOWN, location(loc.x, loc.y - 1)));
  if (loc.x < height - 1)
    neighborTup.push_back(
        Neighbor(DIRECTION::LEFT, location(loc.x + 1, loc.y)));
  if (loc.y < width - 1)
    neighborTup.push_back(Neighbor(DIRECTION::UP, location(loc.x, loc.y + 1)));
  return neighborTup;
}

// Finds all the tiles with the lowest # of building blocks left to collapse and
// returns one at random
WFC::location WFC::get_location_with_fewest_choices() {
  int min = 100;
  vector<location> locs;
  for (int row = 0; row < width; row++) {
    for (int col = 0; col < width; col++) {
      if ((int)tilemap[row][col].superPositions.size() < min &&
          tilemap[row][col].superPositions.size() > 1) {
        min = tilemap[row][col].superPositions.size();
        locs.clear();
      }
      if ((int)tilemap[row][col].superPositions.size() == min) {
        locs.push_back(location(col, row));
      }
    }
  }
  if (!locs.empty()) {
    int randindx = rand() % locs.size();
    return locs[randindx];
  } else {
    return location();
  }
}

// Helper function used by propagate to build a list of locations that need
// updating
vector<WFC::location> WFC::get_locations_requiring_updates() {
  vector<location> locs;
  int height = tilemap.size();
  int width = tilemap[0].size();

// PRAGMA CANDIDATE
#pragma omp parallel for num_threads(threads)
  for (int row = 0; row < height; row++) {
#pragma omp parallel for num_threads(threads)
    for (int col = 0; col < width; col++) {
      if (tilemap[row][col].needsUpdate) locs.push_back(location(row, col));
    }
  }
  return locs;
}

// helper function for propagate to use as while loop condition
// if tiles need updating, keep propagating
bool WFC::updatesRequired() {
  bool needsUpdate = false;

#pragma omp parallel for num_threads(threads)
  for (int row = 0; row < height; row++) {
#pragma omp parallel for num_threads(threads)
    for (int col = 0; col < width; col++) {
      if (tilemap[row][col].needsUpdate) needsUpdate = true;
    }
  }
  return needsUpdate;
}

// helper function used in propagate
void WFC::resetUpdateValues() {
  // reset all tilemap update bools to false in prep for propogate

// PRAGMA CANDIDATE
#pragma omp parallel for num_threads(threads)
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      tilemap[row][col].needsUpdate = false;
    }
  }
}

// Constrain the available blocks of a neighbor to our source and return a bool
// true if changed, false if not
bool WFC::add_constraint(Neighbor neighbor,
                         vector<BuildingBlock> sourceSuperPositions) {
  // Get the sides of each tile we'll be constraining by
  DIRECTION incDirection = neighbor.dir;
  DIRECTION outDirection = reverse(incDirection);
  bool changed = false;
  set<bool> neighborConstraint;

  // Get set of constraints from our source tile
  for (auto& superPosition : sourceSuperPositions) {
    neighborConstraint.insert(superPosition.connections[outDirection]);
  }

  // Only want to constrain neighbors that haven't collapsed
  if ((int)tilemap[neighbor.loc.y][neighbor.loc.x].superPositions.size() != 1) {
    vector<int> indicies_to_remove;
    for (int i = 0;
         i < (int)tilemap[neighbor.loc.y][neighbor.loc.x].superPositions.size();
         i++) {
      auto searchVal = tilemap[neighbor.loc.y][neighbor.loc.x]
                           .superPositions[i]
                           .connections[incDirection];
      if (neighborConstraint.find(searchVal) == neighborConstraint.end()) {
        // remove superPosition at i
        indicies_to_remove.push_back(i);
        changed = true;
      }
    }

    // Remove building blocks that don't fit based on the constraints of our
    // source
    for (int i = indicies_to_remove.size() - 1; i >= 0; i--) {
      tilemap[neighbor.loc.y][neighbor.loc.x].RemoveSuperPositionAtIndex(
          indicies_to_remove[i]);
    }
  }

  // failsafe: if we happen to remove all options for a tile throw an error
  if (tilemap[neighbor.loc.y][neighbor.loc.x].superPositions.size() == 0) {
    cerr << "LocX: " << neighbor.loc.x << " LocY:" << neighbor.loc.y << endl;
    throw std::runtime_error("No patterns left at location.");
  }
  return changed;
}

// Meat & potatoes; Propagates constraints out to all needed tiles based on the
// most recently collapsed tile
void WFC::Propogate(location next_location) {
  // sets our starting point for propagation
  resetUpdateValues();
  tilemap[next_location.x][next_location.y].needsUpdate = true;

  // Loop to continue propagating until there are no more tiles that have been
  // constrained
  while (updatesRequired()) {
    // Gathers a list of locations that need to be propagated from
    vector<location> locations = get_locations_requiring_updates();
    resetUpdateValues();
    for (auto& location : locations) {
      // Gets all neighbors of the location we're propagating from
      vector<Neighbor> neighbors = get_neighbors(location, height, width);

// PRAGMA CANDIDATE
#pragma omp parallel for num_threads(threads)
      for (auto& neighbor : neighbors) {
        // Applying constraints to each neighbor of our source(location)
        bool wasUpdated = add_constraint(
            neighbor, tilemap[location.y][location.x].superPositions);

        // If any constraints were added, mark it as needing to update so we
        // propagate from this location next
        tilemap[neighbor.loc.y][neighbor.loc.x].needsUpdate |= wasUpdated;
      }
    }
  }
}

// Where we collapse each tile to a single building block and call propagate
void WFC::Iterate() {
  location next_location = get_location_with_fewest_choices();

  // failsafe / end if iterating
  if (next_location.x == -1) {
    throw(std::runtime_error("No more tiles to collapse!"));
  }
  if (tilemap[next_location.y][next_location.x].superPositions.size() < 2) {
    throw(std::runtime_error("No choices at tile location"));
  }
  tilemap[next_location.y][next_location.x].CollapseSuperPositions();
  Propogate(next_location);
}

// Allows for collapsing of specific blocks at specific locations
void WFC::IterateSpecific(int x, int y, string name) {
  tilemap[y][x].CollapseSpecific(name);
  location collapsed(x, y);
  Propogate(collapsed);
}

// Used to make mazes using WFC
//  ONLY WORKS WITH BUILDING BLOCKS THAT HAVE NAMES LIKE WHAT IS PASSED IN
//  ITERATESPECIFIC
void WFC::MakeMaze() {
  int in = rand() % width;
  int out = rand() % width;

  // setting in and out
  IterateSpecific(out, 0, "vertPipe");
  IterateSpecific(in, height - 1, "vertPipe");

  // border off T/B except for
  for (int i = 0; i < height; i++) {
    if (i != in) IterateSpecific(i, height - 1, "Blank");
    if (i != out) IterateSpecific(i, 0, "Blank");
  }

  // border off L/R
  for (int i = 1; i < width - 1; i++) {
    IterateSpecific(0, i, "Blank");
    IterateSpecific(height - 1, i, "Blank");
  }
}

// Goes tile by tile to check it's immediate neighbors to make sure connections
// are valid
bool WFC::validate() {
  // Loops to work through entire tilemap
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < height; j++) {
      location cur(i, j);
      vector<Neighbor> neighbors = get_neighbors(cur, height, width);
      // compare current tile to its neighbors for valid connections
      for (int k = 0; k < neighbors.size(); k++) {
        // get the indexes of "connections" for cur tile and its neighbor
        DIRECTION neighbor_dir = neighbors[k].dir;
        DIRECTION cur_dir = reverse(neighbor_dir);

        // first element is the connection of the cur tile while the 2nd is the
        // connection of its touching neighbor
        if (tilemap[i][j].superPositions[0].connections[cur_dir] !=
            tilemap[neighbors[k].loc.x][neighbors[k].loc.y]
                .superPositions[0]
                .connections[neighbor_dir]) {
          return false;
        }
      }
    }
  }
  return true;
}

// Converting pixel data of building blocks to pgm format for visualization
PGMDAT WFC::Tilemap_To_Pgmdat() {
  PGMDAT dat;
  dat.rows = tilemap.size() * 3;
  dat.cols = tilemap[0].size() * 3;
  dat.pixelvector =
      vector<vector<uint8_t>>(dat.rows, vector<uint8_t>(dat.cols, 0));

  for (int t_row = 0; t_row < height; t_row++) {
    for (int t_col = 0; t_col < width; t_col++) {
      vector<uint8_t> bitmap = tilemap[t_row][t_col].superPositions[0].bitmap;
      for (int pixdat = 0; pixdat < (int)bitmap.size(); pixdat++) {
        dat.pixelvector[t_row * 3 + (int)floor(pixdat / 3)]
                       [t_col * 3 + (pixdat % 3)] = bitmap[pixdat];
      }
    }
  }

  return dat;
}
