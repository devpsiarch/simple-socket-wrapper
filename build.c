#include <stdbool.h>
#include "ouroc.h"
#define OUROC_IMPLI

/*THIS IS AN BOILER PLATE FOR A BUILDER FILE*/

int main(int argc,char*argv[]){
    bool run = false;
    if(argc >= 2){
        if(strcmp(argv[1],"run") == 0) run = true;
    }
    Builder tester = {
        .bdir = NULL,
        .buffer_used = 0,
        .buffer_max = 256
    };
    initbuilder(&tester);

    tester.appendcc(&tester,"g++");
    tester.appendtarget(&tester,"main");
    tester.appendsrcs(&tester,"main.cpp");

    tester.appendflags(&tester,"-Wall");
    tester.appendflags(&tester,"-Wextra");
    tester.appendflags(&tester,"-std=c++20");
    tester.appendflags(&tester,"-o");

    tester.construct(&tester);
    SHOWCOMMAND(tester);
    tester.execute(&tester);
    tester.clean_up(&tester);

    if(run) system("./main");

    return 0;
}
