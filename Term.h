#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

class Term{
private:
  string name;
  string arity;
  vector<string> args;
  Term *parent;
public:

  Term(){
    name = "";
    arity = "";
    parent = NULL;
  }
  
  void set_name(string n){
    name = n;
  }
  
  string get_name(){
    return name;
  }

  void set_arity(string a){
    arity = a;
  }

  string get_arity(){
    return arity;
  }

  void add_arg(string arg){
    args.push_back(arg);
  }

  vector<string> get_args(){
    return args;
  }

  void set_parent(Term *p){
    parent= p;
  }

  Term* get_parent(){
    return parent;
  }

  string to_string(){
    string s = name + "(";
    for(int i = 0; i<args.size(); i++){
      if(i!=args.size()-1)
	s += args.at(i) + ",";
      else
	s += args.at(i) + ")";
    }
    return s;
  }
};
