#include "Node.h"

Node::Node(){
}

Node::Node(Term t){
  term = t;
}

void Node::add_child(Node* t){
  children.push_back(t);
}

void Node::add_parent(Node* t){
  parents.push_back(t);
}

vector<Node*> Node::get_children(){
  return children;
}

vector<Node*> Node::get_parents(){
  return parents;
}

Term Node::get_term(){
  return term;
}
bool Node::operator == (Node* t){
  string s = t->get_term().to_string();
  if(strcmp(this->get_term().to_string().c_str(), s.c_str())==0)
    return true;
  return false;
}
