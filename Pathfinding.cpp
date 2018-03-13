#include "Term.h"
#include "Clause.h"
#include "Modes.h"
#include "Node.h"
#include "Hypergraph.h"
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <map>
#include <algorithm>
#include <SWI-cpp.h>
#include <time.h>
#include <unistd.h>
#include <set>
#include <limits.h>

using namespace std;

//Function declarations

vector<string> parse_bottom_clause(string bottom);
Clause build_clause(vector<string> tokens);
string add_spaces(string b);
Term construct_head(string head_txt);
Term construct_term(string t);
vector<Mode> parse_modes(vector<string> modes);
void generateHG(Hypergraph* hg,vector<Mode*> m);
vector<int> get_io_pos(int flag, Term t, vector<Mode*> modes);
void print_hg_bfs();
void print_hg_dfs();
vector<Node*> search_dfs(vector<string> vars);
bool coverage_threshold(vector<Node*> v);
vector<Node*> rearrange(vector<Node*> v);
bool unifies_with(Clause c1, Clause c2);
vector<Node*> merge(vector<Node*> v1, vector<Node*> v2);
int coverage(vector<Node*> v);
bool io_consistent(vector<Node*> v);
vector<Node*> get_not_expanded(vector<Node*> v);
/*

Compile: 

swipl-ld -cc-options,-std=c++11  Pathfinding.cpp Term.cpp Clause.cpp Modes.cpp Node.cpp Hypergraph.cpp

run:

./a.out -f $file_name [-t $threshold -h $heuristic]

1. Saturate a positive example
2. Ignoring the modes, generate all "good" combinations of literals, where at least one variable appears in the head (see example below)
3. "Score" each of the generated rules according to the "paths" between variables. Each rule will be evaluated according to the path length of the variables in the bottom clause AND according to its score on pos and negs (não sei ainda como definir esta métrica, mas pode-se tentar algumas combinações)
3. In the next steps of refinement choose one of (now obeying the modes):
     3.1 add a new literal to the "best rule" so far
4. Repeat step (3)

*/

//Global variables
Clause bottom;
Clause b_c;
vector<Mode> mode_decs;
vector<Mode*> m_c;
Hypergraph* hyperg;
map <string,Node*> nodes_created;
vector<vector<vector<Node*>>> hyperpaths;
vector<vector<Node*>> paths;
vector<Clause> pathsc;
map<string,vector<string>> variable_sets;
map<string,vector<string>> expanded;
double threshold = 0;
string heuristic;
int positives=0;
int negatives=0;
int pos_number=0;
vector<Clause> candidates;
vector<vector<Node*>> possible_combos;
//vector<vector<Node*>> max_clauses;
int max_cov=0;
int max_pos = 0;
int max_neg = 0;
int gen_c = 0;
int max_score = INT_MIN;
vector<Clause> max_clauses;
int cases = 0;
double coverage_total_time = 0;
double pathlength_total_time = 0;
vector<string> positives_f;
vector<int> positive_index;


vector<string> parse_bottom_clause(string bottom){
  string delimiter = " ";
  size_t pos = 0;
  string token;
  vector <string> tokens;
  while((pos = bottom.find(delimiter)) != string::npos){
    token = bottom.substr(0,pos);
    tokens.push_back(token);
    bottom.erase(0, pos + delimiter.length());
  }

  tokens.push_back(bottom);

  return tokens;
}
Term construct_head(string head_txt){
  string h_name="";
  string h_var = "";
  int var = 0;
  Term head;
  for(string::size_type i = 0; i<head_txt.size(); i++){
    if(!var){
      if(head_txt[i]!='(')
	h_name += head_txt[i];
      else{
	head.set_name(h_name);
	var = 1;
      }
    }
    else{
      if(head_txt[i] == ')' || head_txt[i] == ','){
	head.add_arg(h_var);
	h_var="";
      }
      else{
	h_var+=head_txt[i];
      }
    }
  }

  string s = to_string(head.get_args().size());
  
  head.set_arity(s);

  return head;
 
}

Term construct_term(string t){
  t = t.substr(0,t.length()-1);
  int var = 0;
  string t_name="";
  string t_var="";
  Term term;
  for(string :: size_type i = 0; i<t.size();i++){
    if(!var){
      if(t[i]!='(')
	t_name += t[i];
      else{
	term.set_name(t_name);
	var = 1;
      }
    }
    else{
      if(t[i]==',' || t[i] == ')'){
	term.add_arg(t_var);
	t_var="";
      }
      else{
	t_var+=t[i];
      }
    }
  }

  string s = to_string(term.get_args().size());
  
  term.set_arity(s);

  return term;
  
}

Clause build_clause(vector<string> tokens){
  string head_txt = tokens.front();

  Term head = construct_head(head_txt);

  Clause bottom (head);
  for(int i = 2; i< tokens.size(); i++){
    Term term = construct_term(tokens.at(i));
    bottom.add_term(term);
  }
  return bottom;
}

string add_spaces(string b){
  vector<char> b2;
  string::size_type k;
  for(string::size_type i = 0; i<b.size(); i++){
    char a = b[i];
    if(a==')'){
      b2.push_back(a);
      b2.push_back(' ');
      b2.push_back(':');
      b2.push_back('-');
      b2.push_back(' ');
      k=i+3;
      break;
    }
    else b2.push_back(a);
  }

  for(string::size_type i=k; i<b.size(); i++){
    char a = b[i];
    if(a == ')' && b[i+1] == ','){
      b2.push_back(')');
      b2.push_back(',');
      b2.push_back(' ');
      i++;
    }
    else b2.push_back(a);
  }

  b2.push_back('.');
  string s = string(b2.begin(), b2.end());
  return s;
}

vector<Mode> parse_modes(vector<string> modes){
  vector<Mode> ret;
  for(int i = 0; i < modes.size(); i++){
    string mode = modes.at(i);
    while(mode.at(0)!='m')
      mode = mode.substr(1);
    
    char type = mode.at(4);

    while(mode.at(0)!=',')
      mode = mode.substr(1);
    
    mode = mode.substr(1);

    string pred = "";
    int j = 0;
    while(mode.at(j)!='('){
      pred = pred + mode.at(j++);
    }
    
    mode = mode.substr(j+1);

    vector<string> mode_ds;

    j=0;
    string m="";
    
    while(mode.at(j)!=')'){
      if(mode.at(j)==','){
	mode_ds.push_back(m);
	m="";
      }
      else{
	m = m + mode.at(j);
      }
      j++;
    }
    mode_ds.push_back(m);
    //Mode aux = new Mode(pred,mode_ds,type);
    Mode aux;
    aux.set_predicate(pred);
    aux.set_modes(mode_ds);
    aux.set_type(type);
    ret.push_back(aux);
  }
  return ret;
}

