#ifndef _CLAUSE_
#define _CLAUSE_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "Term.h"

using namespace std;

class Clause{
 private:
  Term head;
  vector<Term> body;
  
 public:
  Clause (Term head);
  void set_head(Term h);
  Term get_head();
  void add_term(Term t);
  vector<Term> get_terms();
  string to_string();
};

#endif
