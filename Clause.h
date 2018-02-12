#ifndef _CLAUSE_
#define _CLAUSE_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "Term.h"
#include "Node.h"

using namespace std;

class Clause{
 private:
  Term head;
  vector<Term> body;
  vector<Node*> to_exp;
  vector<Node*> raw;
  vector<string> invars;
  vector<string> outvars;
  vector<Node*> candidates;
  vector<string> covers;
  vector<string> coversn;
  
 public:
  Clause();
  Clause (Term head);
  void set_head(Term h);
  Term get_head();
  void add_term(Term t);
  vector<Term> get_terms();
  string to_string();
  string body_to_string();
  vector<Node*> get_raw();
  void add_to_raw(Node* n);
  void add_to_invars(string s);
  vector<string> get_invars();
  void add_to_outvars(string s);
  vector<string> get_outvars();
  void add_to_candidates(Node* n);
  vector<Node*> get_candidates();
  vector<string> get_covers();
  void set_covers(vector<string> c);
  vector<string> get_coversn();
  void set_coversn(vector<string> c);
  
  bool operator == (Clause t);

};

#endif
