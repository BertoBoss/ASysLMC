#ifndef _TERM_
#define _TERM_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

class Term{
private:
  string name;
  string arity;
  vector<string> args;
  Term *parent;
public:

  Term();

  void set_name(string n);
  string get_name();
  void set_arity(string n);
  string get_arity();
  void add_arg(string arg);
  vector<string> get_args();
  void set_parent(Term *p);
  Term* get_parent();
  string to_string();
};

#endif
