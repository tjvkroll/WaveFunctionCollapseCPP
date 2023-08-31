#include "block.h"
#include <random>
#include <time.h>
#include <cassert>


int WFCBlock::RandomBasedOnWeights(){

  // Approach implemented from: https://www.keithschwarz.com/darts-dice-coins/
  int sum_weights = 0; 
  for (uint i = 0; i < superPositions.size(); i++) {
    sum_weights += superPositions[i].weight;
  }
  vector<int> probs(sum_weights); 
  for (uint i = 0; i < superPositions.size(); i++) {
    for(int j = 0; j < superPositions[i].weight; j++ ){
      probs.push_back(i);
    }
  }
  int random_idx = rand() % probs.size();
  return probs[random_idx];
}


// Choosing a BUILDING BLOCK OUT OF REMAINING OPTIONS
void WFCBlock::CollapseSuperPositions() {
  int rnd = RandomBasedOnWeights();
  BuildingBlock tmp = superPositions[rnd];
  superPositions.clear();
  superPositions.push_back(tmp); 
}

// Allowing for collapsing of specific tiles so we can force images
void WFCBlock::CollapseSpecific(string name){
  for(auto index: superPositions){
    if(index.name == name){
      BuildingBlock tmp = index; 
      superPositions.clear();
      superPositions.push_back(tmp); 
    }
  }

}


// removing BUILDING BLOCKS 
void WFCBlock::RemoveSuperPositionAtIndex(int index) { superPositions.erase(superPositions.begin() + index); }


// returns the mirror of any given direction, used in find all neighbors
DIRECTION reverse(DIRECTION dir) {
  switch (dir) {
    case UP:
      return DOWN;
      break;
    case DOWN:
      return UP;
      break;
    case LEFT:
      return RIGHT;
      break;
    case RIGHT:
      return LEFT;
      break;
    default:
      assert(false);
  }
}