vector<int> get_io_pos(int flag, Term t){
  vector<int> to_return;
  if(flag==0){ //output vars
    string pred = t.get_name();
    for(int i = 0; i<mode_decs.size(); i++){
      string mode_pred = mode_decs.at(i).get_predicate();
      char mode_type = mode_decs.at(i).get_type();
      if((strcmp(pred.c_str(),mode_pred.c_str())==0) && (mode_decs[i].get_modes().size() == t.get_args().size())){
	vector<string> mode_declarations = mode_decs.at(i).get_modes();
	for(int i = 0; i<mode_declarations.size(); i++){
	  if(mode_declarations.at(i).at(0)=='-' || mode_declarations.at(i).at(0)=='#'){
	    if(find(to_return.begin(),to_return.end(),i)==to_return.end())
	      to_return.push_back(i);
	  }
	}
      }
    }
  }
  else{ //input vars
    string pred = t.get_name();
    for(int i = 0; i<mode_decs.size(); i++){
      string mode_pred = mode_decs.at(i).get_predicate();
      char mode_type = mode_decs.at(i).get_type();
      if((strcmp(pred.c_str(),mode_pred.c_str())==0) && (mode_decs[i].get_modes().size() == t.get_args().size())){
	vector<string> mode_declarations = mode_decs.at(i).get_modes();
	for(int i = 0; i<mode_declarations.size(); i++){
	  if(mode_declarations.at(i).at(0)=='+'){
	    to_return.push_back(i);
	  }
	}
      }
    }
  }
  return to_return;
}

vector<string> get_io_vars(vector<int> pos,vector<string> vars){
  vector<string> return_vars;
  for(int i = 0; i<pos.size(); i++){
    return_vars.push_back(vars[pos[i]]);
  }
  return return_vars;
}

vector<Term> get_io_terms(vector<string> vars, int flag){
  vector<Term> terms;
  if(flag==1){ //check if terms in bottom clause contain vars as input variables
    vector<Term> bottom_terms = bottom.get_terms();
    for(int i = 0; i<bottom_terms.size(); i++){
      Term t  = bottom_terms.at(i);
      vector<int> input_pos = get_io_pos(1,t);
      vector<string> input_vars = get_io_vars(input_pos,t.get_args());
      bool contains = false;
      for(int j = 0; j<vars.size(); j++){
	for(int k = 0; k<input_vars.size(); k++){
	  if(strcmp(vars.at(j).c_str(),input_vars.at(k).c_str())==0){
	    contains = true;
	  }
	}
      }
      if(contains)
	terms.push_back(t);
    }
  }
  else{
    vector<Term> bottom_terms = bottom.get_terms();
    for(int i = 0; i<bottom_terms.size(); i++){
      Term t  = bottom_terms.at(i);
      vector<int> output_pos = get_io_pos(0,t);
      vector<string> output_vars = get_io_vars(output_pos,t.get_args());
      bool contains = false;
      for(int j = 0; j<vars.size(); j++){
	for(int k = 0; k<output_vars.size(); k++){
	  if(strcmp(vars.at(j).c_str(),output_vars.at(k).c_str())==0){
	    contains = true;
	  }
	}
      }
      if(contains)
	terms.push_back(t);
    }
  }
  return terms;
}

void generateHG(){
  Node* root = hyperg->get_root();
  Term t = root -> get_term();
  //cout << t.to_string() << endl;
  vector<Node*> to_visit;
  vector<int> input_pos = get_io_pos(1,t);
  //for(int i = 0; i<input_pos.size(); i++)
  //cout << input_pos.at(i)<<endl;
  vector<string> input_vars = get_io_vars(input_pos,t.get_args());
  vector<Term> init_terms = get_io_terms(input_vars,1);
  /*for(int i = 0; i<init_terms.size(); i++)
    cout << init_terms[i].to_string() << " ";
  cout << endl;
  cout << "got init terms" << endl;*/
  for(int i = 0; i<init_terms.size();i++){
    Node* aux = new Node(init_terms.at(i));
    aux->add_parent(root);
    root->add_child(aux);
    nodes_created[aux->get_term().to_string()] = aux;
    hyperg->count();
    to_visit.push_back(aux);
  }
  nodes_created[root->get_term().to_string()] = root;
  hyperg->count();
  //cout << "1st level" << endl;
  
  while(to_visit.size()>0){
    Node* aux = to_visit.front();
    to_visit.erase(to_visit.begin());
    //cout << aux->get_term().to_string() << endl;
    vector<int> output_pos = get_io_pos(0,aux->get_term());
    //cout << "got output pos" << endl; 
    vector<string> output_vars = get_io_vars(output_pos,aux->get_term().get_args());
    //cout << "got output vars" << endl;
    vector<Term> child_terms = get_io_terms(output_vars,1);
    //cout << "got output terms" << endl;
    for(int i = 0; i<child_terms.size(); i++){
      Term c = child_terms.at(i);
      if(nodes_created.find(c.to_string())==nodes_created.end()){
	//cout << "new node" << endl;
	hyperg->count();
	Node* nnode = new Node(c);
	nnode->add_parent(aux);
	aux->add_child(nnode);
	nodes_created[nnode->get_term().to_string()] = nnode;
	to_visit.push_back(nnode);
      }
      else{
	Node* nnode = nodes_created[c.to_string()];
	nnode->add_parent(aux);
	aux->add_child(nnode);
	nodes_created[nnode->get_term().to_string()] = nnode;
      }
    }
    nodes_created[aux->get_term().to_string()] = aux;
  } 
}

void print_hg_bfs(){
  cout << "BFS VISIT" << endl;
  vector<Node*> to_print;
  vector<string> visited;
  to_print.push_back(hyperg->get_root());
  while(to_print.size()>0){
    Node* tmp = to_print.front();
    to_print.erase(to_print.begin());
    Term aux = tmp->get_term();
    if(find(visited.begin(),visited.end(),aux.to_string())==visited.end()){
      cout << aux.to_string() << endl;
      vector<Node*> childs = tmp->get_children();
      for(int i = 0; i<childs.size(); i++)
	to_print.push_back(childs.at(i));
      visited.push_back(aux.to_string());
    }
  }
  cout << "END OF BFS VISIT" << endl;
}

void print_hg_dfs(){
  cout << "DFS VISIT" << endl;
  vector<Node*> to_print;
  vector<string> visited;
  to_print.push_back(hyperg->get_root());
  while(to_print.size()>0){
    Node* tmp = to_print.front();
    to_print.erase(to_print.begin());
    Term aux = tmp->get_term();
    if(find(visited.begin(),visited.end(),aux.to_string())==visited.end()){
      vector<Node*> childs = tmp->get_children();
      cout << aux.to_string() << endl;
      for(int i = 0; i<childs.size(); i++)
	to_print.insert(to_print.begin(),childs.at(i));
      visited.push_back(aux.to_string());
    }
  }
  cout << "END OF DFS VISIT" << endl;
}

vector<string> covers(Clause c){
  ofstream file;
  file.open("theos.txt", ofstream::out | ofstream::trunc);

  file << c.to_string() << endl;
  
  PlCall("rdhyp('theos.txt').");
  
  PlCall("covers('covers.txt').");
  
  ifstream coverpositive ("covers.txt");
  
  string line;
  vector<string> covered;
  while(getline (coverpositive, line)){
    covered.push_back(line);
  }
  covered.pop_back();
  file.close();
  coverpositive.close();
  return covered;
}


