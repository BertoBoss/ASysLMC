#include "Term.h"
#include "Clause.h"
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

//treats parameters
string parse_params(int argc, char* argv[]){
  string file="";
  
  if(argc < 2){
    file = "bottom";
  }
  
  else{
    for(int i = 1; i< argc; i++){
      if(strcmp(argv[i],"-f") == 0)
	file = argv[i+1];
    }
    if(file.compare("")==0)
      file = "bottom";
  }
  return file;
}

string open_file(string file){
  string line;
  ifstream clause;
  clause.open(file.c_str());
  if(clause.is_open()){
    getline(clause,line);
  }
  else
    cout << "Unable to open file. \n";
  
  return line;
}

vector<string> parse_bottom_clause(string bottom){
  string delimiter = " ";
  size_t pos = 0;
  string token;
  vector <string> tokens;
  while((pos = bottom.find(delimiter)) != string::npos){
    token = bottom.substr(0,pos);
    //cout << token << endl;
    tokens.push_back(token);
    bottom.erase(0, pos + delimiter.length());
  }

  tokens.push_back(bottom);

  return tokens;
}
Term* construct_head(string head_txt){
  string h_name="";
  string h_var = "";
  int var = 0;
  Term* head = new Term();
  for(string::size_type i = 0; i<head_txt.size(); i++){
    if(!var){
      if(head_txt[i]!='(')
	h_name += head_txt[i];
      else{
	head->set_name(h_name);
	var = 1;
      }
    }
    else{
      if(head_txt[i] == ')' || head_txt[i] == ','){
	head -> add_arg(h_var);
	h_var="";
      }
      else{
	h_var+=head_txt[i];
      }
    }
  }

  string s = to_string(head->get_args().size());
  
  head->set_arity(s);

  return head;
 
}

Term* construct_term(string t){
  t = t.substr(0,t.length()-1);
  int var = 0;
  string t_name="";
  string t_var="";
  Term * term = new Term();
  for(string :: size_type i = 0; i<t.size();i++){
    if(!var){
      if(t[i]!='(')
	t_name += t[i];
      else{
	term->set_name(t_name);
	var = 1;
      }
    }
    else{
      if(t[i]==',' || t[i] == ')'){
	term->add_arg(t_var);
	t_var="";
      }
      else{
	t_var+=t[i];
      }
    }
  }

  string s = to_string(term->get_args().size());
  
  term->set_arity(s);

  return term;
  
}

Clause* build_clause(vector<string> tokens){
  string head_txt = tokens.front();

  Term* head = construct_head(head_txt);
  
  //cout << head->to_string()<<endl;

  Clause* bottom = new Clause(*head);
  for(int i = 2; i< tokens.size(); i++){
    Term* term = construct_term(tokens.at(i));
    //cout << "Term" << i << ": " <<term -> to_string() << endl;
    bottom->add_term(*term);
  }

  //  cout << bottom->to_string() << endl;

  return bottom;
}

int main(int argc, char* argv[]){
  string file = parse_params(argc, argv);

  string b_text = open_file(file);

  if(b_text.compare("")==0){
    cout << "File is empty. Aborting exec. \n";
    exit(1);
  }

  vector<string> tokens = parse_bottom_clause(b_text);

  Clause * bottom = build_clause (tokens);
  
  cout << "Parsing fase completed.\nThe result is: ";
  
  cout << bottom->to_string() << endl;

  return 0;
}
