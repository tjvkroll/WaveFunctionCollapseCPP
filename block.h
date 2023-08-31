#pragma once

#include <vector>
#include <string>
using namespace std;

enum DIRECTION { UP, DOWN, LEFT, RIGHT };

DIRECTION reverse(DIRECTION dir);


// A single block that exists as a option in super position
class BuildingBlock {
 public:
  BuildingBlock(string name, vector<uint8_t> bitmap, bool* sides, int weight, double density)
      : name(name), bitmap(bitmap), connections(sides), weight(weight), density(density) {}
  string name;
  vector<uint8_t> bitmap;
  bool* connections;
  int weight;
  double density;
};

// All possible options for a tile
class WFCBlock {
 public:
  WFCBlock(vector<BuildingBlock> bb) : superPositions(bb){};
  vector<BuildingBlock> superPositions;
  bool needsUpdate;
  void CollapseSuperPositions();
  void CollapseSpecific(string name);
  void RemoveSuperPositionAtIndex(int index);
  int RandomBasedOnWeights();
};

// #endif