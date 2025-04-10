#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <opencv2/core/utils/logger.hpp>
 
using namespace std;
using namespace cv;
 
static void help(char** argv)
{
    // print a welcome message, and the OpenCV version
    cout << "\nThis is a demo program shows how perspective transformation applied on an image, \n"
         "Using OpenCV version " << CV_VERSION << endl;
 
    cout << "\nUsage:\n" << argv[0] << " [image_name -- Default right.jpg]\n" << endl;
 
    cout << "\nHot keys: \n"
         "\tESC, q - quit the program\n"
         "\tr - change order of points to rotate transformation\n"
         "\tc - delete selected points\n"
         "\ti - change order of points to inverse transformation \n"
         "\nUse your mouse to select a point and move it to see transformation changes" << endl;
}
 
static void onMouse(int event, int x, int y, int, void*);
Mat warping(Mat image, Size warped_image_size, vector< Point2f> srcPoints, vector< Point2f> dstPoints);
 
String windowTitle = "Perspective Transformation Demo";
String labels[4] = { "TL","TR","BR","BL" };
vector< Point2f> roi_corners;
vector< Point2f> midpoints(4);
vector< Point2f> dst_corners(4);
int roiIndex = 0;
bool dragging;
int selected_corner_index = 0;
bool validation_needed = true;
Mat image;
std::vector<cv::Point2f> transformed_points;
std::vector<cv::Point2f> g_po ;
float original_image_cols ;
float original_image_rows ;
int g_mx, g_my;

int main(int argc, char** argv)
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR); //只打印错误信息
    help(argv);
    CommandLineParser parser(argc, argv, "{@input| ../pic7174.png |}");
 
    string filename = samples::findFile(parser.get<string>("@input"));
    Mat original_image = imread( filename );

 
    original_image_cols = (float)original_image.cols;
    original_image_rows = (float)original_image.rows;

    cout << "width:" << original_image_cols << " height:" << original_image_rows << endl;

    //roi_corners.push_back(Point2f( (float)(original_image_cols / 1.70), (float)(original_image_rows / 4.20) ));
    //roi_corners.push_back(Point2f( (float)(original_image.cols / 1.15), (float)(original_image.rows / 3.32) ));
    //roi_corners.push_back(Point2f( (float)(original_image.cols / 1.33), (float)(original_image.rows / 1.10) ));
    //roi_corners.push_back(Point2f( (float)(original_image.cols / 1.93), (float)(original_image.rows / 1.36) ));

    //roi_corners.push_back(Point2f( (float)(427), (float)(416) ));
    //roi_corners.push_back(Point2f( (float)(662), (float)(416) ));

    //roi_corners.push_back(Point2f( (float)(1072), (float)(665) ));
    //roi_corners.push_back(Point2f( (float)(0), (float)(665) ));

   /* roi_corners.push_back(Point2f((float)(241), (float)(510)));
    roi_corners.push_back(Point2f((float)(1002), (float)(510)));

    roi_corners.push_back(Point2f((float)(1281), (float)(719)));
    roi_corners.push_back(Point2f((float)(0), (float)(719)));*/

    roi_corners.push_back(Point2f((float)(420), (float)(602)));
    roi_corners.push_back(Point2f((float)(948), (float)(602)));
    roi_corners.push_back(Point2f((float)(original_image.cols -1), (float)(original_image.rows -1)));
    roi_corners.push_back(Point2f((float)(0), (float)(original_image.rows - 1)));

 
    namedWindow(windowTitle, WINDOW_NORMAL);
    namedWindow("Warped Image", WINDOW_AUTOSIZE);
    moveWindow("Warped Image", 20, 20);
    moveWindow(windowTitle, 330, 20);
 
    transformed_points.push_back(Point2f(0, 0));
    transformed_points.push_back(Point2f(0, 0));

    setMouseCallback(windowTitle, onMouse, 0);
 
    bool endProgram = false;
    while (!endProgram)
    {
        if ( validation_needed & (roi_corners.size() < 4) )
        {
            validation_needed = false;
            image = original_image.clone();
 
            for (size_t i = 0; i < roi_corners.size(); ++i)
            {
                circle( image, roi_corners[i], 5, Scalar(0, 255, 0), 3 );
 
                if( i > 0 )
                {
                    line(image, roi_corners[i-1], roi_corners[(i)], Scalar(0, 0, 255), 2);
                    circle(image, roi_corners[i], 5, Scalar(0, 255, 0), 3);
                    putText(image, labels[i].c_str(), roi_corners[i], FONT_ITALIC, 0.8, Scalar(255, 0, 0), 1);
                }
            }
            imshow( windowTitle, image );
        }
 
        if ( validation_needed & ( roi_corners.size() == 4 ))
        {
            image = original_image.clone();
            for ( int i = 0; i < 4; ++i )
            {
                line(image, roi_corners[i], roi_corners[(i + 1) % 4], Scalar(0, 0, 255), 2);
                circle(image, roi_corners[i], 5, Scalar(0, 255, 0), 3);
                putText(image, labels[i].c_str(), roi_corners[i], FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 0, 0), 2);
            }
 
            imshow( windowTitle, image );
 
            midpoints[0] = (roi_corners[0] + roi_corners[1]) / 2;
            midpoints[1] = (roi_corners[1] + roi_corners[2]) / 2;
            midpoints[2] = (roi_corners[2] + roi_corners[3]) / 2;
            midpoints[3] = (roi_corners[3] + roi_corners[0]) / 2;
 
            dst_corners[0].x = 0;
            dst_corners[0].y = 0;
            dst_corners[1].x = (float)norm(midpoints[1] - midpoints[3]);
            dst_corners[1].y = 0;
            dst_corners[2].x = dst_corners[1].x;
            dst_corners[2].y = (float)norm(midpoints[0] - midpoints[2]);
            dst_corners[3].x = 0;
            dst_corners[3].y = dst_corners[2].y;
 
           // Size warped_image_size = Size(cvRound(dst_corners[2].x), cvRound(dst_corners[2].y));
            Size warped_image_size = Size(original_image.cols ,original_image.rows);

            Mat M = getPerspectiveTransform(roi_corners, dst_corners);
 
            Mat warped_image;
            warpPerspective(original_image, warped_image, M, warped_image_size); // do perspective transformation
 
            putText(image, "("+to_string((int)(transformed_points[1].x - transformed_points[0].x)) + ":" + to_string((int)(-(transformed_points[1].y - transformed_points[0].y)))+")", cv::Point2i(g_mx, g_my), FONT_ITALIC, 1, Scalar(255, 0, 0), 2);
            imshow(windowTitle, image);

            imshow("Warped Image", warped_image);
        }
 
        char c = (char)waitKey( 10 );
 
        if ((c == 'q') | (c == 'Q') | (c == 27))
        {
            endProgram = true;
        }
 
        if ((c == 'c') | (c == 'C'))
        {
            roi_corners.clear();
        }
 
        if ((c == 'r') | (c == 'R'))
        {
            roi_corners.push_back(roi_corners[0]);
            roi_corners.erase(roi_corners.begin());
        }
 
        if ((c == 'i') | (c == 'I'))
        {
            swap(roi_corners[0], roi_corners[1]);
            swap(roi_corners[2], roi_corners[3]);
        }
        if (cv::waitKey(1) == 27) // Wait for 'esc' key press to exit
        {
            break;
        }

    }
    return 0;
}
 