vector<string> coversn(Clause c){
  ofstream file;
  file.open("theosn.txt", ofstream::out | ofstream::trunc);

  file << c.to_string() << endl;
  
  PlCall("rdhyp('theosn.txt').");
   
  PlCall("coversn('coversn.txt').");
  
  ifstream covernegative ("coversn.txt");
  string line;
  vector<string> covered;
  while(getline (covernegative, line)){
    covered.push_back(line);
  }
  covered.pop_back();
  file.close();
  covernegative.close();
  return covered;
}

int coverage(vector<Node*> v){

  v = rearrange(v);
  Clause c (bottom.get_head());
  for(int i = 1; i<v.size(); i++)
    c.add_term(v[i]->get_term());
  
  ofstream file;
  file.open("theos.txt", fstream::out | fstream::trunc);

  file << c.to_string() << endl;
  
  PlCall("rdhyp('theos.txt').");
  
  PlCall("covers('covers.txt').");
  
  ifstream coverpositive ("covers.txt");
  
  //cout << "NUMA DE CURTE: " << endl;
  string line;
  string lastpos;
  while(getline (coverpositive, line)){
    lastpos = line;
  }
  
  PlCall("coversn('coversn.txt').");
  
  ifstream covernegative ("coversn.txt");
  
  //cout << "NUMA DE CURTE: " << endl;
  string lastneg;
  while(getline (covernegative, line)){
    lastneg = line;
  }
  
  //cout << "POS: " << lastpos << endl;
  //cout << "NEG: " << lastneg << endl;
  
  string pos_almost = lastpos.substr(20);
  string neg_almost = lastneg.substr(20);
  //cout << "CORTADO: " << pos_almost << endl;
  
  pos_almost = pos_almost.substr(0,pos_almost.size()-1);
  neg_almost = neg_almost.substr(0,neg_almost.size()-1);
  
  //cout << "SO N: " << pos_almost << endl;
  
  int poss = atoi(pos_almost.c_str());
  int negs = atoi(neg_almost.c_str());
  
  //cout << "POSS: " << poss << endl;
  //cout << "NEGS: " << negs << endl;
  
  int score = poss-negs;
  
  return score;
}

void generate_all_combinations_aux(vector<vector<Node*>> m, int n, vector<Node*> k){
  if(n==m.size()){
    k = rearrange(k);
    paths.push_back(k);
    return;
  }

  //cout << n << endl;
  
  for(int i = 0; i<m[n].size(); i++){
    vector<Node*> a;
    a.push_back(m[n][i]);
    vector<Node*> cont = merge(k,a);
    generate_all_combinations_aux(m,n+1,cont);
  }
}

void generate_all_possibles(vector<vector<Node*>> m, int n, vector<Node*> k){

  /*cout << "EL KAPPO" << endl;
  for(int i = 0; i<k.size(); i++)
    cout << k[i]->get_term().to_string() << " ";
  cout << "FINO" << endl;
  */
  if(n==m.size()){
    k = rearrange(k);
    possible_combos.push_back(k);
    return;
  }
  
  for(int i = 0; i<m[n].size(); i++){
    vector<Node*> a;
    a.push_back(m[n][i]);
    vector<Node*> cont = merge(k,a);
    generate_all_possibles(m,n+1,cont);
  }
}

vector<vector<int>> generate_combo_sets(int size, int n, vector<vector<int>> sets){
  if(size==0)
    return sets;
  if(n==0){
    vector<int> empty;
    sets.push_back(empty);
    empty.push_back(n);
    sets.push_back(empty);
    return generate_combo_sets(size,n+1,sets);
  }
  if(n == size){
    return sets;
  }
  vector<vector<int>> aux;
  for(int i = 0; i<sets.size(); i++){
    vector<int> aux2 = sets[i];
    aux2.push_back(n);
    aux.push_back(aux2);
  }

  for(int i = 0; i<aux.size(); i++){
    sets.push_back(aux[i]);
  }
  return generate_combo_sets(size,n+1,sets);
}

vector<Term> get_all_terms(string var){
  vector<Term> bottom_terms = bottom.get_terms();
  vector<Term> to_ret;
  for(int i = 0; i<bottom_terms.size(); i++){
    vector<string> term_vars = bottom_terms[i].get_args();
    if(find(term_vars.begin(),term_vars.end(),var)!=term_vars.end())
      to_ret.push_back(bottom_terms[i]);
  }
  return to_ret;
}

bool contains_all_vars(vector<Node*> p, vector<string> vars){
  //TODO: ver se p cobre vars.
  bool cov=false;
  for(int i = 0; i<vars.size(); i++){
    string v = vars[i];
    for(int j = 0; j<p.size(); j++){
      Node* n = p[j];
      Term t = n->get_term();
      vector<string> args = t.get_args();
      for(int k = 0; k<args.size(); k++){
	string arg = args[k];
	if(!strcmp(arg.c_str(),v.c_str())){
	  cov = true;
	  break;
	}
      }
      if(cov)
	break;
    }
    if(!cov)
      return false;
    else{
      cov = false;
    }
  }
  return true;
}

bool node_is_good(Node* n, Clause c, vector<Node*> p){
  //TODO: ver se n pertence a c e não a p
  bool in_c=false;
  bool in_p=false;
  Term t2 = n->get_term();
  vector<Term> terms = c.get_terms();

  for(int i = 0; i<terms.size(); i++){
    Term t = terms[i];
    if(t == t2){
      in_c = true;
      break;
    }
  }

  if(find(p.begin(),p.end(),n)!=p.end())
    in_p = true;

  if(in_c && !in_p)
    return true;
  return false;
}

//expandir a partir todas as clausulas possíveis com termos em c tq cubram todas as variaveis em vars
Clause get_possibles(Node* t,vector<string> vars,Clause c){
  vector<vector<Node*>> possibles;
  vector<Node*> start;
  start.push_back(t);
  possibles.push_back(start);
  Term t1;
  t1.set_name("NULL");
  Clause c1(t1);

  vector<Term> c_terms = c.get_terms();

  if(find(c_terms.begin(),c_terms.end(),t->get_term())==c_terms.end())
    return c1;
  
  while(possibles.size()>0){
    vector<Node*> path = possibles.at(0);
    possibles.erase(possibles.begin());
    //cout << "PL: ";
    for(int i = 0; i<path.size(); i++)
      //cout << path[i]->get_term().to_string() << " ";
      //cout << endl;
    if(contains_all_vars(path,vars)){
      //Tratar primeiro
      //cout << "ENCONTREI" << endl;
      path = rearrange(path);
      Clause c(path[0]->get_term());
      for(int i = 1; i<path.size(); i++)
	c.add_term(path[i]->get_term());
      return c;
    }
    //cout << "Passa o primeiro" << endl;
    for(int i = 0; i<path.size(); i++){
      Node* n = path[i];
      vector<Node*> childs = n->get_children();
      for(int j = 0; j<childs.size(); j++){
	Node* child = childs[j];
	vector<Node*> aux;
	if(node_is_good(child,c,path)){
	  aux = path;
	  aux.push_back(child);
	  //cout << "is good:";
	  for(int i = 0; i<aux.size(); i++)
	    //cout << aux[i]->get_term().to_string() << " ";
	    //cout << endl;
	  possibles.push_back(aux);
	}
	if(contains_all_vars(aux,vars)){
	  //cout << "ENCONTREI" << endl;
	  aux = rearrange(aux);
	  Clause c2(aux[0]->get_term());
	  for(int i = 1; i<aux.size(); i++)
	    c2.add_term(aux[i]->get_term());
	  return c2;
	}
      }
    }
  }

  //cout << "retorna null" << endl;
  return c1;
}

