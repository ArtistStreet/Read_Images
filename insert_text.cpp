#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>

using namespace cv;
using namespace std;

int main() {
    Mat image = imread("IMG_0764.jpg");

    if (image.empty()) {
        cerr << "Can not open image!" << std::endl;
        return -1;
    }
    
    int fontFace = FONT_HERSHEY_SIMPLEX;
    double fontScale = 1;
    int thickness = 2;

    Point position(10, 50);

    putText(image, "chao", position, fontFace, fontScale, thickness);
    
    imshow("Display Image", image);

    waitKey(0);

    return 0;
}
