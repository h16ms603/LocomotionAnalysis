
#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "Labeling.h"

//#define VIDEO

#define   WIDTH     1280
#define   HEIGHT     720
#define   FPS         30

#define   THRESHOLD_BINARY    80
#define   THRESHOLD_MAX      255
#define   ITERATION_DILATE     1
#define   ITERATION_ERODE      1

#define   NUMBER_DATA   2000
#define   NUMBER_ITEM      4

using namespace cv;
using namespace std;

int loopCount;          

cv::Mat    currentImage;
cv::Mat backgroundImage;
cv::Mat differenceImage;
cv::Mat    extractImage;

int xg, yg;     
double direction;

double data[NUMBER_DATA][NUMBER_ITEM];

cv::Mat iExtract( Mat source, Mat background ) {

	Mat extract( source.size(), CV_8UC1 );
	Mat labelArray( source.size(), CV_16SC1 );
	LabelingBS imageLabel;

	cvtColor( source, source, CV_RGB2GRAY );
	cvtColor( background, background, CV_RGB2GRAY );

	cv::absdiff( source, background, differenceImage );

	cv::threshold( differenceImage, extract, THRESHOLD_BINARY, THRESHOLD_MAX, CV_THRESH_BINARY );

	dilate( extract, extract, cv::Mat(), cv::Point(-1, -1), ITERATION_DILATE );
	erode( extract, extract, cv::Mat(), cv::Point(-1, -1), ITERATION_ERODE );

	imageLabel.Exec( extract.data, (short *)labelArray.data, source.rows, source.cols, true, 0 );
	for ( int y = 0; y < labelArray.rows; y++ ) {
		for ( int x = 0; x < labelArray.cols; x++ ) {
			if ( labelArray.at<short>(y, x) > 0 ) {
				extract.at<unsigned char>(y, x) = 255;
			}
			else {
				extract.at<unsigned char>(y, x) = 0;
			}
		}
	}

	return extract;
}

void iEstimate( Mat source ) {

	double m00, m01, m10;
	double mu02, mu11, mu20;
	Moments imageMoment;

	imageMoment = moments( source );
	m00 = imageMoment.m00;
	m01 = imageMoment.m01;
	m10 = imageMoment.m10;
	mu02 = imageMoment.mu02;
	mu11 = imageMoment.mu11;
	mu20 = imageMoment.mu20;

	xg = (int)(m10 / m00);
	yg = (int)(m01 / m00);
	if ( xg < 0 && yg < 0 ) {
		xg = -1;
		yg = -1;
	}

	direction = 0.5 * atan2(2 * mu11, (mu20 - mu02));
	if ( direction > 0 ) {
		direction = (CV_PI / 2) - direction;
	}
	else if ( direction < 0 ) {
		direction = (CV_PI / 2) + direction;
		direction *= -1;
	}
	else {
		direction = 0;
	}
}

void iSaveData( string name ) {

	int k = 0;
	stringstream fname;
	ofstream file;
	ifstream ifile;

	while (true) {
		std::ifstream ifile;
		fname.str("");
		fname << name << k << ".txt";
		ifile.open(fname.str(), std::ios::in);
		if (!ifile.is_open()) {
			ifile.close();
			break;
		}
		k += 1;
	}

	file.open( fname.str(), ios::out );

	if ( file.is_open() ) {
		for ( int count = 0; count < loopCount; count++ ) {
			file << count << "\t";
			file << fixed << setprecision(2) << data[count][0] << "\t";
			file << (int)data[count][1] << "\t" << (int)data[count][2] << "\t";
			file << fixed << setprecision(2) << data[count][3];
			file << endl;
		}
	}
	else {
		cerr << " Error : Do not open output file." << endl;
		cerr << " Output file : " << fname.str() << endl;
		cout << endl;
	}

	file.close();
}