int path_length(Clause c){
  Term head = c.get_head();
  Node* root = nodes_created[head.to_string()];
  vector<string> head_vars = head.get_args();
  vector<Node*> first_var_terms = root->get_children();
  vector<Clause> candidates;
  for(int i = 0; i<first_var_terms.size(); i++){
    Clause unions = get_possibles(first_var_terms[i],head_vars,c);
    if(strcmp(unions.get_head().get_name().c_str(),"NULL")!=0){
      vector<Term> the_path = unions.get_terms();
      vector<string> out_vars,in_vars;
      vector<string> head_vars = unions.get_head().get_args();
      for(int j = 0; j<the_path.size(); j++){
	vector<int> in_pos = get_io_pos(1,the_path[j]);
	vector<int> out_pos = get_io_pos(0,the_path[j]);
	vector<string> ini_vars = get_io_vars(in_pos,the_path[j].get_args());
	vector<string> outi_vars = get_io_vars(out_pos,the_path[j].get_args());
	for(int k = 0; k<ini_vars.size(); k++)
	  in_vars.push_back(ini_vars[k]);
	for(int k = 0; k<outi_vars.size(); k++)
	  out_vars.push_back(outi_vars[k]);
      }
      for(int j = 0; j<head_vars.size(); j++)
	out_vars.push_back(head_vars[j]);
      bool flag = false;
      for(int j = 0; j<in_vars.size(); j++){
	if(find(out_vars.begin(),out_vars.end(),in_vars[j])==out_vars.end())
	  flag = true;
      }
      if(!flag)
	candidates.push_back(unions);
    }
  }
  if(!candidates.size() == 0){
    int min = candidates[0].get_terms().size();
    for(int i = 1; i<candidates.size(); i++)
      if(candidates[i].get_terms().size() < min)
	min = candidates[i].get_terms().size();
    return min;
  }
  return 2*positives;
}

void remove_clauses_covered(Clause c){
  vector<int> to_remove;
  vector<string> covered = covers(c);
  //cout << "TAMANHO DO COVERS: " << covered.size() << endl;
  for(int i = 0; i<covered.size(); i++){
    for(int j = 0; j<positives_f.size(); j++){
      string s1  = covered[i];
      string s2 = positives_f[j];
      s1.erase(remove_if(s1.begin(), s1.end(), ::isspace), s1.end());
      s2.erase(remove_if(s2.begin(), s2.end(), ::isspace), s2.end());
      
      //cout << s1 << endl;
      
      //cout << s1 << "," << s2 << "," << endl;
      //cout << s1.size() << " " << s2.size() << endl;
      //cout << strcmp(positives_f[j].c_str(),covered[i].c_str()) << endl;
      //cout << s1.compare(s2) << endl;
      if((s1.compare(s2))==0){
	//cout << "UELELELELELE" << endl;
	to_remove.push_back(j);
	break;
      }
    }
  }

  //cout << "vamos remover" << endl;
  //cout << positive_index.size() << endl;
  vector<string> tmp_pos_f;
  vector<int> tmp_pos_in;
  for(int i = 0; i<positives_f.size(); i++){
    if(find(to_remove.begin(),to_remove.end(),i)==to_remove.end()){
      tmp_pos_f.push_back(positives_f[i]);
      tmp_pos_in.push_back(positive_index[i]);
    }
  }
  positives_f = tmp_pos_f;
  positive_index = tmp_pos_in;
  //cout << positives_f.size() << endl;
  //cout << positive_index.size() << endl;
}

