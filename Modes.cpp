#include "Modes.h"

Mode::Mode(){
  predicate="";
}

Mode::Mode(string p,vector<string> m,char t){
  predicate = p;
  modes = m;
  type = t;
}

void Mode::print(){
  cout << "mode" << type << " : " << predicate << "(";
  for(int i = 0; i<modes.size()-1; i++)
    cout << modes.at(i) << ",";
  cout << modes.at(modes.size()-1) << ")" << endl;
}

void Mode:: set_predicate(string s){
  predicate = s;
}

string Mode::get_predicate(){
  return predicate;
}

void Mode::set_modes(vector<string> v){
  modes = v;
}

vector<string> Mode::get_modes(){
  return modes;
}

void Mode::set_type(char t){
  type = t;
}

char Mode::get_type(){
  return type;
}
