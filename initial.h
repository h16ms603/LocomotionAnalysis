
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

// 画像サイズ
#define   WIDTH    1280     // フレームの横幅
#define   HEIGHT    720     // フレームの縦幅
#define   FPS        30     // フレームレート

// 画像処理パラメータ
#define   TRANSFORM_WIDTH    720     // フレームの横幅
#define   TRANSFORM_HEIGHT   720     // フレームの縦幅
#define   MAGNIFICATION        5     // 拡大画像の倍率

int pointCount = 0;

cv::Point  cursorPoint = { 0, 0 };       // カーソル点
cv::Point   clickPoint = { 0, 0 };       // クリック点
cv::Point  configPoint = { 0, 0 };       // 設定画像上の点
cv::Point enlargePoint = { 0, 0 };       // 拡大画像上の点
cv::Point cornerPoint[4] = { 0, 0 };     // コーナ点

cv::Mat    configImage;     // 設定画像
cv::Mat   enlargeImage( 100 * MAGNIFICATION, 100 * MAGNIFICATION, CV_8UC3 );     // 拡大画像

// ステップ移行
void iNextStep( void ) {

	// コーナ点の取得
	if ( clickPoint.x != 0 && clickPoint.y != 0 ) {
		cornerPoint[pointCount].x = (configPoint.x - 50) + (enlargePoint.x / MAGNIFICATION);
		cornerPoint[pointCount].y = (configPoint.y - 50) + (enlargePoint.y / MAGNIFICATION);
		pointCount += 1;
	}

	// コンソールの表示
	switch ( pointCount ) {
	case 0:
		std::cout << " > 左上の位置を指定してください．" << "\r" << std::flush;
		break;
	case 1:
		std::cout << " > 右上の位置を指定してください．" << "\r" << std::flush;
		break;
	case 2:
		std::cout << " > 左下の位置を指定してください．" << "\r" << std::flush;
		break;
	case 3:
		std::cout << " > 右下の位置を指定してください．" << "\r" << std::flush;
		break;
	case 4:
		std::cout << " > 領域の確認をしてください．　　" << "\n";
		std::cout << std::endl;
		std::cout << " [Enter] 決定" << "\t" << " [BackSpace] 再指定" << "\n";
		std::cout << " [Esc] 強制終了" << "\n";
		std::cout << std::endl;
		break;
	}

	// 点の初期化
	clickPoint.x = 0;
	clickPoint.y = 0;
}

// マウス操作
void iMouseCallBack( int event, int x, int y, int flags, void *param )  {

	switch ( event ) {

	// カーソル移動
	case CV_EVENT_MOUSEMOVE:
		cursorPoint.x = x - enlargeImage.cols;
		cursorPoint.y = y;
		break;

	// 左クリック
	case CV_EVENT_LBUTTONDOWN:
		clickPoint.x = x - enlargeImage.cols;
		clickPoint.y = y;
		break;

	// 右クリック
	case CV_EVENT_RBUTTONDOWN:
		iNextStep();
		break;
	}

	// 点の取得
	if ( event == CV_EVENT_LBUTTONDOWN ) {
		if ( clickPoint.x < 50 ) {
			configPoint.x = 50;
			enlargePoint.x = clickPoint.x * MAGNIFICATION;
		}
		else if ( clickPoint.x > configImage.cols - 50 ) {
			configPoint.x = configImage.cols - 50;
			enlargePoint.x = (100 - (configImage.cols - clickPoint.x)) * MAGNIFICATION;
		}
		else {
			configPoint.x = clickPoint.x;
			enlargePoint.x = 50 * MAGNIFICATION;
		}
		if ( clickPoint.y < 50 ) {
			configPoint.y = 50;
			enlargePoint.y = clickPoint.y * MAGNIFICATION;
		}
		else if ( clickPoint.y > configImage.rows - 50 ) {
			configPoint.y = configImage.rows - 50;
			enlargePoint.y = (100 - (configImage.rows - clickPoint.y)) * MAGNIFICATION;
		}
		else {
			configPoint.y = clickPoint.y;
			enlargePoint.y = 50 * MAGNIFICATION;
		}
	}
}

// 座標点設定
void iConfig( cv::Mat source ) {

	// 設定画像の取得
	source.copyTo( configImage );

	if ( clickPoint.x > 0 || clickPoint.y > 0 ) {
		
		// 拡大画像の取得
		cv::Mat  enlargeROI = source( cv::Rect(configPoint.x - 50, configPoint.y - 50,
	   							   	  100, 100) );
		cv::resize( enlargeROI, enlargeImage, cv::Size(enlargeImage.cols, enlargeImage.rows) );

		// 領域の描画
		cv::rectangle( configImage,
			           cv::Point(configPoint.x - 50, configPoint.y - 50),
			           cv::Point(configPoint.x + 50, configPoint.y + 50),
			           CV_RGB(0, 0, 255), 2, 8, 0 );

		// 点の描画
		cv::circle( enlargeImage, cv::Point(enlargePoint.x, enlargePoint.y), 4, CV_RGB(255, 0, 0), -1, 8, 0 );
	}
	else {
		enlargeImage = cv::Mat::zeros( enlargeImage.rows, enlargeImage.cols, CV_8UC3 );
	}
}

// ホモグラフィ行列算出
cv::Mat iHomography( cv::Point point[4] ) {

	cv::Mat matrix;
	cv::Mat beforeMatrix(4, 2, CV_32F);
	cv::Mat  afterMatrix(4, 2, CV_32F);

	// 射影変換前点行列の取得
	for ( int i = 0; i < 4; i++ ) {
		beforeMatrix.at<float>(i, 0) = static_cast<float>(point[i].x);
		beforeMatrix.at<float>(i, 1) = static_cast<float>(point[i].y);
	}

	// 射影変換後点行列の取得
	afterMatrix.at<float>(0, 0) = 0;
	afterMatrix.at<float>(0, 1) = 0;
	afterMatrix.at<float>(1, 0) = TRANSFORM_WIDTH;
	afterMatrix.at<float>(1, 1) = 0;
	afterMatrix.at<float>(2, 0) = 0;
	afterMatrix.at<float>(2, 1) = TRANSFORM_HEIGHT;
	afterMatrix.at<float>(3, 0) = TRANSFORM_WIDTH;
	afterMatrix.at<float>(3, 1) = TRANSFORM_HEIGHT;

	// ホモグラフィ行列の算出
	matrix = findHomography( beforeMatrix, afterMatrix );

	return matrix;
}