vector<Clause> transform_hypergraph(){
  int temp_max = 0;
  vector<vector<Node*>> max_clauses_tmp;
  Node* root = hyperg -> get_root();
  vector<Node*> children = root->get_children();
  vector<int> pos = get_io_pos(1,root->get_term());
  vector<string> vars = get_io_vars(pos,root->get_term().get_args());
  vector<vector<Node*>> init_terms;
  
  for(int i = 0; i<vars.size(); i++){ //obtain terms with init vars
    vector<Term> terms = get_all_terms(vars[i]);
    vector<Node*> aux;
    for(int j = 0; j<terms.size(); j++){
      expanded[root->get_term().to_string()].push_back(terms[j].to_string());
      aux.push_back(nodes_created[terms[j].to_string()]);
    }
    if(aux.size()>0)
      init_terms.push_back(aux);
  }

  /*cout << init_terms.size() << endl;
  cout << "VAMOS" << endl;
  for(int i = 0; i<init_terms.size(); i++){
    for(int j = 0; j<init_terms[i].size(); j++)
      cout << init_terms[i][j]->get_term().to_string() << " ";
    cout << endl;
  }  
  cout << "ACABOU" << endl;*/
  
  //cout << "INIT TERMS SIZE: " <<  init_terms.size() << endl;
  vector<Node*> empty;
  
  generate_all_combinations_aux(init_terms,0,empty); //generate combinations of init terms and save it in paths
  
  /*cout << "Numa de curte" << endl;
  for(int i = 0; i<paths.size(); i++){
    for(int j = 0; j<paths[i].size(); j++)
      cout << paths[i][j]->get_term().to_string() << " ";
    cout << endl;
  }
  cout << "FIM" << endl;*/
  
  vector<Clause> to_expand;
  
  for(int i = 0; i<paths.size(); i++){
    //transformar em clauses cada um dos caminhos 
    vector<Node*> rearranged = rearrange(paths[i]);
    vector<string> outconst_vars;
    vector<string> in_vars;
    //get out and in vars from rearranged clause
    for(int j = 0; j<rearranged.size(); j++){
      vector<int> outconst_pos = get_io_pos(0,rearranged[j]->get_term());
      vector<int> in_pos = get_io_pos(1,rearranged[j]->get_term());
      vector<string> outconst_temp = get_io_vars(outconst_pos,rearranged[j]->get_term().get_args());
      vector<string> in_temp = get_io_vars(in_pos,rearranged[j]->get_term().get_args());
      for(int k = 0; k<outconst_temp.size(); k++)
	if(find(outconst_vars.begin(),outconst_vars.end(),outconst_temp[k])==outconst_vars.end())
	  outconst_vars.push_back(outconst_temp[k]);
      for(int k = 0; k<in_temp.size(); k++)
	if(find(in_vars.begin(),in_vars.end(),in_temp[k])==in_vars.end())
	  in_vars.push_back(in_temp[k]);
    }
    Clause c_aux(rearranged[0]->get_term());
    for(int j = 1; j<rearranged.size(); j++){
      c_aux.add_term(rearranged[j]->get_term());
      //fazer lista dos proximos filhos/estados
      vector<Node*> j_children = rearranged[j]->get_children();
      vector<int> j_pos = get_io_pos(1,rearranged[j]->get_term());
      vector<string> j_vars = get_io_vars(j_pos,rearranged[j]->get_term().get_args());
      vector<string> not_consistent;
      for(int k = 0; k<j_vars.size(); k++)
	if(find(outconst_vars.begin(),outconst_vars.end(),j_vars[k])==outconst_vars.end()){
	  not_consistent.push_back(j_vars[k]);
	}
      vector<Node*> tutti;
      if(not_consistent.size()>0){
	for(int k = 0; k<not_consistent.size(); k++){
	  vector<Term> con_terms = get_io_terms(not_consistent,0);
	  for(int m = 0; m<con_terms.size(); m++)
	    tutti.push_back(nodes_created[con_terms[m].to_string()]);
	}
      }
      for(int k = 0; k<j_children.size(); k++)
	c_aux.add_to_candidates(j_children[k]);
      for(int k = 0; k<tutti.size(); k++)
	c_aux.add_to_candidates(tutti[k]);
      for(int k = 0; k<outconst_vars.size(); k++)
	c_aux.add_to_outvars(outconst_vars[k]);
      for(int k = 0; k<in_vars.size(); k++)
	c_aux.add_to_invars(in_vars[k]);
      c_aux.add_to_raw(rearranged[j]);
    }
    to_expand.push_back(c_aux);
  }

  paths.clear();

  clock_t cov_start,cov_end,pathl_start,pathl_end;
  double cov_time, pathl_time;
  
  while(to_expand.size()>0){
    gen_c++;
    
    Clause hyper_aux = to_expand[0];
    to_expand.erase(to_expand.begin());

    vector<Node*> cands = hyper_aux.get_candidates();
    vector<Node*> raw = hyper_aux.get_raw();
    int score = INT_MIN;
    int cand_pos = -1;

    cov_start = clock();
    vector<string> positive_cover = covers(hyper_aux);
    vector<string> negative_cover = coversn(hyper_aux);
    int cov = positive_cover.size() - negative_cover.size();
    cov_end = clock();
    cov_time = (double) (cov_end - cov_start) / CLOCKS_PER_SEC;
    coverage_total_time += cov_time;

    pathl_start = clock();
    int length = path_length(hyper_aux);
    pathl_end = clock();
    pathl_time = (double) (pathl_end - pathl_start) / CLOCKS_PER_SEC;
    pathlength_total_time += pathl_time;
    
    //cout << "PATH LENGTH: " << length << endl;
    int sc = cov-length;

    //cout << "score: " << sc;
    
    //cout << hyper_aux.to_string() << cov << endl;
    
    for(int i = 0; i<cands.size(); i++){
      vector<Node*> new_hyp = raw;
      Clause c_tmp = hyper_aux;
      new_hyp.push_back(cands[i]);
      c_tmp.add_term(cands[i]->get_term());
      /*      cout << "EVALUATING: " << endl;
      for(int j = 0; j<new_hyp.size(); j++)
	cout << new_hyp[j]->get_term().to_string() << " ";
	cout << endl;*/
      cov_start = clock();
      vector<string> positive_cover = covers(c_tmp);
      vector<string> negative_cover = coversn(c_tmp);
      int cov = positive_cover.size() - negative_cover.size();
      cov_end = clock();
      cov_time = (double) (cov_end - cov_start) / CLOCKS_PER_SEC;
      coverage_total_time += cov_time;

      pathl_start = clock();
      length = path_length(c_tmp);
      pathl_end = clock();
      pathl_time = (double) (pathl_end - pathl_start) / CLOCKS_PER_SEC;
      pathlength_total_time += pathl_time;
      
      int sc_tmp = cov - length;
      if(sc_tmp > score && sc_tmp > sc){
	score = sc_tmp;
	cand_pos = i;
      }
    }

    if(cand_pos != -1){
      Node* niet = cands[cand_pos];
    
      vector<string> hyper_outv = hyper_aux.get_outvars();
      vector<Node*> n_children = niet->get_children();
      vector<int> n_pos = get_io_pos(1,niet->get_term());
      vector<string> n_vars = get_io_vars(n_pos,niet->get_term().get_args());
      vector<string> not_cons;
      
      for(int i = 0; i<n_vars.size(); i++)
	if(find(hyper_outv.begin(),hyper_outv.end(),n_vars[i])==hyper_outv.end())
	  not_cons.push_back(n_vars[i]);
      
      vector<Node*> ats;
    
      if(not_cons.size()>0){
	for(int i = 0; i<not_cons.size(); i++){
	  vector<Term> c_terms = get_io_terms(not_cons,0);
	  for(int m = 0; m<c_terms.size(); m++)
	    ats.push_back(nodes_created[c_terms[m].to_string()]);
	}
      }			 
      
      for(int i = 0; i<n_children.size(); i++)
	if(find(cands.begin(), cands.end(), n_children[i])==cands.end())
	  hyper_aux.add_to_candidates(n_children[i]);
      
      for(int i = 0; i<ats.size(); i++)
	if(find(cands.begin(), cands.end(), ats[i])==cands.end())
	  hyper_aux.add_to_candidates(ats[i]);
      
      n_pos = get_io_pos(0,niet->get_term());
      n_vars = get_io_vars(n_pos,niet->get_term().get_args());
      
      for(int i = 0; i < n_vars.size(); i++){
	if(find(hyper_outv.begin(),hyper_outv.end(),n_vars[i])==hyper_outv.end())
	  hyper_aux.add_to_outvars(n_vars[i]);
      }
      
      hyper_aux.add_term(niet->get_term());
      hyper_aux.add_to_raw(niet);
      
      if(score == max_score && find(max_clauses.begin(),max_clauses.end(),hyper_aux)==max_clauses.end()){
	//remove_clauses_covered(hyper_aux);
	max_clauses.push_back(hyper_aux);
      }
      if(score>max_score){
	max_score = score;
	max_clauses.clear();
	//remove_clauses_covered(hyper_aux);
	max_clauses.push_back(hyper_aux);
      }

      //cout << "NEW: " << hyper_aux.to_string() << endl;
      
      to_expand.push_back(hyper_aux);
    }
    else{
      if(sc == max_score && find(max_clauses.begin(),max_clauses.end(),hyper_aux)==max_clauses.end()){
	//remove_clauses_covered(hyper_aux);
	max_clauses.push_back(hyper_aux);
      }
      if(sc>max_score){
	max_score = sc;
	max_clauses.clear();
	max_clauses.push_back(hyper_aux);
	//remove_clauses_covered(hyper_aux);
      }
    }
  }

  return max_clauses;
  
}

vector<Node*> get_not_expanded(vector<Node*> v){
  v = rearrange(v);

  Clause c(v[0]->get_term());
  for(int j = 1; j<v.size(); j++)
    c.add_term(v[j]->get_term());

  vector<string> exp = expanded[c.to_string()];
  
  vector<Node*> to_expand;
  for(int i = 0; i<v.size(); i++)
    if(find(exp.begin(),exp.end(),v[i]->get_term().to_string())==exp.end())
      to_expand.push_back(v[i]);
  return to_expand;
}

