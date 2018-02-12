#include "Clause.h"
#include "Term.h"
#include "Node.h"

Clause::Clause(){
}

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

string Clause::body_to_string(){
  string s = "";
  
  Term tmp;
  
  for(int i = 0; i< body.size(); i++){
    if(i!=body.size()-1){
      tmp = body.at(i);
      s+= tmp.to_string() + ",";
    }
    else{
      tmp = body.at(i);
      s+= tmp.to_string();
    }
  }

  return s;
}

void Clause::add_to_raw(Node* n){
  raw.push_back(n);
  return;
}

vector<Node*> Clause::get_raw(){
  return raw;
}

void Clause::add_to_invars(string s){
  invars.push_back(s);
}

vector<string> Clause::get_invars(){
  return invars;
}

void Clause::add_to_outvars(string s){
  outvars.push_back(s);
}

vector<string> Clause::get_outvars(){
  return outvars;
}

void Clause::add_to_candidates(Node* n){
  candidates.push_back(n);
}

vector<Node*> Clause::get_candidates(){
  return candidates;
}

vector<string> Clause::get_covers(){
  return covers;
}

void Clause::set_covers(vector<string> c){
  covers = c;
}

vector<string> Clause::get_coversn(){
  return coversn;
}

void Clause::set_coversn(vector<string> c){
  coversn = c;
}

bool Clause::operator == (Clause t){
  string s = t.to_string();
  if(strcmp(this->to_string().c_str(), s.c_str())==0)
    return true;
  return false;
}
