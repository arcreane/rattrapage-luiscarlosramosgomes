#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Document Scanner  //

Mat imgOriginal, imgGray, imgBlur, imgCanny, imgThre, imgDil, imgErode, imgWarp, imgCrop, imgSharp;
vector<Point> initialPoints, docPoints;
float w = 420, h = 596;


//Process image

Mat preProcessing(Mat img)
{
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgCanny, 25, 75);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernel);
	
	return imgDil;
}

//Get the countours of the document

vector<Point> getContours(Mat image) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	vector<Point> biggest;
	int maxArea = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		

		string objectType;

		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

			if (area > maxArea && conPoly[i].size() == 4) {

				
				biggest = { conPoly[i][0],conPoly[i][1] ,conPoly[i][2] ,conPoly[i][3] };
				maxArea = area;
			}
			
		} 
	}
	return biggest;
}

//Draw 4 points of documents

void drawPoints(vector<Point> points, Scalar color)
{
	for (int i = 0; i < points.size(); i++)
	{
		circle(imgOriginal, points[i], 10, color, FILLED);
		putText(imgOriginal, to_string(i), points[i], FONT_HERSHEY_PLAIN, 4, color, 4);
	}
}

//Reorder 4 points

vector<Point> reorder(vector<Point> points)
{
	vector<Point> newPoints;
	vector<int>  sumPoints, subPoints;

	for (int i = 0; i < 4; i++)
	{
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}

	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //0
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //1
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //2
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //3

	return newPoints;
}

// Warp the perspective of the document

Mat getWarp(Mat img, vector<Point> points, float w, float h)
{
	Point2f src[4] = { points[0],points[1],points[2],points[3] };
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));

	return imgWarp;
}

////CODE FOR IMAGES
void main() {

	string path = "Resources/paper3.jpg";
	imgOriginal = imread(path);
	

	// Image processing
	imgThre = preProcessing(imgOriginal);

	// Get Contours
	initialPoints = getContours(imgThre);
	//Reorder points
	docPoints = reorder(initialPoints);
	//drawPoints(docPoints, Scalar(0, 255, 0));

	// Warp 
	imgWarp = getWarp(imgOriginal, docPoints, w, h);

	//Crop 
	int cropVal = 5;
	Rect roi(cropVal, cropVal, w - (2 * cropVal), h - (2 * cropVal));
	imgCrop = imgWarp(roi);

	//Sharpen (2 methods)

	//Using Kernel
	/*Mat sharpening_kernel = (Mat_<double>(3, 3) << -1, -1, -1,
		-1, 9, -1,
		-1, -1, -1);
	filter2D(imgCrop, imgSharp, -1, sharpening_kernel);*/

	//Subtract smoothed image from original
	double sigma = 1, amount = 1;
	Mat blurry, sharp;
	GaussianBlur(imgCrop, blurry, Size(), sigma);
	addWeighted(imgCrop, 1 + amount, blurry, -amount, 0, sharp);




	imshow("Image", imgOriginal);
	imshow("Image Sharp", sharp);


	int k;
	while (true)
	{
		k = waitKey(20) & 0xFF;
		
		
		if (k == 115) {												//if 's' is pressed, save image on disk
			imwrite("output.jpg", sharp);
			cout << "Saved the output image on disk!" << endl;
		}
		else if (k == 27) {											//if ESC is pressed close all windows
	
			destroyAllWindows();
		
}
	}
//CODE FOR WEBCAM


	//void main() {
	//	VideoCapture cap(0);
	//	

	//	while (true) {

	//		cap.read(imgOriginal);
	//		// Preprocessing
	//		imgThre = preProcessing(imgOriginal);

	//		// Get Contours 
	//		initialPoints = getContours(imgThre);

	//		/////drawPoints(initialPoints, Scalar(0,0,255));
	//		//docPoints = reorder(initialPoints);
	//		////drawPoints(docPoints, Scalar(0, 255, 0));

	//		//// Warp
	//		//imgWarp = getWarp(imgOriginal, docPoints, w, h);

	//		////Crop
	//		//int cropVal = 5;
	//		//Rect roi(cropVal, cropVal, w - (2 * cropVal), h - (2 * cropVal));
	//		//imgCrop = imgWarp(roi);

	//		imshow("Image", imgOriginal);
	//		//show("Image Crop", imgCrop);
	//		waitKey(1);
	//	
	//
	
}
	