//TODO
bool io_consistent(vector<Node*> v){

  /*cout << "A avaliar: ";
  for(int i = 0; i<v.size(); i++)
    cout << v[i]->get_term().to_string() << " ";
    cout << endl;*/
  
  vector<Node*> v2;
  for(int i = 0; i<v.size(); i++)
    if(strcmp(v[i]->get_term().get_name().c_str(),bottom.get_head().get_name().c_str())!=0)
      v2.push_back(v[i]);

  vector<int> posh = get_io_pos(1,v[0]->get_term());
  
  set<int> s;
  unsigned size = posh.size();
  for( unsigned j = 0; j < size; ++j ) s.insert( posh[j] );
  posh.assign( s.begin(), s.end() );
  /*cout << "v2: ";
  for(int i = 0; i<v2.size(); i++)
    cout << v2[i]->get_term().to_string() << " ";
    cout << endl;*/
  
  vector<string> head_input_vars = get_io_vars(posh,v[0]->get_term().get_args());
  
  /*cout << "head input vars" << endl;
  for(int i = 0; i<head_input_vars.size(); i++)
    cout << head_input_vars[i] << " ";
    cout << "______" << endl;*/
  vector<bool> consistent;
  for(int i = 0; i<v2.size(); i++)
    consistent.push_back(true);
  
  for(int i = 0; i< v2.size(); i++){
    Node* aux = v2[i];
    //cout << aux->get_term().to_string() << endl;
    vector<int> pos = get_io_pos(1,aux->get_term());
    /*cout << "got pos" << endl;
    for(int j = 0; j<pos.size(); j++)
      cout << pos[j] << " ";
      cout << endl;*/
    set<int> s;
    unsigned size = pos.size();
    for( unsigned j = 0; j < size; ++j ) s.insert( pos[j] );
    pos.assign( s.begin(), s.end() );
    vector<string> args = aux->get_term().get_args();
    /*for(int j = 0; j<args.size(); j++)
      cout << args[j] << " ";
      cout << endl;*/
    vector<string> input_vars = get_io_vars(pos,args);
    //cout << "got vars" << endl;
    /*for(int j = 0; j<input_vars.size(); j++)
      cout << input_vars[j] << " ";
      cout << endl;*/
    for(int j = 0 ; j<input_vars.size(); j++)
      if(find(head_input_vars.begin(),head_input_vars.end(),input_vars[j])!=head_input_vars.end())
	input_vars.erase(input_vars.begin()+j);
    if(input_vars.size()!=0){
      //cout << "nao consistente" << endl;
      consistent[i] = false;
    }
    else{
      //cout << "consistente" << endl;
      consistent[i] = true;
    }
    for(int j = 0; j<i; j++){
      Node* aux2 = v2[j];
      //cout << aux2->get_term().to_string() << endl;
      vector<int> pos_out = get_io_pos(0,aux2->get_term());
      /*cout << "pos j" << endl;
      for(int k = 0; k<pos_out.size(); k++)
	cout << pos_out[k] << " ";
	cout << endl;*/
      vector<string> out_vars = get_io_vars(pos_out,aux2->get_term().get_args());
      //cout << "vars j" << endl;
      for(int k = 0 ; k<input_vars.size(); k++)
	if(find(out_vars.begin(),out_vars.end(),input_vars[k])!=out_vars.end())
	  input_vars.erase(input_vars.begin()+k);
    if(input_vars.size()==0)
      consistent[i] = true;
    else
      consistent[i] = false;
    }
  }

  for(int i = 0; i<consistent.size(); i++)
    if(!consistent[i])
      return false;
  return true;
}

//TODO
vector<string> reachable_vars(vector<Node*> v){
  vector<string> reachable;
  for(int i = 0; i<v.size(); i++){
    Node* aux = v[i];
    vector<string> vars = aux->get_term().get_args();
    for(int j = 0; j<vars.size(); j++)
      if(find(reachable.begin(),reachable.end(),vars[j])==reachable.end())
	reachable.push_back(vars[j]);
  }
  return reachable;
}

//TODO
vector<string> merge_vars(vector<string> v1, vector<string> v2){
  for(int i = 0; i<v2.size(); i++)
    if(find(v1.begin(),v1.end(),v2[i])==v1.end())
      v1.push_back(v2[i]);
  return v1;
}

//TODO
vector<Node*> merge(vector<Node*> v1, vector<Node*> v2){
  vector<Node*> to_merge;
  for(int i = 0; i<v2.size(); i++){
    if(find(v1.begin(),v1.end(),v2[i])==v1.end()){
      to_merge.push_back(v2[i]);
    }
  }
  if(to_merge.size()>0){
    if(v1.size()>0)
      for(int i = to_merge.size()-1; i>=0; i--){
	v1.insert(v1.begin()+1,to_merge[i]);
      }
    else{ 
      for(int i = to_merge.size()-1; i>=0; i--){
	v1.push_back(to_merge[i]);
      }
    }
  }
  return v1;
}

//TODO
vector<string> intersection(map<string,vector<string>> m){
  vector<string> vars = m.begin()->second;
  vector<string> inter;
  bool flag=true;
  map<string, vector<string>>::iterator it;
  for( int i = 0; i<vars.size(); i++){
    string s = vars[i];
    for(it = m.begin(); it!= m.end(); it++){
      vector<string> vars_aux = it->second;
      if(find(vars_aux.begin(),vars_aux.end(),s)==vars_aux.end()){
	//cout << s << " ta na intersecção" << endl;
	flag=false;
	break;
      }
    }
    if(flag)
      inter.push_back(s);
    else
      flag = true;
  }
  return inter;
}

//TODO
bool contains_intersection(string s, vector<Node*> v){
  for(int i = 0; i<v.size(); i++){
    Node* aux = v[i];
    vector<string> args = aux->get_term().get_args();
    if(find(args.begin(),args.end(),s)!=args.end())
      return true;
  }
  return false;
}

//TODO
void generate_all_combinations(map<string,vector<vector<Node*>>> m, int n, vector<Node*> k){
  if(n==m.size()){
    k = rearrange(k);
    Clause c (k[0]->get_term());
    for(int i = 1; i<k.size(); i++)
      c.add_term(k[i]->get_term());
    bool uni = false;
    if(pathsc.size()>0)
      for(int i = 0; i<pathsc.size(); i++){
	//clock_t s = clock();
	bool unifies = unifies_with(pathsc[i],c);
	//clock_t e = clock();
	//cout << "uni: " << (double) (e-s) / CLOCKS_PER_SEC << endl;
	if(unifies){
	  uni = true;
	  break;
	}
      }
    if(!uni){
      pathsc.push_back(c);
      //cout << "New Clause: " << endl;
    }
    //cout << pathsc.size() << endl;
    return;
  }
  
  map<string, vector<vector<Node*>>>::iterator it = m.begin();
  for(int i = 0; i<n; i++)
    it++;
  vector<vector<Node*>> hyperps = it->second;
  for(int i = 0; i<hyperps.size(); i++){
    vector<Node*> cont = merge(k,hyperps[i]);
    generate_all_combinations(m,n+1,cont);
  }
}

bool is_var(string s){
  if(s[0]=='_' || toupper(s[0])==s[0])
    return true;
  return false;
}

bool is_cons(string s){
  if(s[0]!='_' && tolower(s[0]) == s[0])
    return true;
  return false;
}

