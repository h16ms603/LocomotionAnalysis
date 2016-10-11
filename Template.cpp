
#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Labeling.h"

// 動画保存モード
//#define VIDEO

// 動画データ
#define   WIDTH     1280     // フレームの横幅
#define   HEIGHT     720     // フレームの縦幅
#define   FPS         30     // フレームレート

// 画像処理パラメータ
#define   THRESHOLD_BINARY    80     // 二値化の閾値
#define   THRESHOLD_MAX      255     // 二値化の最大値
#define   ITERATION_DILATE     1     // 膨張処理の回数
#define   ITERATION_ERODE      1     // 収縮処理の回数

// 保存データ
#define   NUMBER_DATA   2000    // データの数
#define   NUMBER_ITEM      4    // 項目の数

using namespace cv;
using namespace std;

int loopCount;          // カウンタ

cv::Mat    currentImage;     // 現在画像
cv::Mat backgroundImage;     // 背景画像
cv::Mat differenceImage;     // 差分画像
cv::Mat    extractImage;     // 抽出画像

int xg, yg;           // 観察対象の位置
double direction;     // 観察対象の姿勢

double data[NUMBER_DATA][NUMBER_ITEM];     // 保存データ

// 領域抽出
cv::Mat iExtract( Mat source, Mat background ) {

	Mat extract( source.size(), CV_8UC1 );
	Mat labelArray( source.size(), CV_16SC1 );
	LabelingBS imageLabel;

	// グレースケール化
	cvtColor( source, source, CV_RGB2GRAY );
	cvtColor( background, background, CV_RGB2GRAY );

	// 背景差分
	cv::absdiff( source, background, differenceImage );

	// 二値化
	cv::threshold( differenceImage, extract, THRESHOLD_BINARY, THRESHOLD_MAX, CV_THRESH_BINARY );

	// 膨張収縮処理
	dilate( extract, extract, cv::Mat(), cv::Point(-1, -1), ITERATION_DILATE );
	erode( extract, extract, cv::Mat(), cv::Point(-1, -1), ITERATION_ERODE );

	// 最大連結要素取得
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

// 位置・姿勢推定
void iEstimate( Mat source ) {

	double m00, m01, m10;
	double mu02, mu11, mu20;
	Moments imageMoment;

	// モーメント取得
	imageMoment = moments( source );
	m00 = imageMoment.m00;
	m01 = imageMoment.m01;
	m10 = imageMoment.m10;
	mu02 = imageMoment.mu02;
	mu11 = imageMoment.mu11;
	mu20 = imageMoment.mu20;

	// 重心の位置
	xg = (int)(m10 / m00);
	yg = (int)(m01 / m00);
	if ( xg < 0 && yg < 0 ) {
		xg = -1;
		yg = -1;
	}

	// 慣性主軸の傾き
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

// データ保存
void iSaveData( string name ) {

	int k = 0;
	stringstream fname;
	ofstream file;
	ifstream ifile;

	// ファイルの探索
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

	// ファイルのオープン
	file.open( fname.str(), ios::out );

	// データの保存
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
		cerr << " Error : 出力ファイルが開けません．" << endl;
		cerr << " Output file : " << fname.str() << endl;
	}

	// ファイルのクローズ
	file.close();
}

/******************************************************************************/
int main(void) {

	bool playFlag;

	Mat sourceImage;
	Mat destinationImage;
	Mat leftShowImage;
	Mat rightShowImage;

	// コンソールの設定
	char consoleName[] = "Behavior Measurement";
	SetConsoleTitleA( consoleName );
	Sleep(20);
	MoveWindow( FindWindowA(NULL, consoleName), 0, 0, 640, 480, true );
	cout << endl;

	// ウィンドウの設定
	string windowName = "Left:Result Image, Right:Extracted Image";
	namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	moveWindow( windowName, 0, 320 );

	// 入力動画の用意
	VideoCapture capture( "Ant.mp4" );
	if ( capture.isOpened() ) {
		playFlag = true;
		capture.set( CV_CAP_PROP_FRAME_WIDTH, WIDTH );
		capture.set( CV_CAP_PROP_FRAME_HEIGHT, HEIGHT );
		capture.set( CV_CAP_PROP_FPS, FPS );
	}
	else {
		playFlag = false;
		cerr << " Error : 入力動画が用意されていません．" << endl;
	}
#ifdef VIDEO
	// 保存動画の用意
	VideoWriter writerCurrent( "Current.avi", CV_FOURCC('X', 'V', 'I', 'D'), FPS, cv::Size(WIDTH, HEIGHT) );
	VideoWriter writerExtract( "Extract.avi", CV_FOURCC('X', 'V', 'I', 'D'), FPS, cv::Size(WIDTH, HEIGHT) );
	if ( !writerResult.isOpened() || !writerExtract.isOpened() ) {
		cerr << " Error : 動画の保存ができません．" << endl;
	}
#endif
	// コンソールの表示
	cout << " > 入力動画に対して観察対象の位置・姿勢推定を実行中..." << "\n";
	cout << endl;
	cout << " [n] 再生" << "\n";
	cout << " [b] 停止" << "\n";
	cout << " [r] はじめから再生" << "\n";
	cout << " [q] 終了" << "\n";
	cout << endl;

	// 背景画像の取得
	capture >> sourceImage;
	sourceImage.copyTo( backgroundImage );

	// カウンタのリセット
	loopCount = 0;
	
	/* メインループ */
	while ( true ) {

		// 入力画像の取得
		if ( playFlag == true ) {
			capture >> sourceImage;
			if ( sourceImage.empty() ) {
				break;
			}
		}
		sourceImage.copyTo( currentImage );

		// 領域の抽出
		extractImage = iExtract( currentImage, backgroundImage );

		// 位置・姿勢の推定
		iEstimate( extractImage );

		// データの取得
		double time = capture.get(CV_CAP_PROP_POS_MSEC) * 0.001;
		data[loopCount][0] = time;
		data[loopCount][1] = xg;
		data[loopCount][2] = yg;
		data[loopCount][3] = direction;
		loopCount += 1;

		// 表示画像の生成
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
		
		// 線の描画・点の描画
		if ( xg != -1 && yg != -1 ) {
			int  x1, x2;
			x1 = (int)(xg - yg * tan(direction));
			x2 = (int)(xg + (HEIGHT - yg) * tan(direction));
			line( rightShowImage, Point(x1, 0), Point(x2, HEIGHT), CV_RGB(0, 0, 255), 2, 8, 0 );
			circle( rightShowImage, Point(xg, yg), 4, CV_RGB(255, 0, 0), -1, 8, 0 );
		}

		// 表示画像の結合
		hconcat( leftShowImage, rightShowImage, destinationImage );

		// 出力画像の縮小
		resize( destinationImage, destinationImage, Size(0, 0), 0.5, 0.5 );
#ifdef VIDEO
		// 保存動画の取得
		writerCurrent << leftShowImage;
		writerExtract << rightShowImage;
#endif
		// コンソールの表示
		cout << " > 再生時間：" << fixed << setprecision(2) << time << "\r" << flush;
		
		// 出力画像の表示
		imshow( windowName, destinationImage );

		// キーの入力
		int key = cv::waitKey( 1 );
		switch ( key ) {
		case 'n':     // 動画の再生
			playFlag = true;
			break;
		case 'b':     // 動画の停止
			playFlag = false;
			break;
		case 'r':     // 動画のはじめから再生
			playFlag = true;
			loopCount = 0;
			capture.set( CV_CAP_PROP_POS_AVI_RATIO, 0 );
			break;
		case 'q':     // 終了
			return 0;
			break;
		default:
			break;
		}
	}

	// コンソールの表示
	cout << " > 取得したデータを保存しますか．" << "\n";
	cout << endl;
	cout << " [y] はい" << "\n";
	cout << " [n] いいえ" << "\n";
	cout << endl;
	
	/* データ保存の選択 */
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
/******************************************************************************/