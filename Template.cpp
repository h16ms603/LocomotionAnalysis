
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

// ����ۑ����[�h
//#define VIDEO

// ����f�[�^
#define   WIDTH     1280     // �t���[���̉���
#define   HEIGHT     720     // �t���[���̏c��
#define   FPS         30     // �t���[�����[�g

// �摜�����p�����[�^
#define   THRESHOLD_BINARY    80     // ��l����臒l
#define   THRESHOLD_MAX      255     // ��l���̍ő�l
#define   ITERATION_DILATE     1     // �c�������̉�
#define   ITERATION_ERODE      1     // ���k�����̉�

// �ۑ��f�[�^
#define   NUMBER_DATA   2000    // �f�[�^�̐�
#define   NUMBER_ITEM      4    // ���ڂ̐�

using namespace cv;
using namespace std;

int loopCount;          // �J�E���^

cv::Mat    currentImage;     // ���݉摜
cv::Mat backgroundImage;     // �w�i�摜
cv::Mat differenceImage;     // �����摜
cv::Mat    extractImage;     // ���o�摜

int xg, yg;           // �ώ@�Ώۂ̈ʒu
double direction;     // �ώ@�Ώۂ̎p��

double data[NUMBER_DATA][NUMBER_ITEM];     // �ۑ��f�[�^