Clause replace(Clause c, string to_take, string to_put){
  vector<Term> c_terms = c.get_terms();
  c_terms.insert(c_terms.begin(),c.get_head());
  vector<Term> aux2;
  Term t;
  vector<string> t_args;
  string argj;
  for(int i = 0; i<c_terms.size(); i++){
    t = c_terms[i];
    Term t_aux;
    t_args = t.get_args();
    for(int j = 0; j<t_args.size(); j++){
      argj = t_args[j];
      if(strcmp(argj.c_str(),to_take.c_str())==0){
	t_args.erase(t_args.begin()+j);
	t_args.insert(t_args.begin()+j,to_put);
      }
    }
    t_aux.set_name(t.get_name());
    for(int j = 0; j<t_args.size(); j++)
      t_aux.add_arg(t_args[j]);
    aux2.push_back(t_aux);
  }
  Clause c2 (aux2[0]);
  for(int i = 1; i<aux2.size(); i++)
    c2.add_term(aux2[i]);
  //cout << "REPLACE: " << c2->to_string() << endl;
  return c2;
}

bool unifies_with(Clause c1, Clause c2){
  //cout << "começo uni" << endl;
  Clause aux1 (c1.get_head());
  for(int i = 0; i<c1.get_terms().size(); i++)
    aux1.add_term(c1.get_terms()[i]);
  Clause aux2 (c2.get_head());
  for(int i = 0; i<c2.get_terms().size(); i++)
    aux2.add_term(c2.get_terms()[i]);
  //cout << aux1->to_string() << " vs " << aux2->to_string() << endl;
  vector<Term> aux1t = aux1.get_terms();
  vector<Term> aux2t = aux2.get_terms();
  aux1t.insert(aux1t.begin(),aux1.get_head());
  aux2t.insert(aux2t.begin(),aux2.get_head());
  if(aux1t.size() != aux2t.size())
    return false;
  //cout << "Tamanho igual" << endl;
  for(int i = 0; i<aux1t.size(); i++){
    Term t1 = aux1t[i];
    Term t2 = aux2t[i];
    vector<string> t1_args = t1.get_args();
    vector<string> t2_args = t2.get_args();
    if(t1_args.size()!=t2_args.size() || strcmp(t1.get_name().c_str(), t2.get_name().c_str())!=0){
      return false;
    }
    //cout << "Tamanho igual" << endl;
    for(int j = 0; j<t1_args.size(); j++){
      string arg1 = t1_args[j];
      string arg2 = t2_args[j];
      if(is_var(arg1)){
	if(is_var(arg2))
	  aux2 = replace(aux2,arg2,arg1);
	else
	  aux1 = replace(aux1,arg1,arg2);
      }
      else{
	if(is_var(arg2))
	  aux2 = replace(aux2,arg2,arg1);
	else if(strcmp(arg1.c_str(),arg2.c_str())!=0){
	  return false;
	}
      }
      /*
      if(is_var(arg1) && is_var(arg2))
	aux2 = replace(aux2, arg2, arg1);
      else if(is_var(arg1) && is_cons(arg2))
	aux1 = replace(aux1, arg1, arg2);
      else if(is_cons(arg1) && is_var(arg2))
	aux2 = replace(aux2, arg2, arg1);
      else if(is_cons(arg1) && is_cons(arg2))
	if(strcmp(arg1.c_str(),arg2.c_str())!=0)
	return false;*/
    }
  }
  if(strcmp(aux1.to_string().c_str(),aux2.to_string().c_str())==0)
    return true;
  return false;
}

void create_test_modes(){
  vector<string> x1;
  x1.push_back("+");
  x1.push_back("-");
  vector<string> x3;
  x3.push_back("+");
  x3.push_back("+");
  vector<string> x2;
  x2.push_back("+");
  x2.push_back("+");
  x2.push_back("-");
  /*m_c.push_back(new Mode("a",x3,'h'));
  m_c.push_back(new Mode("c",x1,'b'));
  m_c.push_back(new Mode("b",x1,'b'));
  m_c.push_back(new Mode("g",x1,'b'));
  m_c.push_back(new Mode("f",x1,'b'));
  m_c.push_back(new Mode("d",x1,'b'));
  m_c.push_back(new Mode("e",x2,'b'));
  m_c.push_back(new Mode("e",x1,'b'));*/
}

void create_test_clause(){
  Term t1;
  t1.set_name("a");
  t1.add_arg("X");
  t1.add_arg("Y");
  Term t2;
  t2.set_name("c");
  t2.add_arg("X");
  t2.add_arg("W");
  Term t3;
  t3.set_name("c");
  t3.add_arg("X");
  t3.add_arg("B");
  Term t4;
  t4.set_name("b");
  t4.add_arg("Y");
  t4.add_arg("Z");
  Term t5;
  t5.set_name("b");
  t5.add_arg("Y");
  t5.add_arg("A");
  Term t6;
  t6.set_name("d");
  t6.add_arg("W");
  t6.add_arg("F");
  Term t7;
  t7.set_name("e");
  t7.add_arg("Z");
  t7.add_arg("C");
  Term t8;
  t8.set_name("g");
  t8.add_arg("A");
  t8.add_arg("F");
  Term t9;
  t9.set_name("e");
  t9.add_arg("F");
  t9.add_arg("B");
  t9.add_arg("D");
  Term t10;
  t10.set_name("f");
  t10.add_arg("C");
  t10.add_arg("B");
  Clause b_c (t1);
  b_c.add_term(t2);
  b_c.add_term(t3);
  b_c.add_term(t4);
  b_c.add_term(t5);
  b_c.add_term(t6);
  b_c.add_term(t7);
  b_c.add_term(t8);
  b_c.add_term(t9);
  b_c.add_term(t10);
}

vector<Node*> rearrange(vector<Node*>v){
  vector<Node*> v2;
  for(int i = 0; i<v.size(); i++){
    if(strcmp(v[i]->get_term().get_name().c_str(),bottom.get_head().get_name().c_str())!=0)
      v2.push_back(v[i]);
  }
  Node* head = new Node(bottom.get_head());
  v2.insert(v2.begin(),head);
  return v2;
}

