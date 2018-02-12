#include "Hypergraph.h"

Hypergraph::Hypergraph(){
}

Hypergraph::Hypergraph(Node* r){
  root = r;
  size = 0;
}

Node* Hypergraph::get_root(){
  return root;
}

int Hypergraph::get_size(){
  return size;
}

void Hypergraph::count(){
  size++;
}
