#include "System.h"
#include <iostream>
using namespace std;

System::System(sc_module_name n, std::string train_file, std::string query_file)
    : sc_module(n), tb("tb"), hm("hm"), clk("clk", CLOCK_PERIOD, SC_NS),
      rst("rst") {

  // CLK & RST
  tb.i_clk(clk);
  tb.o_rst(rst);
  hm.i_clk(clk);
  hm.i_rst(rst);

  // Descriptor Channels
  tb.o_train(train_ch);
  hm.i_train(train_ch);
  tb.o_query(query_ch);
  hm.i_query(query_ch);

  // Results Channels
  hm.o_best_idx(best_idx_ch);
  tb.i_best_idx(best_idx_ch);
  hm.o_min_dist(min_dist_ch);
  tb.i_min_dist(min_dist_ch);

  // Read File
  tb.read_train(train_file);
  tb.read_query(query_file);
}

System::~System() {
  cout << "Simulated time == " << sc_time_stamp() << endl;
  cout << "Run time       == " << tb.total_run_time << endl;
  tb.write_results("results.csv");
}
