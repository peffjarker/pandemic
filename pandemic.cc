//
// Pandemic.cc
//
// Author: David W. Juedes
// Purpose: Compares two DNA sequences using a dynamic programming
// algorithm.    Finds the longest identical subsequence between two
// sequences.
//

#include "omp.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <future>
#include <iostream>
#include <numeric>
#include <sstream>
#include <thread>
#include <vector>

using namespace std;

string LS1 = "";
vector<vector<int>> LSQ;
vector<vector<pair<int, int>>> from;
vector<vector<future<bool>>> ready;
vector<vector<promise<bool>>> ready_p;

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

string LS(string &DNA1, string &DNA2, int y1, int y2) {

  for (int i = 1; i < DNA1.length(); i++) {
    for (int j = y1; j < y2; j++) {
      if (i != 1) {
        bool go = ready[i - 1][j].get() && ready[i][j - 1].get() &&
                  ready[i - 1][j - 1].get();
      }
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
      if (j < DNA2.length() - 1) {
        ready_p[i][j].set_value(true);
      }
    }
  }

  cout << "LSQ length = " << LSQ[DNA1.length()][y2] << endl;

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
  // assert(return_it.length() == LSQ[DNA1.length()][DNA2.length()]);

  return return_it;
}

int main(int argc, char *argv[]) {
  cout << argc << endl;
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

  cout << "DNA1 Length = " << DNA1.length() << endl;
  cout << "DNA2 Length = " << DNA2.length() << endl;

  LSQ.resize(DNA1.length() + 1);
  from.resize(DNA1.length() + 1);
  for (int i = 0; i < DNA1.length() + 1; i++) {
    LSQ[i].resize(DNA2.length() + 1, 0);
    from[i].resize(DNA2.length() + 1);
  }
  cout << "resize" << endl;
  for (int i = 0; i < DNA2.length() + 1; i++) {
    LSQ[0][i] = 0;
    from[0][i] = make_pair(-1, -1);
  }
  cout << "first make pair" << endl;
  for (int i = 1; i < DNA1.length() + 1; i++) {
    LSQ[i][0] = 0;
    from[i][0] = make_pair(-1, -1);
  }

  cout << "help" << endl;

  ready.resize(DNA1.length());
  ready_p.resize(DNA1.length());
  for (int i = 0; i < DNA1.length(); ++i) {
    cout << i;
    ready[i].resize(DNA2.length());
    ready_p[i].resize(DNA2.length());
    for (int j = 0; j < DNA2.length(); ++j) {
      if (i == 0 || j == 0) {
        ready_p[i][j].set_value(true);
      } else {
        ready[i][j] = ready_p[i][j].get_future();
      }
    }
  }

  future<string> th1 = async(LS, ref(DNA1), ref(DNA2), 1, DNA2.length() / 4);
  future<string> th2 =
      async(LS, ref(DNA1), ref(DNA2), DNA2.length() / 4 + 1, DNA2.length() / 2);
  future<string> th3 = async(LS, ref(DNA1), ref(DNA2), DNA2.length() / 2 + 1,
                             3 * DNA2.length() / 4);
  future<string> th4 = async(LS, ref(DNA1), ref(DNA2),
                             3 * DNA2.length() / 4 + 1, DNA2.length() + 1);

  LS1 += th1.get();
  LS1 += th2.get();
  LS1 += th3.get();
  LS1 += th4.get();

  cout << LS1 << endl;
  cout << "Similarity score 1 vs 2=" << LS1.length() / (DNA1.length() * 1.0)
       << endl;
  cout << "Similarity score 2 vs 1=" << LS1.length() / (DNA2.length() * 1.0)
       << endl;
}
