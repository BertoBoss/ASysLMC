#include "Term.h"
#include <iostream>
#include <stdio.h>

int main(){
  Term t;
  t.set_name("oi");
  t.set_arity("2");
  t.add_arg("arg1");
  t.add_arg("arg2");
  Term p;
  p.set_name("oip");
  t.set_parent(&p);
  cout << t.get_name() << " " << t.get_arity() << " " << "\n";
  vector <string> args = t.get_args();
  for(int i = 0; i< args.size(); i++)
    cout << args.at(i) << "\n";
  Term q = *t.get_parent();
  cout << q.get_name() << "\n";
  return 0;
}
