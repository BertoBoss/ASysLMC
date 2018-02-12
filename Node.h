#ifndef _NODE_
#define _NODE_

#include "Term.h"
#include <string>
#include <vector>

using namespace std;

class Node{
 private:
  Term term;
  vector<Node*> children;
  vector<Node*> parents;
 public:
  Node();
  Node(Term t);
  void add_child(Node* t);
  void add_parent(Node* t);
  vector<Node*> get_children();
  vector<Node*> get_parents();
  Term get_term();

  bool operator == (Node* t);
};

#endif
