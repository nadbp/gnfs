#include <iostream>
#include <fstream>
#include <sys/time.h>

using namespace std;

int main(int argc, char* argv[]) {
    struct timeval begin, end;
    unsigned long begin_time, end_time;

    int msg_size = 1000;
    int loop = 2000;
    char message[msg_size];
    for(int i = 0; i < msg_size; i++) {
        message[i] = 'h';
    }


    for(int i = 0; i < 3; i++) {
        if(i ==0){
            loop = 1000;
        } else if( i== 1) {
            loop = 3000;
        } else {
            loop = 5000;
        }

        ofstream test(argv[1]);
        gettimeofday(&begin, NULL);

        for(int i = 0; i < loop; i++) {
            test << message;
        }

        test.close();

        gettimeofday(&end, NULL);

        begin_time = begin.tv_sec * 1000000 + begin.tv_usec;

        end_time = end.tv_sec * 1000000 + end.tv_usec;

        float diff = (float) (end_time - begin_time)/1000;
        float d_size = (float) (loop * msg_size)/1000;
        cout << "Time used: " << diff << "ms" << endl;
        cout << "number of bytes write: " << d_size << "KB" << endl;
    }
}
