//
// Pandemic.cc
//
// Author: David W. Juedes
// Purpose: Compares two DNA sequences using a dynamic programming
// algorithm.    Finds the longest identical subsequence between two
// sequences.
//

#include <iostream>

#include "omp.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

vector<vector<int>> LSQ;
vector<vector<pair<int, int>>> from;

string read_string(istream &in) {
  string temp;
  string line;
  while (!in.eof()) {
    getline(in, line);
    if (!in.fail()) {
      istringstream in1(line);
      int l;
      in1 >> l;
      // cout << l << endl;
      while (!in1.eof()) {
        string t;
        in1 >> t;
        if (!in1.fail()) {
          temp += t;
        }
      }
    }
  }
  return temp;
}

// Compute the longest identical subsequence between DNA1 and DNA2
//
//

void LS(string &DNA1, string &DNA2, int x1, int x2) {

  int start = x1;
  int end = x2;

  for (int i = 1; i <= DNA1.length(); i++) {
    for (int j = start; j <= end; j++) {
      if (DNA1[i - 1] == DNA2[j - 1]) {
        if (LSQ[i - 1][j - 1] + 1 > max(LSQ[i - 1][j], LSQ[i][j - 1])) {
          LSQ[i][j] = LSQ[i - 1][j - 1] + 1;
          from[i][j] = make_pair(i - 1, j - 1);
        } else {
          if (LSQ[i - 1][j] > LSQ[i][j - 1]) {
            LSQ[i][j] = LSQ[i - 1][j];
            from[i][j] = make_pair(i - 1, j);
          } else {
            LSQ[i][j] = LSQ[i][j - 1];
            from[i][j] = make_pair(i, j - 1);
          }
        }
      } else {
        if (LSQ[i - 1][j] > LSQ[i][j - 1]) {
          LSQ[i][j] = LSQ[i - 1][j];
          from[i][j] = make_pair(i - 1, j);
        } else {
          LSQ[i][j] = LSQ[i][j - 1];
          from[i][j] = make_pair(i, j - 1);
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  assert(argc == 3); // Fail if this isn't true.

  ifstream in1;
  in1.open(argv[1]);
  if (in1.fail()) {
    cout << "Couldn't open " << argv[1] << endl;
    exit(-1);
  }
  ifstream in2;
  in2.open(argv[2]);
  if (in2.fail()) {
    cout << "Couldn't open " << argv[2] << endl;
    exit(-1);
  }

  string DNA1;
  DNA1 = read_string(in1);
  // cout << DNA1 << endl;
  string DNA2;
  DNA2 = read_string(in2);
  // cout << DNA2 << endl;

  LSQ.resize(DNA1.length() + 1);
  from.resize(DNA1.length() + 1);
  for (int i = 0; i < DNA1.length() + 1; i++) {
    LSQ[i].resize(DNA2.length() + 1, 0);
    from[i].resize(DNA2.length() + 1);
  }
  for (int i = 0; i < DNA2.length() + 1; i++) {
    LSQ[0][i] = 0;
    from[0][i] = make_pair(-1, -1);
  }
  for (int i = 1; i < DNA1.length() + 1; i++) {
    LSQ[i][0] = 0;
    from[i][0] = make_pair(-1, -1);
  }

  cout << "DNA1 Length = " << DNA1.length() << endl;
  cout << "DNA2 Length = " << DNA2.length() << endl;

  LS(DNA1, DNA2, 1, DNA2.length() / 4);
  LS(DNA1, DNA2, DNA2.length() / 4 + 1, 2 * DNA2.length() / 4);
  LS(DNA1, DNA2, 2 * DNA2.length() / 4 + 1, 3 * DNA2.length() / 4);
  LS(DNA1, DNA2, 3 * DNA2.length() / 4 + 1, DNA2.length());

  string return_it;
  // Construct the LIS.
  int l1 = DNA1.length();
  int l2 = DNA2.length();
  while ((l1 != 0) && (l2 != 0)) {
    pair<int, int> t;
    t = from[l1][l2];
    if ((t.first == l1 - 1) && (t.second == l2 - 1)) {
      assert(DNA1[l1 - 1] == DNA2[l2 - 1]);
      return_it.insert(0, 1, DNA1[l1 - 1]);
    }
    l1 = t.first;
    l2 = t.second;
  }

  assert(return_it.length() == LSQ[DNA1.length()][DNA2.length()]);

  cout << return_it << endl;
  cout << "Similarity score 1 vs 2="
       << return_it.length() / (DNA1.length() * 1.0) << endl;
  cout << "Similarity score 2 vs 1="
       << return_it.length() / (DNA2.length() * 1.0) << endl;
}