/***************************************************************************************************/
int main(void) {

	bool playFlag;

	double time;

	Mat sourceImage;
	Mat destinationImage;
	Mat leftImage;
	Mat rightImage;

	char consoleName[] = "Behavior Measurement";
	SetConsoleTitleA( consoleName );
	Sleep(20);
	MoveWindow( FindWindowA(NULL, consoleName), 0, 0, 640, 480, true );
	cout << endl;

	string windowName = "Left:Result Image, Right:Extracted Image";
	namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	moveWindow( windowName, 0, 320 );

	VideoCapture capture( "Ant.mp4" );
	if ( capture.isOpened() ) {
		playFlag = true;
		capture.set( CV_CAP_PROP_FRAME_WIDTH, WIDTH );
		capture.set( CV_CAP_PROP_FRAME_HEIGHT, HEIGHT );
		capture.set( CV_CAP_PROP_FPS, FPS );
	}
	else {
		cerr << " Error : Do not have the input video." << endl;
		cout << endl;
	}
#ifdef VIDEO
	VideoWriter writerCurrent( "Current.avi", CV_FOURCC('X', 'V', 'I', 'D'), FPS, cv::Size(WIDTH, HEIGHT) );
	VideoWriter writerExtract( "Extract.avi", CV_FOURCC('X', 'V', 'I', 'D'), FPS, cv::Size(WIDTH, HEIGHT) );
	if ( !writerCurrent.isOpened() || !writerExtract.isOpened() ) {
		cerr << " Error : Can not save the video." << endl;
		cout << endl;
	}
#endif

	cout << " > Estimating position and attitube of observed target in input video." << endl;
	cout << endl;
	cout << " [n] Play" << endl;
	cout << " [b] Stop" << endl;
	cout << " [r] Replay" << endl;
	cout << " [q] End" << endl;
	cout << endl;

	capture >> sourceImage;
	sourceImage.copyTo( backgroundImage );

	loopCount = 0;
	
	while ( true ) {

		if ( playFlag == true ) {
			capture >> sourceImage;
			if ( sourceImage.empty() ) {
				break;
			}
		}
		sourceImage.copyTo( currentImage );

		extractImage = iExtract( currentImage, backgroundImage );

		iEstimate( extractImage );

		double time = capture.get(CV_CAP_PROP_POS_MSEC) * 0.001;
		data[loopCount][0] = time;
		data[loopCount][1] = xg;
		data[loopCount][2] = yg;
		data[loopCount][3] = direction;
		loopCount += 1;

		currentImage.copyTo( leftImage );
		currentImage.copyTo( rightImage );
		for ( int y = 0; y < currentImage.rows; y++ ) {
			for ( int x = 0; x < currentImage.cols; x++ ) {
				if ( extractImage.at<unsigned char>(y, x) > 0 ) {
					rightImage.at<cv::Vec3b>(y, x)[0] = 255;
					rightImage.at<cv::Vec3b>(y, x)[1] = 255;
					rightImage.at<cv::Vec3b>(y, x)[2] = 255;
				}
				else {
					rightImage.at<cv::Vec3b>(y, x)[0] = 0;
					rightImage.at<cv::Vec3b>(y, x)[1] = 0;
					rightImage.at<cv::Vec3b>(y, x)[2] = 0;
				}
			}
		}
		
		if ( xg != -1 && yg != -1 ) {
			int  x1, x2;
			x1 = (int)(xg - yg * tan(direction));
			x2 = (int)(xg + (HEIGHT - yg) * tan(direction));
			line( rightImage, Point(x1, 0), Point(x2, HEIGHT), CV_RGB(0, 0, 255), 2, 8, 0 );
			circle( rightImage, Point(xg, yg), 4, CV_RGB(255, 0, 0), -1, 8, 0 );
		}
#ifdef VIDEO
		writerCurrent << leftImage;
		writerExtract << rightImage;
#endif
		hconcat( leftImage, rightImage, destinationImage );

		resize( destinationImage, destinationImage, Size(0, 0), 0.5, 0.5 );

		cout << " > Play time : " << fixed << setprecision(2) << time << "\r" << flush;
		
		imshow( windowName, destinationImage );

		int key = cv::waitKey(1);
		switch (key) {
		case 'n':
			playFlag = true;
			break;
		case 'b':
			playFlag = false;
			break;
		case 'r':
			playFlag = true;
			loopCount = 0;
			capture.set( CV_CAP_PROP_POS_AVI_RATIO, 0 );
			break;
		case 'q':
			return 0;
			break;
		default:
			break;
		}
	}

	cout << " > Do you save the get data?" << endl;
	cout << endl;
	cout << " [y] Yes" << endl;
	cout << " [n] No" << endl;
	cout << endl;
	
	while ( true ) {
		int key = cv::waitKey(1);
		if ( key == 'y' || key == 'n' ) {
			if ( key == 'y' ) {
				iSaveData( "data" );
			}
			return 0;
		}
	}
}
/***************************************************************************************************/