// �̈撊�o
cv::Mat iExtract( Mat source, Mat background ) {

	Mat extract( source.size(), CV_8UC1 );
	Mat labelArray( source.size(), CV_16SC1 );
	LabelingBS imageLabel;

	// �O���[�X�P�[����
	cvtColor( source, source, CV_RGB2GRAY );
	cvtColor( background, background, CV_RGB2GRAY );

	// �w�i����
	cv::absdiff( source, background, differenceImage );

	// ��l��
	cv::threshold( differenceImage, extract, THRESHOLD_BINARY, THRESHOLD_MAX, CV_THRESH_BINARY );

	// �c�����k����
	dilate( extract, extract, cv::Mat(), cv::Point(-1, -1), ITERATION_DILATE );
	erode( extract, extract, cv::Mat(), cv::Point(-1, -1), ITERATION_ERODE );

	// �ő�A���v�f�擾
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

// �ʒu�E�p������
void iEstimate( Mat source ) {

	double m00, m01, m10;
	double mu02, mu11, mu20;
	Moments imageMoment;

	// ���[�����g�擾
	imageMoment = moments( source );
	m00 = imageMoment.m00;
	m01 = imageMoment.m01;
	m10 = imageMoment.m10;
	mu02 = imageMoment.mu02;
	mu11 = imageMoment.mu11;
	mu20 = imageMoment.mu20;

	// �d�S�̈ʒu
	xg = (int)(m10 / m00);
	yg = (int)(m01 / m00);
	if ( xg < 0 && yg < 0 ) {
		xg = -1;
		yg = -1;
	}

	// �����厲�̌X��
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

// �f�[�^�ۑ�
void iSaveData( string name ) {

	int k = 0;
	stringstream fname;
	ofstream file;
	ifstream ifile;

	// �t�@�C���̒T��
	while ( true ) {
		std::ifstream ifile;
		fname.str("");
		fname << name << k << ".txt";
		ifile.open( fname.str(), std::ios::in );
		if ( !ifile.is_open() ) {
			ifile.close();
			break;
		}
		k += 1;
	}

	// �t�@�C���̃I�[�v��
	file.open( fname.str(), ios::out );

	// �f�[�^�̕ۑ�
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
		cerr << " Error : �o�̓t�@�C�����J���܂���D" << endl;
		cerr << " Output file : " << fname.str() << endl;
		cout << endl;
	}

	// �t�@�C���̃N���[�Y
	file.close();
}

/***************************************************************************************************/
int main(void) {

	bool playFlag;

	Mat sourceImage;
	Mat destinationImage;
	Mat leftShowImage;
	Mat rightShowImage;

	// �R���\�[���̐ݒ�
	char consoleName[] = "Behavior Measurement";
	SetConsoleTitleA( consoleName );
	Sleep(20);
	MoveWindow( FindWindowA(NULL, consoleName), 0, 0, 640, 480, true );
	cout << endl;

	// �E�B���h�E�̐ݒ�
	string windowName = "Left:Result Image, Right:Extracted Image";
	namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	moveWindow( windowName, 0, 320 );

	// ���͓���̗p��
	VideoCapture capture( "Ant.mp4" );
	if ( capture.isOpened() ) {
		playFlag = true;
		capture.set( CV_CAP_PROP_FRAME_WIDTH, WIDTH );
		capture.set( CV_CAP_PROP_FRAME_HEIGHT, HEIGHT );
		capture.set( CV_CAP_PROP_FPS, FPS );
	}
	else {
		playFlag = false;
		cerr << " Error : ���͓��悪�p�ӂ���Ă��܂���D" << endl;
		cout << endl;
	}
#ifdef VIDEO
	// �ۑ�����̗p��
	VideoWriter writerCurrent( "Current.avi", CV_FOURCC('X', 'V', 'I', 'D'), FPS, cv::Size(WIDTH, HEIGHT) );
	VideoWriter writerExtract( "Extract.avi", CV_FOURCC('X', 'V', 'I', 'D'), FPS, cv::Size(WIDTH, HEIGHT) );
	if ( !writerResult.isOpened() || !writerExtract.isOpened() ) {
		cerr << " Error : ����̕ۑ����ł��܂���D" << endl;
		cout << endl;
	}
#endif
	// �R���\�[���̕\��
	cout << " > ���͓���ɑ΂��Ċώ@�Ώۂ̈ʒu�E�p����������s��..." << endl;
	cout << endl;
	cout << " [n] �Đ�" << endl;
	cout << " [b] ��~" << endl;
	cout << " [r] �͂��߂���Đ�" << endl;
	cout << " [q] �I��" << endl;
	cout << endl;

	// �w�i�摜�̎擾
	capture >> sourceImage;
	sourceImage.copyTo( backgroundImage );

	// �J�E���^�̃��Z�b�g
	loopCount = 0;
	
	/* ���C�����[�v */
	while ( true ) {

		// ���͉摜�̎擾
		if ( playFlag == true ) {
			capture >> sourceImage;
			if ( sourceImage.empty() ) {
				break;
			}
		}
		sourceImage.copyTo( currentImage );

		// �̈�̒��o
		extractImage = iExtract( currentImage, backgroundImage );

		// �ʒu�E�p���̐���
		iEstimate( extractImage );

		// �f�[�^�̎擾
		double time = capture.get(CV_CAP_PROP_POS_MSEC) * 0.001;
		data[loopCount][0] = time;
		data[loopCount][1] = xg;
		data[loopCount][2] = yg;
		data[loopCount][3] = direction;
		loopCount += 1;

		// �\���摜�̐���
		currentImage.copyTo( leftShowImage );
		currentImage.copyTo( rightShowImage );
		for ( int y = 0; y < rightShowImage.rows; y++ ) {
			for ( int x = 0; x < rightShowImage.cols; x++ ) {
				if ( extractImage.at<unsigned char>(y, x) > 0 ) {
					rightShowImage.at<cv::Vec3b>(y, x)[0] = 255;
					rightShowImage.at<cv::Vec3b>(y, x)[1] = 255;
					rightShowImage.at<cv::Vec3b>(y, x)[2] = 255;
				}
				else {
					rightShowImage.at<cv::Vec3b>(y, x)[0] = 0;
					rightShowImage.at<cv::Vec3b>(y, x)[1] = 0;
					rightShowImage.at<cv::Vec3b>(y, x)[2] = 0;
				}
			}
		}
		
		// ���̕`��E�_�̕`��
		if ( xg != -1 && yg != -1 ) {
			int  x1, x2;
			x1 = (int)(xg - yg * tan(direction));
			x2 = (int)(xg + (HEIGHT - yg) * tan(direction));
			line( rightShowImage, Point(x1, 0), Point(x2, HEIGHT), CV_RGB(0, 0, 255), 2, 8, 0 );
			circle( rightShowImage, Point(xg, yg), 4, CV_RGB(255, 0, 0), -1, 8, 0 );
		}

		// �\���摜�̌���
		hconcat( leftShowImage, rightShowImage, destinationImage );

		// �o�͉摜�̏k��
		resize( destinationImage, destinationImage, Size(0, 0), 0.5, 0.5 );
#ifdef VIDEO
		// �ۑ�����̎擾
		writerCurrent << leftShowImage;
		writerExtract << rightShowImage;
#endif
		// �R���\�[���̕\��
		cout << " > �Đ����ԁF" << fixed << setprecision(2) << time << "\r" << flush;
		
		// �o�͉摜�̕\��
		imshow( windowName, destinationImage );

		// �L�[�̓���
		int key = cv::waitKey( 1 );
		switch ( key ) {
		case 'n':     // ����̍Đ�
			playFlag = true;
			break;
		case 'b':     // ����̒�~
			playFlag = false;
			break;
		case 'r':     // ����̂͂��߂���Đ�
			playFlag = true;
			loopCount = 0;
			capture.set( CV_CAP_PROP_POS_AVI_RATIO, 0 );
			break;
		case 'q':     // �I��
			return 0;
			break;
		default:
			break;
		}
	}

	// �R���\�[���̕\��
	cout << " > �擾�����f�[�^��ۑ����܂����D" << endl;
	cout << endl;
	cout << " [y] �͂�" << endl;
	cout << " [n] ������" << endl;
	cout << endl;
	
	/* �f�[�^�ۑ��̑I�� */
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