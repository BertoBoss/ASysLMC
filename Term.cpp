#include "Term.h"

Term::Term(){
  name = "";
  arity = "";
  parent = NULL;
}


void Term::set_name(string n){
  name = n;
}
  
string Term::get_name(){
  return name;
}

void Term::set_arity(string a){
  arity = a;
}
  

string Term::get_arity(){
  return arity;
}

void Term::add_arg(string arg){
  args.push_back(arg);
}

vector<string> Term::get_args(){
  return args;
}

void Term::set_parent(Term *p){
  parent= p;
}

Term* Term::get_parent(){
  return parent;
}


string Term::to_string(){
  string s = name + "(";
  for(int i = 0; i<args.size(); i++){
    if(i!=args.size()-1)
      s += args.at(i) + ",";
    else
      s += args.at(i) + ")";
  }
  return s;
}
