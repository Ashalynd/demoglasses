#ifndef DETECTOR_H
#define DETECTOR_H
#include "cv.h"
#include "highgui.h"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;



class Detector
{
//variables
private:
    Mat image, spectacles;
    Mat specMask;
    string imageSrc, spectaclesSrc;
    string pathToModels;
    bool debug;
    CascadeClassifier faceCascade;
    CascadeClassifier eyesCascade;
    string faceCascadeName;
    string eyesCascadeName;
public:
    enum Status {
        StatusOK = 0,
        StatusNoImage = -1,
        StatusNoSpectacles = -2,
        StatusNoModel = -3,
        StatusNotLoaded = -4,
        StatusError = -100
    };

//methods
private:
    Status doLoadImage(const string path, Mat& dest, const string windowName = "");
    void doThreshold(const Mat& source, Mat& dest, double thresholdvalue, double maxvalue, int type, bool adaptive = false);
    void doMarkObjects(const vector<Rect>& objects, const Scalar color);
    void doPutSpectaclesOnFace(const Rect face, vector<Rect>::const_iterator eye);
public:
    Detector(bool debug = false);
    ~Detector();
    Status loadImage(const string path);
    Status loadSpectacles(const string path);
    Status loadModels(const string path);
    Status detectAndDraw();
    bool isReady();
};

#endif // DETECTOR_H
