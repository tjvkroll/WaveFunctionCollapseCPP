#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

struct PGMDAT {
  int rows;
  int cols;
  vector<vector<uint8_t>> pixelvector;
};

class PGM {
 public:
  static PGMDAT ReadPGM(string pgm_filename) {
    PGMDAT data;
    int vmax;
    ifstream infile(pgm_filename);
    stringstream ss;
    string version, comment;
    getline(infile, version);  // Version Declare
    getline(infile, comment);  // Author comment
    // Access buffer directly view Stringstream.
    ss << infile.rdbuf();
    ss >> data.cols >> data.rows;
    ss >> vmax;
    vector<vector<uint8_t>> pixelvector(data.rows,
                                        vector<uint8_t>(data.cols, 0));
    for (int y = 0; y < data.rows; y++) {
      for (int x = 0; x < data.cols; x++) {
        char val[1];
        ss.read(val, sizeof(uint8_t));
        pixelvector[y][x] = *val;
      }
    }
    infile.close();
    data.pixelvector = pixelvector;
    return data;
  }

  static void WritePGM(string outfile_fn, PGMDAT dat) {
    ofstream outfile(outfile_fn, ios::binary);
    outfile << "P5" << endl
            << "# pgm.h generation" << endl
            << dat.cols << " " << dat.rows << endl
            << "255" << endl;
    for (int y = 0; y < dat.rows; y++) {
      for (int x = 0; x < dat.cols; x++) {
        uint8_t pv = static_cast<uint8_t>(dat.pixelvector[y][x] * 254);
        outfile.write(reinterpret_cast<char *>(&pv), sizeof(pv));
      }
    }
    outfile.close();
  }
};