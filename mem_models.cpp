#include "mem_models.h"
#include <string.h>

int main(int argc, char *argv[]){

    if (argc != 3) {
      cout << "Number of Required Arguments: 3 \
               Format: ./sim_program2 <mem_consistency_model> <input_trace_file> " << endl;
      exit(-1);
    }

    Model* hw;

    if (!strcmp(argv[1], "SC")){
        hw = new SC();
    }else if (!strcmp(argv[1], "PC")){
        hw = new PC();
    }else if (!strcmp(argv[1], "WO")){
        hw = new WO();
    }else if (!strcmp(argv[1], "RC")){
        hw = new RC();
    }else{
        cout << "Invalid Consistency Model: " << argv[1] << endl;
        exit(-1);
    }

    hw->extract(argv[2]);
    hw->print_codevec();

    delete hw;

    return 0;
}
