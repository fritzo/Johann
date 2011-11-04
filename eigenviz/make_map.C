
#include "graph.h"
#include "eigs.h"
#include <iostream>

#define LOG(mess) { std::cout << mess << std::endl; }
#define DEBUG(mess) LOG("DEBUG " << mess)

const char * help_message = 
"Usage: make_map infile.graph outfile.map\n"
"Example.graph:\n"
"  GRAPH\n"
"  4 VERTICES\n"
"  4 EDGES\n"
"  0 1 0.5\n"
"  1 2 1.0\n"
"  2 3 2.0\n"
"  3 0 4.0\n"
;

int main (int argc, char ** argv)
{
  if (argc < 3) {
    std::cerr << help_message << std::flush;
    return 1;
  }
  const char * infile = argv[1];
  const char * outfile = argv[2];

  Map map;
  map.load_graph(infile);
  compute_eigs(map, map.eigs(), map.num_verts(), map.num_eigs());
  map.convert_eigs();
  map.save(outfile);

  return 0;
}

