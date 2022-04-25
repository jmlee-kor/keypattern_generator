#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <cassert>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "AES.h"

using namespace std;

int main(int argc, char **argv) {
    // //
    // // check if Td4 is an inverse table of Te4
    // //
    // for (int i=0;i<256;i++) {
    //     int j = Te4[i]>>24;
    //     printf("%dth Te4 is %d, ",i,j);
    //     int k = Td4[j]>>24;
    //     printf("%dth Td4 is %d, same:%c\n",j,k,i==k?'Y':'N');
    //     assert(i==k);
    // }


    //check input values and define shared variables
    if(argc < 4) {
        printf("USAGE: generator FILE ByteIndex InfKey\n");
        return 1;
    }

    string path(argv[1]);
    string ct;
    uint byte_index = atoi(argv[2]);
    uint inf_key = atoi(argv[3]);
    string output_path = "./data";
    output_path += "/byte" + to_string(byte_index);

    if(byte_index >= 16 ) {
        printf("ByteIndex must be lower than 16\n");
        return 1;
    }
    if(inf_key >= 256) {
        printf("InfKey must be lower than 256\n");
        return 1;
    }
    

    //
    // get output file ready
    //
    mkdir(output_path.c_str(),0777);
    string output_filename = output_path + "/key" + to_string(inf_key);
    FILE* fo = fopen(output_filename.c_str(),"a");

    if(fo==NULL){
        printf("Cannot open the output File : %s \n",output_filename.c_str());
        return 1;
    }


    for (int file_no=1;file_no<100000;file_no++){
        //
        //open and read file
        //
        string filename = path;
        // int file_no=1;
        filename += to_string(file_no) + ".txt";
        FILE * f = fopen(filename.c_str(),"rb");

        fseek(f, 0, SEEK_END);
        uint f_size = ftell(f);
        rewind(f);

        ct.resize(f_size);
        fread(&ct[0],1,f_size,f);
        fclose(f);
        
        // printf("%s\n",ct.c_str());
        // printf("%d\n",ct.size());
        // printf("%d,%d\n",byte_index,inf_key);


        //
        //compute the number of CMR for given byte index and inferred key
        //
        char bucket[16] = {0};
        for(int tid=0;tid<32;tid++){
            unsigned char ct_byte_for_tid = ct.c_str()[tid*16+byte_index];
            unsigned char inv_idx_tt = ct_byte_for_tid ^ inf_key;
            unsigned char value = Td4[inv_idx_tt]>>24;
            bucket[value>>4]++;
        }
        int cnt = 0;
        for(int index=0;index<16;index++){
            cnt += bucket[index]>0;
        }
        // printf("%d:%d\n",file_no,cnt);


        //
        // print out the number of CMR
        //
        fprintf(fo,"sampleNo:%d,CMRs:%d\n",file_no,cnt);
    }

    //
    // close the output file
    //
    fclose(fo);

    return 0;
}
