#include "Clause.h"
#include "Term.h"

using namespace std;

Clause::Clause (Term h){
  head = h;
}

void Clause::set_head(Term h){
  head = h;
}

Term Clause::get_head(){
  return head;
}

void Clause::add_term(Term t){
  body.push_back(t);
}

vector<Term> Clause:: get_terms(){
  return body;
}

string Clause::to_string(){
  string s = "";
  s += head.to_string() + ":- ";
  
  Term tmp;
  
  for(int i = 0; i< body.size(); i++){
    if(i!=body.size()-1){
      tmp = body.at(i);
      s+= tmp.to_string() + ",";
    }
    else{
      tmp = body.at(i);
      s+= tmp.to_string() + ".";
    }
  }

  return s;
}
