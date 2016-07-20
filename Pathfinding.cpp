#include "Term.h"
#include "Clause.h"
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <map>
#include <algorithm>

using namespace std;

map <string, Term> visited;
map <string, vector<Term>> search_terms; 

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

vector<Term> get_terms(string var, vector<Term> ts){
  vector<Term> aux;
  for(int i = 0; i<ts.size(); i++){
    Term t = ts.at(i);
    vector<string> t_args = t.get_args();
    if((find(t_args.begin(), t_args.end(), var) != t_args.end()) && (visited.find(t.to_string()) == visited.end())){
      aux.push_back(t);
      visited[t.to_string()] = t;
    }
  }
  return aux;
}

void init_search_terms(Clause *bottom){
  Term h = bottom->get_head();
  vector<Term> ts = bottom->get_terms();
  vector<string> h_args = h.get_args();
  for(int i = 0; i<h_args.size(); i++){
    vector<Term> i_terms = get_terms(h_args.at(i),ts);
    search_terms[h_args.at(i)] = i_terms;
  }
}

vector<Term> check_intersection(){
  vector<Term> aux;
  for(auto it = search_terms.begin(); it != search_terms.end(); it++){
    string key = it->first;
    //cout << "KEY:" <<key << endl;
    vector<Term> terms = it->second;
    for(auto it2 = search_terms.begin(); it2!= search_terms.end(); it2++){
      string key2 = it2->first;
      //cout <<"KEY2:" <<key2 << endl;
      vector<Term> terms2 = it2->second;
      if(strcmp(key.c_str(),key2.c_str())!=0){
	//cout << "Diifs " << key << "     " << key2 << endl;
	for(int i = 0; i<terms.size(); i++){
	  Term t = terms.at(i);
	  //cout << "T: " << t.to_string() << endl;
	  vector<string> t_args = t.get_args();
	  for(int j = 0; j<t_args.size(); j++){
	    string var = t_args.at(j);
	    for(int k = 0; k<terms2.size(); k++){
	      Term t2 = terms2.at(k);
	      //cout << "T2: " << t2.to_string() << endl;
	      vector<string> t2_args = t2.get_args();
	      if(find(t2_args.begin(), t2_args.end(), var) != t2_args.end()){
		//cout << "FOUND" << endl;
		if(aux.size()==0){
		  aux.push_back(t);
		}
		if(find(aux.begin(), aux.end(), t) == aux.end()){
		  //cout << "NOT IN AUX " << t.to_string() << endl;
		  aux.push_back(t);
		}
		if(find(aux.begin(), aux.end(), t2) == aux.end()){
		  //cout << "NOT IN AUX " << t2.to_string() << endl;
		  aux.push_back(t2);
		}
	      }
	    }
	  }
	}
	if(aux.size()>0)
	  return aux;
      }
    }
  }
  return aux;
}

int main(int argc, char* argv[]){
  
  //start of parsing
  string file = parse_params(argc, argv);

  string b_text = open_file(file);

  if(b_text.compare("")==0){
    cout << "File is empty. Aborting exec.\n";
    exit(1);
  }

  vector<string> tokens = parse_bottom_clause(b_text);

  Clause * bottom = build_clause (tokens);
  
  cout << "Parsing completed.\nThe result is: ";
  
  cout << bottom->to_string() << endl;

  //start of search

  init_search_terms(bottom);

  vector<Term> intersect = check_intersection();

  //cout << intersect.size() << endl;

  if(intersect.size() > 0){

    cout << "Rule found in step 0.\nPrinting rule... \n";

    //printing prolog like missing...
    for(int i = 0; i<intersect.size(); i++)
      cout << intersect.at(i).to_string() << endl;
  }
  //bidirected search

  
  
  return 0;
}
