#include "detector.h"

const char* DEFAULT_FACE_CASCADE = "haarcascade_frontalface_alt.xml";
const char* DEFAULT_EYES_CASCADE = "haarcascade_eye_tree_eyeglasses.xml";

const char* title = "Image";

struct by_x: public binary_function< vector<Rect>::iterator&, vector<Rect>::iterator&, bool>
{
    bool operator()(const Rect &a,const Rect &b) {
        return a.x<b.x;
    }
};

//static Rect r;
struct pred_within_rect {
    Rect r;
    bool operator()(const Rect& a) {
        return r.contains(a.tl()) && r.contains(a.br());
    }
};
Detector::Detector(bool aDebug):
    debug(aDebug),
    faceCascadeName(DEFAULT_FACE_CASCADE),
    eyesCascadeName(DEFAULT_EYES_CASCADE)
{
}

Detector::~Detector()
{
}

/**
  loads image from the given path and shows it
  */
Detector::Status Detector::loadImage(const string path)
{
    imageSrc = path;
    return doLoadImage(imageSrc, image, title);
}

Detector::Status Detector::loadSpectacles(const string path)
{
    spectaclesSrc = path;
    Status status = doLoadImage(spectaclesSrc, spectacles);
    if (status!=StatusOK) return status;
    //crop the specs
    doThreshold(spectacles, specMask, 150, 255, CV_THRESH_BINARY_INV );
    if (debug) {
        namedWindow("Threshold");
        imshow("Threshold",specMask);
    }
    int lowRow = -1, highRow = specMask.rows;
    while(sum(specMask.row(++lowRow))==Scalar::all(0));
    while(sum(specMask.row(--highRow))==Scalar::all(0));
    spectacles = spectacles.rowRange(lowRow, highRow);
    specMask = specMask.rowRange(lowRow, highRow);
    int lowCol = -1, highCol = specMask.cols;
    while(sum(specMask.col(++lowCol))==Scalar::all(0));
    while(sum(specMask.col(--highCol))==Scalar::all(0));
    spectacles = spectacles.colRange(lowCol, highCol);
    if (debug) {
        namedWindow("Spectacles");
        imshow("Spectacles", spectacles);
    }
    return StatusOK;
}

Detector::Status Detector::loadModels(const string path)
{
    eyesCascade.load(path+eyesCascadeName);
    faceCascade.load(path+faceCascadeName);
    if( eyesCascade.empty() || faceCascade.empty() )
    {
        return StatusNoModel;
    }
    return StatusOK;
}

bool Detector::isReady() {
    return (!eyesCascade.empty() && !faceCascade.empty() && image.data && spectacles.data);
}

Detector::Status Detector::doLoadImage(const string path, Mat& dest, const string windowName)
{
    dest = imread(path.c_str());
    if (!dest.data) {
        return StatusNotLoaded;
    }
    if (!windowName.empty()) {
        namedWindow(windowName);
        imshow(windowName, dest);
    }
    return StatusOK;
}

void Detector::doThreshold(const Mat& source, Mat& dest, double thresholdvalue, double maxvalue, int type, bool adaptive)
{
    Mat planes[3] = {Mat(source.rows, source.cols, CV_8UC1)};
    dest = Mat(source.rows, source.cols, CV_8UC1);
    split(source, planes);
    Mat tmp = (planes[0]+planes[1]+planes[2])/3;
    if (adaptive) {
        adaptiveThreshold(tmp, dest, thresholdvalue,CV_ADAPTIVE_THRESH_MEAN_C, type, 3, 5);
    } else {
        threshold(tmp, dest, thresholdvalue, maxvalue, type);
    }
}

void Detector::doMarkObjects(const vector<Rect>& objects, const Scalar color)
{
    // mark the found objects on the image
    for(int i = 0; i < objects.size(); i++ )
    {
        const Rect& r = objects[i];
        circle(     image,
                    Point((r.x+r.width/2), (r.y+r.height/2)),
                    (r.width)/2,
                    color, 1, 1, 0
                    );
    }
}

void Detector::doPutSpectaclesOnFace(const Rect face, vector<Rect>::const_iterator eye )
{
    //assumption: spectacles should be larger than detected eyes, and approximately as wide as a face
    int left = face.x;
    int right = left+face.width;
    int scaledHeight = spectacles.rows*(right-left)*1./spectacles.cols;
    int top = (*eye).y +((*eye).height/2) - scaledHeight/2;
    if (top<0) top=1;
    int bottom = top+scaledHeight;

    //resize spectacles and the copy mask
    Mat scaledSpecs(bottom-top, right-left, CV_8UC1);
    Mat scaledSpecsMask(scaledSpecs.size(), CV_8UC1);
    resize(spectacles, scaledSpecs,scaledSpecs.size() );
    doThreshold(scaledSpecs, scaledSpecsMask, 160, 255, CV_THRESH_BINARY_INV);
    //resize(specMask,scaledSpecsMask, scaledSpecs.size());

    //put on target
    const Rect dstRect(left, top, right-left, bottom-top);
    Mat target(image,dstRect );
    bitwise_and(image(dstRect), scaledSpecs, target, scaledSpecsMask);
    bitwise_or(image(dstRect), scaledSpecs, target, scaledSpecsMask);

 }

Detector::Status Detector::detectAndDraw()
{
    const Scalar eye_color({0,255,255});
    const Scalar face_color({255,128,0});
    vector<Rect> objects;
    vector<Rect> faces;

    //check the data
    if( eyesCascade.empty() || faceCascade.empty() )
    {
        return StatusNoModel;
    }
    image = imread(imageSrc.c_str() );
    if (!image.data) {
        return StatusNoImage;
    }
    if (!spectacles.data) {
        return StatusNoSpectacles;
    }
    doThreshold(spectacles, specMask, 150, 255, CV_THRESH_BINARY_INV );
    if (debug) {
        namedWindow("Threshold");
        imshow("Threshold",specMask);
    }

    //convert the image to grayscale
    Mat gray(image.rows, image.cols, CV_8UC1);
    cvtColor(image, gray, CV_BGR2GRAY);
    equalizeHist(gray, gray);

    // detect faces and eyes
    faceCascade.detectMultiScale(gray, faces, 1.1, 2|CV_HAAR_SCALE_IMAGE);
    if (debug) doMarkObjects(faces, face_color);

    eyesCascade.detectMultiScale( gray, objects, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE );
    if (debug) doMarkObjects(objects, eye_color);

    //sort the results from left-to-right
    std::sort(faces.begin(), faces.end(), by_x());
    std::sort(objects.begin(), objects.end(), by_x());

    //iterate over all found faces
    pred_within_rect pwr;
    for ( vector<Rect>::iterator face = faces.begin();face<faces.end();face++)
    {
        //process the detected face: if there are eyes found within it, then put spectacles on it
        pwr.r = (*face);
        vector<Rect>::iterator eye =std::find_if(objects.begin(), objects.end(),pwr);
        if (eye!=objects.end())
        {
            doPutSpectaclesOnFace(*face, eye);
        }
    }
    imshow(title, image);
    return StatusOK;
}
