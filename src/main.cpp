#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
using namespace std;
using namespace cv;

int printUsage(char *programName){
    cerr << "Usage: " << programName << " <input_file> <output_file.avi>" << endl; 
    return -1;
}


int main(int argc, char *argv[]){
    if(argc != 3){
        return printUsage(argv[0]);
    }

    const string inputFileName = argv[1];
    const string outputFileName = argv[2];

    VideoCapture inputVideo(inputFileName);
    if(!inputVideo.isOpened()){
        cerr << "Error opening: " << inputFileName << endl; 
        return -1;
    }
    
    int videoCodec = static_cast<int>(inputVideo.get(CAP_PROP_FOURCC));
    Size videoDimensions = Size((int) inputVideo.get(CAP_PROP_FRAME_WIDTH), (int) inputVideo.get(CAP_PROP_FRAME_HEIGHT));
    VideoWriter outputVideo;
    outputVideo.open(outputFileName, videoCodec, inputVideo.get(CAP_PROP_FPS), videoDimensions, true);

    if(!outputVideo.isOpened()){
        cerr  << "Could not open the output video for write: " << outputFileName << endl;
        return -1;
    }
    
    Mat currentFrame;
    Mat alteredFrame;
    
    //for each frame in input video
    for(;;){
        //read frame
        inputVideo >> currentFrame;
        // check if at end
        if(currentFrame.empty()){
            break;
        }
        alteredFrame = currentFrame.clone();
        
        //send frame to output
        outputVideo << alteredFrame;
    }

    return 0;
}