/*
Jeff Parker
CS4000 HW4
This program altered the original pandemic program by Dr. Juedes which
calculates the longest common substring between 2 strings. This was altered
to include a pipelined performance boost of around ~3.6

This program used omp & c++ threads, compile with:
  g++ parker_pandemic.cc -pthread -fopenmp
*/

//
// Pandemic.cc
//
// Author: David W. Juedes
// Purpose: Compares two DNA sequences using a dynamic programming
// algorithm.    Finds the longest identical subsequence between two
// sequences
//

#include <algorithm>
#include <cassert>
#include <fstream>
#include <future>
#include <iostream>
#include <omp.h>
#include <sstream>
#include <thread>
#include <vector>

using namespace std;

// Global variables
string LSs;
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
// in parallel fashion using future & promises to halt program operation.
//

string LS(string &DNA1, string &DNA2, int i, int n) {

  int start = max(1, (i * (int)DNA2.length() / n));
  int end = min(((i + 1) * DNA2.length()) / n, DNA2.length());

  for (int l = 1; l <= DNA1.length(); l++) {
    // Waits til previous thread says its ready
    if (i != 0) {
      bool go = ready[i - 1][l].get();
    }
    // performs main computations
    for (int j = start; j <= end; j++) {
      if (DNA1[l - 1] == DNA2[j - 1]) {
        if (LSQ[l - 1][j - 1] + 1 > max(LSQ[l - 1][j], LSQ[l][j - 1])) {
          LSQ[l][j] = LSQ[l - 1][j - 1] + 1;
          from[l][j] = make_pair(l - 1, j - 1);
        } else {
          if (LSQ[l - 1][j] > LSQ[l][j - 1]) {
            LSQ[l][j] = LSQ[l - 1][j];
            from[l][j] = make_pair(l - 1, j);
          } else {
            LSQ[l][j] = LSQ[l][j - 1];
            from[l][j] = make_pair(l, j - 1);
          }
        }
      } else {
        if (LSQ[l - 1][j] > LSQ[l][j - 1]) {
          LSQ[l][j] = LSQ[l - 1][j];
          from[l][j] = make_pair(l - 1, j);
        } else {
          LSQ[l][j] = LSQ[l][j - 1];
          from[l][j] = make_pair(l, j - 1);
        }
      }
    }
    // readys next thread
    if (i < n - 1) {
      ready_p[i][l].set_value(true);
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

  // Sets up tables for dynamic programming solution
  LSQ.resize(DNA1.length() + 1);
  from.resize(DNA1.length() + 1);
#pragma omp parallel for schedule(static, 5000)
  for (int i = 0; i < DNA1.length() + 1; i++) {
    LSQ[i].resize(DNA2.length() + 1, 0);
    from[i].resize(DNA2.length() + 1);
  }
#pragma omp parallel for schedule(static, 5000)
  for (int i = 0; i < DNA2.length() + 1; i++) {
    LSQ[0][i] = 0;
    from[0][i] = make_pair(-1, -1);
  }
#pragma omp parallel for schedule(static, 5000)
  for (int i = 1; i < DNA1.length() + 1; i++) {
    LSQ[i][0] = 0;
    from[i][0] = make_pair(-1, -1);
  }
  // Sets up future & promises for pipeline
  ready.resize(3);
  ready_p.resize(3);
  for (int i = 0; i < 3; i++) {
    ready[i].resize(DNA1.length() + 1);
    ready_p[i].resize(DNA1.length() + 1);
    for (int j = 1; j < DNA1.length() + 1; j++) {
      ready[i][j] = ready_p[i][j].get_future();
    }
  }

  cout << "DNA1 Length = " << DNA1.length() << endl;
  cout << "DNA2 Length = " << DNA2.length() << endl;

  // 4 threads compute main longest common string operations
  thread t1(LS, ref(DNA1), ref(DNA2), 0, 4);
  thread t2(LS, ref(DNA1), ref(DNA2), 1, 4);
  thread t3(LS, ref(DNA1), ref(DNA2), 2, 4);
  thread t4(LS, ref(DNA1), ref(DNA2), 3, 4);

  // Blocks until each thread is finished
  t1.join();
  t2.join();
  t3.join();
  t4.join();

  string LS1;
  int l1 = DNA1.length();
  int l2 = DNA2.length();
  while ((l1 != 0) && (l2 != 0)) {
    pair<int, int> t;
    t = from[l1][l2];
    if ((t.first == l1 - 1) && (t.second == l2 - 1)) {
      assert(DNA1[l1 - 1] == DNA2[l2 - 1]);
      LS1.insert(0, 1, DNA1[l1 - 1]);
    }
    l1 = t.first;
    l2 = t.second;
  }

  LSs += LS1;

  cout << LSs << endl;
  cout << "Similarity score 1 vs 2=" << LSs.length() / (DNA1.length() * 1.0)
       << endl;
  cout << "Similarity score 2 vs 1=" << LSs.length() / (DNA2.length() * 1.0)
       << endl;
}
