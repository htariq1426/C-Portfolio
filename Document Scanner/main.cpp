Mat imgOriginal, imgGray, imgCanny, img_thres, img_GS, img_Blr, img_Canny, img_Di, imgWarp, imgCrop;

vector<Point> initialpoints,docPoints;

float w = 420, h = 596; //it is x2, scale it accordlingly and uniformly, it is height and width of A4 size paper


//Warp Function

Mat getWarp(Mat img, vector<Point> points, float w, float h)
{
    Mat img_Wrp;
    Point2f src[4] = { points[0], points[1], points[2], points[3] };
    Point2f dst[4] = { {0.0f,0.0f}, {w,0.0f}, {0.0f,h}, {w,h} };

    Mat matrix = getPerspectiveTransform(src, dst);
    warpPerspective(imgOriginal, img_Wrp, matrix, Point(w,h));
    return img_Wrp;
}


//Reorder the edges in the correct and unofrm order

vector<Point> reorder(vector<Point> points)
{
    vector<Point> newPoints;
    vector<int> sumPoints, subPoints;

    for (int i=0; i<4;i++)
    {
        sumPoints.push_back(points[i].x + points[i].y);
        subPoints.push_back(points[i].x - points[i].y);
    }

    //index of minimum in the sumPoints vectors by min_element-begin, get the point and then push it in newPoints
    newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //edge 0
    newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //edge 1

    newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //edge 2

    //index of maximum in the sumPoints vectors and do the same
    newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //edge 3

    return newPoints;
}


void drawPoints(vector<Point> points, Scalar Color)
{
    for (int i = 0; i<points.size();i++)
    {
        circle(imgOriginal, points[i], 10, Color,FILLED);
        putText(imgOriginal, to_string(i), points[i], FONT_HERSHEY_PLAIN, 4, Color, 4);

    }
}

vector<Point> getContours(Mat image) {

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    //drawContours(img, contours, -1, Scalar(255, 0, 255), 2);
    vector<vector<Point>> conPoly(contours.size());
    vector<Rect> boundRect(contours.size());

    vector<Point> biggest;
    int maxArea=0;

    for (int i = 0; i < contours.size(); i++)
    {
        int area = contourArea(contours[i]);
        //cout << area << endl;

        string objectType;

        if (area > 1000)
        {
            float peri = arcLength(contours[i], true);
            approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

            if (area>maxArea && conPoly[i].size()==4){

                //drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 5);
                biggest = {conPoly[i][0],conPoly[i][1],conPoly[i][2],conPoly[i][3]};
                maxArea=area;

            }

            //drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 2);
            //rectangle(imgOriginal, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);
        }
    }
    return biggest;
}


Mat preProcess(Mat img)
{

    cvtColor(img, img_GS, COLOR_BGR2GRAY); //convert to gray scale
    GaussianBlur(img, img_Blr, Size(3,3),1,0); //Gaussian Blur
    Canny(img_Blr, img_Canny, 50, 150);

    Mat kernel = getStructuringElement(MORPH_RECT, Size(3,3));
    dilate(img_Canny, img_Di, kernel); //Kernel Definitio Required


    return img_Di;
}

int main() {

    string path = ".../doc.jpeg";
    imgOriginal = imread(path);

    //PreProcessing

    //resive to make image small, makes computationally efficient
    //resize(imgOriginal, imgOriginal, Size(),0.7,0.7);

    img_thres = preProcess(imgOriginal);

    //Get biggest contours(Biggesr Rectangle should be our document)
    initialpoints = getContours(img_thres);

    //Let's draw initial points, uncomment to check unordered points
    //drawPoints(initialpoints, Scalar(0,0,255));

    //reorder points correctly
    docPoints = reorder(initialpoints);

    //Draw reodered points
    //drawPoints(docPoints, Scalar(0,255,0));

    //Warp

    imgWarp = getWarp(imgOriginal, docPoints, w, h); //width and height of A4 paper


    //Crop Edges, make neat
    int crop_val = 5;
    Rect roi(crop_val,crop_val,w-(2*crop_val),h-(2*crop_val));
    imgCrop = imgWarp(roi);
    

    imshow("Image", imgOriginal);
    //imshow("Image Preprocessed", img_thres);
    //imshow("Doc", imgWarp);
    
    imshow("Doc Final", imgCrop);

    waitKey(0);
    return 0;
}
