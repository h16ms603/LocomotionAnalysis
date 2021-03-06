
#include <windows.h>
#include <iostream>
#include "config.h"
#include "file.h"

int main(void) {

	bool continueFlag;

	std::string windowName;

	cv::Mat sourceImage;
	cv::Mat destinationImage;

	// コンソールの設定
	char consoleName[] = "Initial Configuration";
	SetConsoleTitleA( consoleName );
	Sleep( 20 );
	MoveWindow( FindWindowA(NULL, consoleName), 0, 0, 640, 480, true );

	// ウィンドウの設定
	windowName = "Left:Enlarged Image, Right:Captured Image";
	cv::namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	cv::moveWindow( windowName, 0, 260 );

	// マウスの設定
	cv::setMouseCallback( windowName, iMouseCallBack );

	// コンソールの表示
	std::cout << std::endl;

	// カメラの設定
	cv::VideoCapture capture(0);
	if ( capture.isOpened() ) {
		capture.set( CV_CAP_PROP_FRAME_WIDTH, WIDTH );
		capture.set( CV_CAP_PROP_FRAME_HEIGHT, HEIGHT );
		capture.set( CV_CAP_PROP_FPS, FPS );
	}
	else {
		std::cerr << " Error : カメラが接続されていません．" << std::endl;
		return -1;
	}

	// コンソールの表示
	std::cout << " > 画像上から射影変換を行う領域を指定してください．" << "\n";
	std::cout << std::endl;
	std::cout << " [右クリック] or [Enter] 点の決定" << "\n";
	std::cout << " [方向キー]　 点の移動" << "\n";
	std::cout << " [左クリック] 拡大画像の表示" << "\n";
	std::cout << " [Esc]       強制終了" << "\n";
	std::cout << std::endl;

	// フラグのセット
	continueFlag = true;
	iNextStep();

	/* 領域設定 */
	while ( continueFlag ) {

		// 画像の取得
		capture >> sourceImage;

		// 点の設定
		iConfig( sourceImage );

		// 領域の描画
		if ( pointCount == 4 ) {
			cv::line( configImage, cornerPoint[0], cornerPoint[1], CV_RGB(0, 255, 0), 2, 8, 0 );
			cv::line( configImage, cornerPoint[0], cornerPoint[2], CV_RGB(0, 255, 0), 2, 8, 0 );
			cv::line( configImage, cornerPoint[1], cornerPoint[3], CV_RGB(0, 255, 0), 2, 8, 0 );
			cv::line( configImage, cornerPoint[2], cornerPoint[3], CV_RGB(0, 255, 0), 2, 8, 0 );
		}

		// 画像の結合
		destinationImage = cv::Mat::zeros( configImage.rows, enlargeImage.cols + configImage.cols, CV_8UC3 );
		enlargeImage.copyTo( destinationImage(cv::Rect(0, 0, enlargeImage.cols, enlargeImage.rows)) );
		configImage.copyTo( destinationImage(cv::Rect(enlargeImage.cols, 0, configImage.cols, configImage.rows)) );

		// 画像の表示
		cv::imshow( windowName, destinationImage );

		int key = cv::waitKey(1);
		switch (key) {
		case 2424832:     // 左方向キーで左へ
			enlargePoint.x -= 1;
			break;
		case 2555904:     // 右方向キーで右へ
			enlargePoint.x += 1;
			break;
		case 2490368:     // 上方向キーで上へ
			enlargePoint.y -= 1;
			break;
		case 2621440:     // 下方向キーで下へ
			enlargePoint.y += 1;
			break;
		case 8:     // BackSpaceキーで再指定
			pointCount = 0;
			clickPoint.x = 0;
			clickPoint.y = 0;
			iNextStep();
			break;
		case 13:     // Enterキーでステップ移行
			if ( pointCount >= 4) {
				continueFlag = false;
			}
			else {
				iNextStep();
			}
			break;
		case 27:     // Escキーで終了
			return 0;
		default:
			break;
		}
	}

	// ウィンドウの破棄
	cv::destroyWindow( windowName );

	// ホモグラフィ行列の取得
	cv::Mat homographyMatrix = iHomography( cornerPoint );

	// ウィンドウの設定
	windowName = "Captured Image";
	cv::namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	cv::moveWindow( windowName, 640, 0 );

	// コンソールの表示
	std::cout << " > 入力画像に対して射影変換の実行中…" << "\n";
	std::cout << std::endl;
	std::cout << " [Enter] データの保存" << "\t" << " [Esc] 強制終了" << "\n";
	std::cout << std::endl;

	// フラグのセット
	continueFlag = true;

	/* 射影変換 */
	while ( continueFlag ) {

		// 画像の取得
		capture >> sourceImage;

		// 射影変換
		cv::warpPerspective( sourceImage, destinationImage, homographyMatrix, cv::Size(TRANSFORM_WIDTH, TRANSFORM_HEIGHT) );

		// 画像の表示
		cv::imshow( windowName, destinationImage );

		// キーの入力
		int key = cv::waitKey(1);
		switch ( key ) {
		case 13:     // Enterキーでデータ保存
			continueFlag = false;
			break;
		case 27:     // Escキーで終了
			return 0;
		default:
			break;
		}
	}

	// データの保存
	iWriteData( "CornerPoint", cornerPoint );
	iWriteData( "HomographyMatrix", homographyMatrix );

	return 0;
}