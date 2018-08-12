#include <iostream>
#include <string>
#include <stdint.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/core/ocl.hpp>

using namespace std;
using namespace cv;

//based on the value of r from https://en.wikipedia.org/wiki/Ordered_dithering
//formula is hightestValue / cube_root(numColors)
//r = 255 / cube_root(2) because we are using 2 colors
#define DITHER_R_COEFFICIENT 0.7937005259840997

#define SUPPORTED_OUTPUT_EXTENSION ".avi"

int printUsage(char *programName){
    cerr << "Usage: " << programName << " <input_file> <output_file.avi>" << endl; 
    return -1;
}

//from: https://stackoverflow.com/questions/20446201/how-to-check-if-string-ends-with-txt
bool endsWith(const std::string &str, const std::string &suffix){
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
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

cv::ocl::Program compileCustomOpenCLKernel(cv::ocl::Context context){
    cv::ocl::Program program;

    std::ifstream ifs("../kernels/bayer.cl");
    if (ifs.fail()){
        return program;
    }
    std::string kernelSource((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    cv::ocl::ProgramSource programSource(kernelSource);

    // Compile the kernel code
    cv::String errmsg;
    // cv::String buildopt = cv::format("-D dstT=%s", cv::ocl::typeToStr(umat_dst.depth())); // "-D dstT=float"
    cv::String buildopt = "-D dstT=float"; // "-D dstT=float"
    program = context.getProg(programSource, buildopt, errmsg);

    if(!errmsg.empty()){
        cerr << errmsg << endl;
    }

    return program;
}


int main(int argc, char *argv[]){
    //opencl stuff from: https://stackoverflow.com/questions/28529458/how-to-launch-custom-opencl-kernel-in-opencv-3-0-0-ocl
    if (!cv::ocl::haveOpenCL())
    {
        cerr << "OpenCL is not avaiable..." << endl;
        return -1;
    }

    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_GPU))
    {
        cerr << "Failed creating the OpenCL context..." << endl;
        return -1;
    }
    #ifdef DEBUG
        // In OpenCV 3.0.0 beta, only a single device is detected.
        cout << context.ndevices() << " GPU devices are detected." << endl;
        for (uint i = 0; i < context.ndevices(); i++)
        {
            cv::ocl::Device device = context.device(i);
            cout << "name                 : " << device.name() << endl;
            cout << "available            : " << device.available() << endl;
            cout << "imageSupport         : " << device.imageSupport() << endl;
            cout << "OpenCL_C_Version     : " << device.OpenCL_C_Version() << endl;
            cout << endl;
        }
    #endif

    // Select the first device
    cv::ocl::Device(context.device(0));

    cv::ocl::Program openclProgram = compileCustomOpenCLKernel(context);


    if(argc != 3){
        return printUsage(argv[0]);
    }

    const string inputFileName = argv[1];
    const string outputFileName = argv[2];

    if(!endsWith(outputFileName, SUPPORTED_OUTPUT_EXTENSION)){
        cerr << "Only .avi output is supported" << endl;
        return -1;
    }

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