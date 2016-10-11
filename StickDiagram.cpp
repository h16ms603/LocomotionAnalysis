
#include <windows.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "file.h"
#include "calculate.h"

// 自動表示モード
#define AUTO

// 動画保存モード
//#define VIDEO

/******************************************************************************/
int main( void ) {
	
	bool showFlag;

	std::stringstream fileName;

	cv::Mat sourceImage;
	cv::Mat destinationImage;
	cv::Mat stickImage;
	
	// コンソールの設定
	char consoleName[] = "Stick Diagram";
	SetConsoleTitleA( consoleName );
	Sleep( 20 );
	MoveWindow( FindWindowA(NULL, consoleName), 0, 0, 640, 480, true );
	std::cout << std::endl;

	// ウィンドウの設定
	std::string windowName = "Source Image";
	cv::namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	cv::moveWindow( windowName, 650, 0 );
#ifdef VIDEO
	// 出力動画のオープン
	cv::VideoWriter writer.open("StickDiagram.avi", CV_FOURCC('X', 'V', 'I', 'D'), RATE_FRAME, cv::Size(2 * WIDTH, HEIGHT));
	if ( !writer.isOpened() ) {
		std::cerr << " Error : 動画の保存ができません．" << std::endl;
	}
#endif
	// データの読み込み
	iReadCoordinate();
	iReadBody();

	// コンソールの表示
	std::cout << " > " << "ファイル：" << " data" << fileNumber << ".txt" << " から座標データを取得．" << "\n";
	std::cout << " > " << "フォルダ：" << NAME_FOLDER << " から身体データを取得．" << "\n";
	std::cout << std::endl;

	// データの探索
	imageCount = 0;
	fileName.str("");
	fileName << std::setw(3) << std::setfill('0') << imageCount << ".bmp";
	sourceImage = cv::imread( fileName.str() );
	while ( sourceImage.empty() ) {
		imageCount += 1;
		fileName.str("");
		fileName << std::setw(3) << std::setfill('0') << imageCount << ".bmp";
		sourceImage = cv::imread( fileName.str() );
	}
	beginCount = imageCount;

	while ( !sourceImage.empty() ) {
		imageCount += 1;
		fileName.str("");
		fileName << std::setw(3) << std::setfill('0') << imageCount << ".bmp";
		sourceImage = cv::imread( fileName.str() );
	}
	endCount = imageCount - 1;

	// コンソールの表示
	std::cout << " > バッタの踵が地面から離れたときの画像を選択してください．" << "\n";
	std::cout << std::endl;
	std::cout << " [←] 前画像の表示" << "\t" << " [→] 次画像の表示" << "\n";
	std::cout << " [Enter] 画像の決定" << "\t" << " [Esc] 強制終了" << "\n";
	std::cout << std::endl;

	// カウンタのリセット
	imageCount = beginCount;
	showFlag = true;

	/* 離陸画像の選択 */
	while ( showFlag ) {

		// 画像の取得
		fileName.str("");
		fileName << std::setw(3) << std::setfill('0') << imageCount << ".bmp";
		sourceImage = cv::imread( fileName.str() );

		// コンソールの表示
		std::cout << " > 画像番号：" << imageCount << "\r" << std::flush;

		// 画像の表示
		cv::imshow( windowName, sourceImage );

		// キーの入力
		int key = cv::waitKey(1);
		switch ( key ) {
		case 2424832:     // 左方向キーで前へ
			if ( imageCount > beginCount ) {
				imageCount -= 1;
			}
			else {
				imageCount = beginCount;
			}
			break;
		case 2555904:     // 右方向キーで次へ
			if ( imageCount < endCount ) {
				imageCount += 1;
			}
			else {
				imageCount = endCount;
			}
			break;
		case 13:     // Enterキーで決定
			showFlag = false;
			liftoffCount = imageCount;
			break;
		case 27:     // Escキーで終了
			return 0;
		default:
			break;
		}
	}

	// コンソールの表示
	std::cout << " > " << liftoffCount << " 枚目を離陸したときの画像に決定．" << "\n";
	std::cout << std::endl;

	// ウィンドウの破棄
	cv::destroyWindow( windowName );

	// 個体情報の取得
	iGetInformation( dataMass, dataLength );

	// リンク・ジョイントの取得
	for ( int count = beginCount; count <= endCount; count++ ) {
		iGetLinkJoint( count, dataX, dataY );
		for ( int i = 0; i < NUMBER_LINK; i++ ) {
			positionX[count] += (linkMass[i] * linkPositionX[count][i]) / mass;
			positionY[count] += (linkMass[i] * linkPositionY[count][i]) / mass;
		}
	}

	// 
	for (int count = beginCount; count <= endCount; count++) {
		iCalculate( count );
	}
	
	// ウィンドウの設定
	windowName = "Left:Source Image, Right:Stick Diagram";
	cv::namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	cv::moveWindow( windowName, 540, 0 );

	// コンソールの表示
	std::cout << " > " << beginCount << " 枚目 ～ " << endCount << " 枚目までの画像を表示" << "\n";
	std::cout << std::endl;
#ifdef AUTO
	std::cout << "----- 自動表示モード --------------------------------------------" << "\n";
	std::cout << std::endl;
#else
	std::cout << "----- 手動表示モード --------------------------------------------" << "\n";
	std::cout << std::endl;
	std::cout << " [←] 前画像の表示" << "\t" << " [→] 次画像の表示" << "\n";
#endif
	std::cout << " [Enter] データの保存" << "\t" << " [Esc] 強制終了" << "\n";
	std::cout << std::endl;

	// カウンタのリセット
	showFlag = true;
	imageCount = beginCount;

	/* メインループ */
	while ( showFlag ) {

		// 画像の取得
		fileName.str("");
		fileName << std::setw(3) << std::setfill('0') << imageCount << ".bmp";
		sourceImage = cv::imread( fileName.str() );
		sourceImage.copyTo( stickImage );
		for ( int y = 0; y < stickImage.rows; y++ ) {
			for ( int x = 0; x < stickImage.cols; x++ ) {
				stickImage.at<cv::Vec3b>(y, x)[0] = 255;
				stickImage.at<cv::Vec3b>(y, x)[1] = 255;
				stickImage.at<cv::Vec3b>(y, x)[2] = 255;
			}
		}
		
		// 線の描画
		if ( imageCount < liftoffCount ) {
			cv::line( stickImage, cv::Point(0, HEIGHT - (int)jointPointY[liftoffCount - 1][2]),
				cv::Point(WIDTH, HEIGHT - (int)jointPointY[liftoffCount - 1][2]), CV_RGB(200, 200, 200), 2, 8, 0 );
		}
		for ( int i = 0; i < NUMBER_LINK; i++ ) {
			cv::line( stickImage, cv::Point((int)linkPointX[imageCount][i], HEIGHT - (int)linkPointY[imageCount][i]), 
				cv::Point((int)linkPointX[imageCount][i + NUMBER_LINK], HEIGHT - (int)linkPointY[imageCount][i + NUMBER_LINK]), CV_RGB(0, 255, 0), 2, 8, 0 );
		}
		
		// 点の描画
		for ( int i = 0; i < NUMBER_JOINT; i++ ) {
			cv::circle( stickImage, cv::Point((int)jointPointX[imageCount][i], HEIGHT - (int)jointPointY[imageCount][i]), 5, CV_RGB(0, 0, 255), -1, 8, 0 );
		}
		for ( int i = 0; i < NUMBER_LINK; i++ ) {
			cv::circle( stickImage, cv::Point((int)linkPositionX[imageCount][i], HEIGHT - (int)linkPositionY[imageCount][i]), 4, CV_RGB(255, 0, 0), -1, 8, 0 );
		}
		cv::circle( stickImage, cv::Point((int)positionX[imageCount], HEIGHT - (int)positionY[imageCount]), 5, CV_RGB(255, 127, 0), -1, 8, 0 );

		// 画像の結合
		cv::hconcat( sourceImage, stickImage, destinationImage );
#ifdef VIDEO
		// 動画の保存
		writer << destinationImage;
#endif
		// コンソールの表示
		std::cout << " > 画像番号：" << imageCount << "\r" << std::flush;

		// 画像の表示
		cv::imshow( windowName, destinationImage );

		// キーの入力
		int key = cv::waitKey(1);
		switch ( key ) {
		case 2424832:     // 左方向キーで前へ
			if ( imageCount > beginCount ) {
				imageCount -= 1;
			}
			else {
				imageCount = beginCount;
			}
			break;
		case 2555904:     // 右方向キーで次へ
			if ( imageCount < endCount ) {
				imageCount += 1;
			}
			else {
				imageCount = endCount;
			}
			break;
		case 13:     // Enterキーで保存
			showFlag = false;
			break;
		case 27:     // Escキーで終了
			return 0;
			break;
#ifdef AUTO
		default:
			if ( imageCount < endCount ) {
				imageCount += 1;
			}
			else {
				imageCount = beginCount;
			}
			Sleep( 1000 / FPS );
			break;
#else
		default:
			break;
#endif
		}
	}

	// データの取得
	for ( int count = beginCount; count <= endCount; count++ ) {
		data[count][0] = xg[0][count];
		data[count][1] = zg[0][count];
		data[count][2] = Lg[count];
		if ( count < liftoffCount ) {
			data[count][3] = forceX[count];
			data[count][4] = forceZ[count];
		}
	}
	/**
	for ( int count = beginCount; count <= endCount; count++ ) {
		for ( int i = 0; i < NUMBER_LINK; i++ ) {
			data[count][0 + i] = linkPositionX[0][count][i];
			data[count][1 + i] = linkPositionY[0][count][i];
			if ( jointPointX[count][0] < jointPointX[count][1] ) {
				data[count][2 + i] = 180.0 - linkAngle[0][count][i] * (180.0 / (atan(1.0) * 4.0));
			}
			else {
				data[count][2 + i] = linkAngle[0][count][i] * (180.0 / (atan(1.0) * 4.0));
			}
		}
		data[count][9]  = xg[0][count];
		data[count][10] = yg[0][count];
	}
	**/
	// データの保存
	iSaveData();
	return 0;
} 
/******************************************************************************/