#include <iostream>
#include <stdio.h>
#include "Term.h"
#include "Clause.h"

int main(){
  Term t;
  t.set_name("oi");
  t.set_arity("2");
  t.add_arg("X");
  t.add_arg("Y");
  cout << t.to_string() << "\n"; //construct 'oi(X,Y)'
  Term q;
  q.set_name("ola");
  q.set_arity("1");
  q.add_arg("X");
  cout << q.to_string() << "\n"; // construct 'ola(X)'
  Term k;
  k.set_name("adeus");
  k.set_arity("1");
  k.add_arg("Y");
  cout << k.to_string() << "\n"; // construct 'adeus(Y)'
  Clause *c = new Clause (t);
  c->set_head(t);
  c->add_term(q);
  c->add_term(k);
  cout << c->to_string() << "\n"; // construct 'oi(X,Y):- ola(X),adeus(Y).'
  return 0;
}