static void onMouse(int event, int x, int y, int, void*)
{
    // Action when left button is pressed
    if (roi_corners.size() == 4)
    {
        for (int i = 0; i < 4; ++i)
        {
            if ((event == EVENT_LBUTTONDOWN) && ((abs(roi_corners[i].x - x) < 10)) && (abs(roi_corners[i].y - y) < 10))
            {
                selected_corner_index = i;
                dragging = true;
            }
        }

        cout << x <<":" <<y << endl;
        g_mx = x; g_my = y;
        transformed_points.clear();
        g_po.push_back(Point2f((float)original_image_cols/2.0, (float)original_image_rows));
        g_po.push_back(Point2f((float)x, (float)y));
        Mat M = getPerspectiveTransform(roi_corners, dst_corners);
     
        //画点
        perspectiveTransform(g_po, transformed_points, M);
        cout << g_mx << ":" << g_my << endl;
        cout << transformed_points[1].x << ":" << transformed_points[1].y << endl;
        cout << transformed_points[1].x - transformed_points[0].x << ":" <<-(transformed_points[1].y- transformed_points[0].y )<< endl;
        
        cout <<endl;
        g_po.clear();
        //transformed_points.clear();
    }
    else if ( event == EVENT_LBUTTONDOWN )
    {
        roi_corners.push_back( Point2f( (float) x, (float) y ) );
        validation_needed = true;
    }
 
    // Action when left button is released
    if (event == EVENT_LBUTTONUP)
    {
        dragging = false;
    }
 
    // Action when left button is pressed and mouse has moved over the window
    if ((event == EVENT_MOUSEMOVE) && dragging)
    {
        roi_corners[selected_corner_index].x = (float) x;
        roi_corners[selected_corner_index].y = (float) y;
        validation_needed = true;
    }
}