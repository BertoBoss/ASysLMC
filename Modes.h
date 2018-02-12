#ifndef _MODES_
#define _MODES_

#include <vector>
#include <string>
#include <iostream>

using namespace std;

class Mode{
 private:
  string predicate;
  vector<string> modes;
  char type;
 public:
  Mode();
  Mode(string p,vector<string> m,char type);
  void print();
  string get_predicate();
  void set_predicate(string s);
  vector<string> get_modes();
  void set_modes(vector<string> v);
  char get_type();
  void set_type(char t);
};

#endif
