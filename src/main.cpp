#include <iostream>
#include <string>
#include <stdint.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>

using namespace std;
using namespace cv;

//based on the value of r from https://en.wikipedia.org/wiki/Ordered_dithering
//formula is hightestValue / cube_root(numColors)
//r = 255 / cube_root(2) because we are using 2 colors
#define DITHER_R_COEFFICIENT 0.7937005259840997

int printUsage(char *programName){
    cerr << "Usage: " << programName << " <input_file> <output_file.avi>" << endl; 
    return -1;
}

float bayer2[2][2] = {
                        {-127.0f * DITHER_R_COEFFICIENT,  42.0f * DITHER_R_COEFFICIENT},
                        {127.0f * DITHER_R_COEFFICIENT, -42.0f * DITHER_R_COEFFICIENT} 
                    };


void bayerDither(Mat *image){
    int width = image->cols;
    int height = image->rows;
    uint8_t currentLightness = 0;
    int bayerDimensions = 2;

    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            currentLightness = image->at<uint8_t>(y,x);
            uint8_t outputColor = currentLightness + bayer2[y%bayerDimensions][x%bayerDimensions] > 127 ? 255 : 0;
            image->at<uint8_t>(y,x) = outputColor;
        }
    }
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
    Mat outputFrame;

    //for each frame in input video
    for(;;){
        //read frame
        inputVideo >> currentFrame;
        // check if at end
        if(currentFrame.empty()){
            break;
        }
        // alteredFrame = currentFrame.clone();
        //convert to grayscale
        cvtColor(currentFrame, alteredFrame, COLOR_RGB2GRAY);
        bayerDither(&alteredFrame);
        
        //convert back to rgb so we can save it 
        cvtColor(alteredFrame, outputFrame, COLOR_GRAY2BGR);
        //send frame to output
        outputVideo << outputFrame;
    }

    return 0;
}