#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) {
    vector<Mat> images;
    vector<String> image_paths;
    String folder_path = ".../Unstitched_Images/*.jpeg";
    glob(folder_path, image_paths, true);

    for (auto& image : image_paths) {
        Mat img = imread(image);
        if (img.empty()) {
            cout << "Unable to read image " << image << endl;
            return -1;
        }
        images.push_back(img);
        imshow("Image", img);
        waitKey(0);
    }

    Mat stitched_img;
    Ptr<Stitcher> imageStitcher = Stitcher::create(Stitcher::Mode::PANORAMA);

    Stitcher::Status status = imageStitcher->stitch(images, stitched_img);

    if (status == Stitcher::OK) {
        imwrite("stitchedoutput.png", stitched_img);
        imshow("Stitched Image", stitched_img);
        waitKey(0);
    }
    else {
        cout << "Images could not be stitched!" << endl;
        cout << "Likely not enough keypoints being detected!" << endl;
        return -1;
    }

    //Post Processing
    int border_size = 10;
    copyMakeBorder(stitched_img, stitched_img, border_size, border_size, border_size, border_size, BORDER_CONSTANT, Scalar(0, 0, 0));
    Mat gray_img;
    cvtColor(stitched_img, gray_img, COLOR_BGR2GRAY);
    Mat thresh_img;
    threshold(gray_img, thresh_img, 0, 255, THRESH_BINARY);

    imshow("Threshold Image", thresh_img);
    waitKey(0);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(thresh_img.clone(), contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Point> areaOI;
    double max_area = 0;
    for (auto& cnt : contours) {
        double area = contourArea(cnt);
        if (area > max_area) {
            max_area = area;
            areaOI = cnt;
        }
    }

    Mat mask = Mat::zeros(thresh_img.size(), CV_8UC1);
    Rect rect = boundingRect(areaOI);
    rectangle(mask, rect, Scalar(255), -1);

    Mat minRectangle = mask.clone();
    Mat sub = mask.clone();
    while (countNonZero(sub) > 0) {
        erode(minRectangle, minRectangle, Mat());
        subtract(minRectangle, thresh_img, sub);
    }

    vector<vector<Point>> contours2;
    vector<Vec4i> hierarchy2;
    findContours(minRectangle.clone(), contours2, hierarchy2, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Point> areaOI2;
    double max_area2 = 0;
    for (auto& cnt : contours2) {
        double area = contourArea(cnt);
        if (area > max_area2) {
            max_area2 = area;
            areaOI2 = cnt;
        }
    }

    Rect rect2 = boundingRect(areaOI2);
    stitched_img = stitched_img(rect2);
    imwrite("stitchedOutputProcessed.png", stitched_img);

    imshow("Stitched Image Processed", stitched_img);
    waitKey(0);

    return 0;
}
