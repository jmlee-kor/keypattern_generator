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

#define NUM_FILES 100000
#define MAX_BYTES_IDX 16
#define MAX_KEY 256

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
    if(argc < 3) {
        printf("USAGE: generator FILE\n");
        return 1;
    }

    string path(argv[1]);
    string ct;
    // char *** CMRs = (char***)malloc(NUM_FILES*MAX_BYTES_IDX*MAX_KEY);
    static char CMRs[NUM_FILES][MAX_BYTES_IDX][MAX_KEY];
    

    for (uint file_no=1;file_no<=NUM_FILES;file_no++){
        //
        //open and read file
        //
        string filename = path;
        filename += to_string(file_no) + ".txt";
        FILE * f = fopen(filename.c_str(),"rb");

        fseek(f, 0, SEEK_END);
        uint f_size = ftell(f);
        rewind(f);

        ct.resize(f_size);
        fread(&ct[0],1,f_size,f);
        fclose(f);
        

        //
        //compute the number of CMR for given byte index and inferred key
        //
        for(uint byte_index = 0; byte_index<MAX_BYTES_IDX;byte_index++) {
            for(uint inf_key = 0; inf_key<MAX_KEY;inf_key++) {
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

                //
                // store the number of CMR
                //
                CMRs[file_no-1][byte_index][inf_key] = cnt;
            }
        }
    }

    string output_path = "./data";
    mkdir(output_path.c_str(),0777);
    for (uint byte_index = 0; byte_index<MAX_BYTES_IDX;byte_index++) {
        for(uint inf_key = 0; inf_key<MAX_KEY;inf_key++) {
            string byte_path = output_path + "/byte" + to_string(byte_index);
            mkdir(byte_path.c_str(),0777);

            //
            // get output file ready
            //
            string output_filename = byte_path + "/key" + to_string(inf_key);
            FILE* fo = fopen(output_filename.c_str(),"a");

            if(fo==NULL){
                printf("Cannot open the output File : %s \n",output_filename.c_str());
                return 1;
            }


            //
            // print out the number of CMR
            //
            for (uint file_no=1;file_no<=NUM_FILES;file_no++) {
                fprintf(fo,"sampleNo:%d,CMRs:%d\n",file_no,CMRs[file_no][byte_index][inf_key]);
            }


            //
            // close the output file
            //
            fclose(fo);
        }
    }

    return 0;
}
