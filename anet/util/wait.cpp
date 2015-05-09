#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <time.h>

using namespace std;
int main(int argc, char *argv[]) {
    signal(SIGHUP, SIG_IGN);
    int seconds = 100;
    if (argc > 1) {
        seconds = atoi(argv[1]);
    }
    time_t quit = time(NULL) + seconds;
    while(quit > time(NULL)) {
        sleep(quit - time(NULL));
    }
}