int main(int argc, char* argv[]){
  srand(time(0));
  clock_t start_main;
  char *av[2];
  int ac = 0;
  vector<Clause> clauses;
  av[ac++] = argv[0];
  av[ac] = NULL;
  double saturation_total_time = 0;
  double hyperg_total_time = 0;
  double search_total_time = 0;
  
  string text_bottom;
  string file;
  
  PlEngine e (ac,av);

  for(int i = 0; i<argc; i++){
    if(strcmp(argv[i],"-f")==0)
      file = argv[++i];
    if(strcmp(argv[i],"-t")==0)
      threshold = (double) atoi(argv[++i]);
    if(strcmp(argv[i],"-h")==0)
      heuristic = argv[++i];
    if(strcmp(argv[i],"-n")==0)
      pos_number = atoi(argv[++i]);
  }
  
  ifstream posFile;
  string dotf = file + ".f";
  posFile.open(dotf, ios::in);
  string linef;
  if(posFile.is_open()){
    while(getline(posFile,linef)){
      //cout << "LINEF: " << linef << endl;
      positives_f.push_back(linef);
      positives++;
      positive_index.push_back(positives);
    }
  }
  else{
    cerr << "No positive facts given" << endl;
    exit(1);
  }
  //cout << positives_f.size() << endl;
  //cout << positive_index.size() << endl;
  ifstream negFile;
  string dotn = file + ".n";
  negFile.open(dotn, ios::in);
  string linen;

  if(negFile.is_open()){
    while(getline(negFile,linen))
      negatives++;
  }
  else{
    cerr << "No negative facts given" << endl;
    exit(1);
  }
  
  ifstream modeFile;
  string dotb = file + ".b";
  modeFile.open(dotb, ios::in);
  string line;

  vector<string> mode_lines;
  
  if(modeFile.is_open()){
    while(getline(modeFile,line)){
      size_t pos = line.find("mode");
      if(pos!=string::npos && line.at(0)==':'){
	line.erase(remove(line.begin(),line.end(),' '),line.end());
	mode_lines.push_back(line);
      }
    }
  }
  else{
    cerr << "No mode delarations found" << endl;
    exit(1);
  }
  modeFile.close();

  mode_decs = parse_modes(mode_lines);
  //create_test_modes();

  //cout << "aqui" << endl;

  
  start_main = clock();

  cases = positives;
  vector<int> rand_pos;
  
  if(pos_number > 0){
    vector<string> new_pos_f;
    vector<int> new_pos_index;
    cases = pos_number;

    rand_pos.push_back(-1);
    
    for(int i = 0; i<pos_number; i++){
      srand(i+1);
      int v1 = rand() % positives + 1;
      if(find(rand_pos.begin(),rand_pos.end(),v1)==rand_pos.end()){
	new_pos_f.push_back(positives_f[v1]);
	new_pos_index.push_back(v1);
	rand_pos.push_back(v1);
	//cout << v1 << endl;
      }
    }
    positives_f = new_pos_f;
    positive_index = new_pos_index;
  }
  
  while(positives_f.size()>0){
    cout << positives_f.size() << endl;
    //generating bottom clause
    try{
      PlCall("[test].");
      string read = "read_all(" + file + ")";
      PlCall(read.c_str());
      string s;
      clock_t sat_start = clock();
      int i = positive_index[0];
	s = "sat(" + to_string(i);
	s = s + ",'sat.txt').";
      //cout << s << endl;
      PlCall(s.c_str());
      PlTermv vv(1);
      PlQuery q1("bottom",vv);
      q1.next_solution();
      text_bottom = (char*) vv[0];
      clock_t sat_end = clock();
      double sat_time = (double) (sat_end - sat_start) / CLOCKS_PER_SEC;
      saturation_total_time += sat_time;
      //cout << text_bottom << endl;
    }
    catch(exception &ex){
      cerr << "ERR: " << ex.what() << endl;
      exit(1);
    }
    catch(PlException &ex){
      cerr << (char*) ex << endl;
      exit(1);
    }
    
    cout << "Text: " << text_bottom << endl;
    string b_text = add_spaces(text_bottom);
    
    if(strcmp(b_text.c_str(),"")==0){
      cout << "File is empty. Aborting execution.\n";
      exit(1);
    }
    positives_f.erase(positives_f.begin());
    positive_index.erase(positive_index.begin());
    vector<string> tokens = parse_bottom_clause(b_text);
    bottom = build_clause(tokens);
    cout << "The bottom clause generated is: ";
    //bottom clause generated
    //create_test_clause();
    //creation of hypergraph according to Mode Directed Pathfinding
    //bottom=b_c;
    //get mode declarations of predicates
    cout << bottom.to_string() << endl;
    
    //bottom = b_c;
    //mode_decs = m_c;
    
    //hypergraph construction
    Node* root = new Node(bottom.get_head());
    hyperg = new Hypergraph(root);
    clock_t start_hyperg = clock();
    //cout << "create hypergraph" << endl;
    generateHG();
    //cout << hyperg->get_size() << " nodes" << endl;
    clock_t end_hyperg = clock();
    double hyperg_time = (double) (end_hyperg - start_hyperg) / CLOCKS_PER_SEC;
    //cout << "Hypergraph created in: " << hyperg_time << endl;
    hyperg_total_time += hyperg_time;
    /*
      start = clock();
      print_hg_bfs();
      end = clock();
      exec_time = (double) (end - start) / CLOCKS_PER_SEC;
      cout << "bfs in: " << exec_time << endl;
      start = clock();
      print_hg_dfs();
      end = clock();
      exec_time = (double) (end - start) / CLOCKS_PER_SEC;
      cout << "dfs in: " << exec_time << endl;
    */
    //hypergraph transformation
    
    clock_t search_start = clock();
    clauses = transform_hypergraph();
    clock_t search_end = clock();
    double search_time = (double) (search_end - search_start) / CLOCKS_PER_SEC;
    //cout << "Search performed in: " << search_time << endl;
    search_total_time += search_time;
    //cout << "SIZE: " << positives_f.size() << endl;
    for(int k = 0; k<clauses.size(); k++)
      remove_clauses_covered(clauses[k]);
    //cout << "SIZE2: " << positives_f.size() << endl;
    //cout << max_score << endl;
    /*
    start = clock();
    print_hg_bfs();
    end = clock();
    exec_time = (double) (end - start) / CLOCKS_PER_SEC;
    cout << "bfs in: " << exec_time << endl;
    start = clock();
    print_hg_dfs();
    end = clock();
    exec_time = (double) (end - start) / CLOCKS_PER_SEC;
    cout << "dfs in: " << exec_time << endl;
    */
    //search for possible theories
    /*cout << "starting search" << endl;
    start = clock();
    search_hyper();
    end = clock();
    exec_time2 = (double) (end - start) / CLOCKS_PER_SEC;
    cout << "finished with: " << exec_time2 << endl;
    delete hyperg;
    delete root;*/
  }  

  
  clock_t end_main= clock();

  double exec_main = (double) (end_main - start_main) / CLOCKS_PER_SEC;

  cout << "TESTING RESULTS:" << endl;

  cout << "Number of generated clauses: " << gen_c << endl;

  cout << "Execution time: " << exec_main << endl;

  cout << "Total sat time: " << saturation_total_time << endl;

  cout << "Hypergraph total time: " << hyperg_total_time << endl;

  cout << "Search total time: " << search_total_time << endl;

  cout << "Coverage total time: " << coverage_total_time << endl;

  cout << "Path length total time: " << pathlength_total_time << endl;
  
  if(clauses.size() >0){

    max_cov = max_score; //- path_length(clauses[0].get_raw())

    cout << "Max coverage: " << max_cov << endl;

    cout << "Clauses: " << endl;
    for(int i = 0; i<clauses.size(); i++)
      cout << clauses[i].to_string() << endl;

    cout << clauses.size() << endl;
    
  }
  else{
    cout << "No clause with more than 0 coverage was found" << endl;
  }
  
  return 0;
}

/*falta:
alterar nome do ficheiro (criar copias de .f, .n e .b com sufixo tmp)
modificar constantemente .n e .f
actualizar dinamicamente covers e coversn de cada hipótese candidata
*/
