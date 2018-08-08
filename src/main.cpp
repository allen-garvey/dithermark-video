#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

int MAX_KERNEL_LENGTH = 31;
Mat src; Mat dst;


int printUsage(char *programName){
    cerr << "Usage: " << programName << " <input_file> <output_file>" << endl; 
    return -1;
}

int main(int argc, char **argv){
    if(argc != 3){
        return printUsage(argv[0]);
    }

    const char* inputFileName = argv[1];
    src = imread(inputFileName, IMREAD_COLOR);
    if(src.empty()){
        cerr << "Error opening: " << inputFileName << endl; 
        return printUsage(argv[0]);
    }
    dst = src.clone();

    for(int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2){
        medianBlur (src, dst, i);
    }

    imwrite(argv[2], dst);

    return 0;
}

