#ifndef _HYPERG_
#define _HYPERG_

#include "Node.h"

using namespace std;

class Hypergraph{
 private:
  Node* root;
  int size;
 public:
  Hypergraph();
  Hypergraph(Node* r);
  Node* get_root();
  int get_size();
  void count();
};

#endif
