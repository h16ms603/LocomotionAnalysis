#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <conio.h>
#include <io.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define	MAX_POINT	  8				// 一画像あたりの探索点の数
#define	MAX_IMAGE	1000			// 一跳躍あたりの画像枚数（多めにとってよい）
#define	SCALING_W	  1.5			// 全体画像（表示画像の右側）の拡大倍率
#define SCALING_C	  3				// 拡大画像（表示画像の左側）の拡大倍率

#define CIRCLE_RADIUS	 3	// 描画点の大きさ
#define LINE_THICKNESS	-1	// 描画円の中を埋める

Mat	imageOriginal;	// 各時刻の取得画像
Mat	imageWhole;		// 全体画像（表示画像の右側）
Mat	imageCloseup;	// 拡大画像（表示画像の左側）
Mat	imageShow;		// 表示画像

int	xWhole[MAX_IMAGE][MAX_POINT] = {};	// X 座標のデータ（全体画像）
int	yWhole[MAX_IMAGE][MAX_POINT] = {};	// Y 座標のデータ（全体画像）
int	xCloseup[MAX_IMAGE][MAX_POINT] = {};// X 座標のデータ（拡大画像）
int	yCloseup[MAX_IMAGE][MAX_POINT] = {};// Y 座標のデータ（拡大画像）
int countPoint = 0;						// 現在探索している点の番号
int countImage = 0;						// 現在探索している画像の番号

bool	continueFlag	= true;

// 探索座標情報のテキスト表示
inline void iDisplay( void ) {
	cout << "\r 現在，画像 " << setfill('0') << setw(3) << right << countImage << ".bmp の " << countPoint + 1  << "/" << MAX_POINT << " 点目の座標を探索中です。" << flush;
}

// 画像ファイルの取得
inline void iGetImage( void ) {

	stringstream nameFile;

	countPoint = 0;

	// 画像の読み込み
	nameFile << setfill('0') << setw(3) << right << countImage << ".bmp";
	imageOriginal = imread( nameFile.str() );

	if ( ( imageOriginal.data == NULL ) && ( countImage != 0 ) ) {

		cout << "一跳躍分のデータを取り終えました。" << endl;
		continueFlag	= false;

	} else {

		// 一枚目の読み込みの場合，表示画像と拡大画像の準備
		if ( countImage == 0 ) {

			while ( imageOriginal.data == NULL ) {
				countImage++;
				nameFile.str("");
				nameFile << setfill('0') << setw(3) << right << countImage << ".bmp";
				imageOriginal = imread( nameFile.str() );

				if ( countImage > MAX_IMAGE ) {
					cout << "ファイルが見つかりません。" << endl;
					exit( 0 );
				}
			}

			imageCloseup = Mat::zeros( Size( (int) ( SCALING_C * imageOriginal.cols ), (int) ( SCALING_C * imageOriginal.rows ) ), CV_8UC3 );
			imageWhole   = Mat::zeros( Size( (int) ( SCALING_W * imageOriginal.cols ), (int) ( SCALING_W * imageOriginal.rows ) ), CV_8UC3 );
			
		}

		// 探索座標情報のテキスト表示
		iDisplay();

		// 表示画像のリセット
		imageShow = Mat::zeros( Size( imageWhole.rows + imageWhole.cols, imageWhole.rows ), CV_8UC3 );
	}
}

// つぎの画像に進む
inline void iNextImage( void ) {
	
	countImage++;
	iGetImage( );

}

// 座標取得の確認
inline bool iCheckPoint( int x, int y ) {

	if ( x != 0 || y != 0 ) {
		return	true;
	} else {
		return	false;
	}
}

// 座標の決定・つぎの探索へ移行
inline void iNextStep( void ) {

	if ( iCheckPoint( xCloseup[countImage][countPoint], yCloseup[countImage][countPoint] ) ) {

		// つぎの点の探索
		countPoint++;
		
		if ( countPoint >= MAX_POINT ) {
			if ( countImage == MAX_IMAGE - 1 ) {
				continueFlag = false;
			} else {
				iNextImage( );
			}
		} else {
			iDisplay();
		}

	} else {
		cout << "\r マウス左クリックにより，座標を確保してください。           " << flush;
	}
}

// マウス操作
void iMouseCallBack( int event, int x, int y, int flags, void* ) {

	switch ( event ) {

		case EVENT_LBUTTONDOWN:
			// マウスの左ボタンがクリックされた場合，座標を一時的に確保
			if ( x > imageWhole.rows ) {
				xWhole[countImage][countPoint]	 = x - imageWhole.rows;
				yWhole[countImage][countPoint]	 = y;
				xCloseup[countImage][countPoint] = (int) ( xWhole[countImage][countPoint] * SCALING_C / SCALING_W );
				yCloseup[countImage][countPoint] = (int) ( yWhole[countImage][countPoint] * SCALING_C / SCALING_W );
			}
			break;

		case EVENT_RBUTTONDOWN:
			// マウスの右ボタンがクリックされた場合，座標を決定
			iNextStep();
			break;
	}
}

// 拡大画像座標から全体画像座標への変換
inline int iCloseupToWhole( int x ) {
	return	(int) ( x * SCALING_W / SCALING_C );
}

inline Point iCloseupToWhole( int x, int y ) {

	return Point( iCloseupToWhole( x ), iCloseupToWhole( y ) );
}

// ( x1, y1 ) と ( x2, y2 ) を通る直線の描画
inline Point iLineRight( int x1, int y1, int x2, int y2, int w ) { 

	return Point( w, (int) ( (double) ( w * ( y1 - y2 ) - x2 * y1 + x1 * y2 ) / ( x1 - x2 ) ) );
}

inline Point iLineLeft( int x1, int y1, int x2, int y2 ) { 

	return Point( 0, (int) ( (double) ( x1 * y2 - x2 * y1 ) / ( x1 - x2 ) ) );
}

inline Point iLineTop( int x1, int y1, int x2, int y2 ) { 

	return Point( (int) ( (double) ( x2 * y1 - x1 * y2 ) / ( y1 - y2 ) ), 0 );
}

inline Point iLineBottom( int x1, int y1, int x2, int y2, int h ) { 

	return Point( (int) ( (double) ( h * ( x1 - x2 ) + x2 * y1 - x1 * y2 ) / ( y1 - y2 ) ), h );
}

inline void iLine( Mat image, int x1, int y1, int x2, int y2 ) {

	if ( abs( y1 - y2 ) > abs( x1 - x2 ) ) {
		if ( y1 != y2 ) {
			line( image, iLineBottom( x1, y1, x2, y2, image.rows ), iLineTop( x1, y1, x2, y2 ), CV_RGB( 100, 100, 100 ), 2 );
			line( image, iLineBottom( x1, y1, x2, y2, image.rows ), iLineTop( x1, y1, x2, y2 ), CV_RGB( 255, 255, 255 ) );
		}
	} else {
		if ( x1 != x2 ) {
			line( image, iLineRight( x1, y1, x2, y2, image.cols ), iLineLeft( x1, y1, x2, y2 ), CV_RGB( 100, 100, 100 ), 2 );
			line( image, iLineRight( x1, y1, x2, y2, image.cols ), iLineLeft( x1, y1, x2, y2 ), CV_RGB( 255, 255, 255 ) );
		}
	}
}

// ( x1, y1 ) を通り，「( x2, y2 ) と ( x3, y3 ) を通る直線」と直交する直線の描画 
inline Point iPerpRight( int x1, int y1, int x2, int y2, int x3, int y3, int w ) { 

	return Point( w, (int) ( (double) ( x2 - x3 ) / ( y2 - y3 ) * ( x1 - w ) + y1 ) );
}

inline Point iPerpLeft( int x1, int y1, int x2, int y2, int x3, int y3 ) { 

	return Point( 0, (int) ( (double) ( x2 - x3 ) / ( y2 - y3 ) * x1 + y1 ) );
}

inline Point iPerpTop( int x1, int y1, int x2, int y2, int x3, int y3 ) { 

	return Point( (int) ( (double) ( y2 - y3 ) / ( x2 - x3 ) * y1 + x1 ), 0 );
}

inline Point iPerpBottom( int x1, int y1, int x2, int y2, int x3, int y3, int h ) { 

	return Point( (int) ( (double) ( y2 - y3 ) / ( x2 - x3 ) * ( y1 - h ) + x1 ), h );
}

inline void iPerp( Mat image, int x1, int y1, int x2, int y2, int x3, int y3 ) {

	if ( abs( y2 - y3 ) > abs( x2 - x3 ) ) {
		if ( y2 != y3 ) {
			line( image, iPerpRight( x1, y1, x2, y2, x3, y3, image.cols ), iPerpLeft( x1, y1, x2, y2, x3, y3 ), CV_RGB( 100, 100, 100 ), 2 );
			line( image, iPerpRight( x1, y1, x2, y2, x3, y3, image.cols ), iPerpLeft( x1, y1, x2, y2, x3, y3 ), CV_RGB( 255, 255, 255 ) );
		}
	} else {
		if ( x2 != x3 ) {
			line( image, iPerpBottom( x1, y1, x2, y2, x3, y3, image.rows ), iPerpTop( x1, y1, x2, y2, x3, y3 ), CV_RGB( 100, 100, 100 ), 2 );
			line( image, iPerpBottom( x1, y1, x2, y2, x3, y3, image.rows ), iPerpTop( x1, y1, x2, y2, x3, y3 ), CV_RGB( 255, 255, 255 ) );
		}
	}
}

//( x1, y1 ) を通り，「( x2, y2 ) と ( x3, y3 ) を通る直線」と平行する直線の描画
inline Point iParallelRight( int x1, int y1, int x2, int y2, int x3, int y3, int w ) { 

	return Point( w, y1 - (int) ( (double) ( x1 - w ) * ( y2 - y3 ) / ( x2 - x3 ) ) );
}

inline Point iParallelLeft( int x1, int y1, int x2, int y2, int x3, int y3 ) { 

	return Point( 0, y1 - (int) ( (double) x1 * ( y2 - y3 ) / ( x2 - x3 ) ) );
}

inline Point iParallelTop( int x1, int y1, int x2, int y2, int x3, int y3 ) { 

	return Point( x1 - (int) ( (double) y1 * ( x2 - x3 ) / ( y2 - y3 ) ), 0 );
}

inline Point iParallelBottom( int x1, int y1, int x2, int y2, int x3, int y3, int h ) { 

	return Point( x1 - (int) ( (double) ( y1 - h ) * ( x2 - x3 ) / ( y2 - y3 ) ), h );
}

inline void iParallel( Mat image, int x1, int y1, int x2, int y2, int x3, int y3 ) {

	if ( abs( y1 - y2 ) > abs( x1 - x2 ) ) {
		if ( y1 != y2 ) {
			line( image, iParallelBottom( x1, y1, x2, y2, x3, y3, image.rows ), iParallelTop( x1, y1, x2, y2, x3, y3 ), CV_RGB( 100, 100, 100 ), 2 );
			line( image, iParallelBottom( x1, y1, x2, y2, x3, y3, image.rows ), iParallelTop( x1, y1, x2, y2, x3, y3 ), CV_RGB( 255, 255, 255 ) );
		}
	} else {
		if ( x1 != x2 ) {
			line( image, iParallelRight( x1, y1, x2, y2, x3, y3, image.cols ), iParallelLeft( x1, y1, x2, y2, x3, y3 ), CV_RGB( 100, 100, 100 ), 2 );
			line( image, iParallelRight( x1, y1, x2, y2, x3, y3, image.cols ), iParallelLeft( x1, y1, x2, y2, x3, y3 ), CV_RGB( 255, 255, 255 ) );
		}
	}
}

// 円の描画
inline void iCircle( int x, int y, Scalar color ) {
	circle( imageWhole,   iCloseupToWhole( x, y ), CIRCLE_RADIUS, color, LINE_THICKNESS );
	circle( imageCloseup, Point( x, y ),           CIRCLE_RADIUS, color, LINE_THICKNESS );
}

// 拡大画像の端点座標の補正
inline int iConerOfCloseup( int x, int max ) {

	x -= imageWhole.rows / 2;

	if ( x < 0 ) {

		x = 0;

	} else if ( x > max - imageWhole.rows - 1 ) {

		x = max - imageWhole.rows - 1;

	}

	return x;
}

//
inline void iCloseupCopyToShow( int x, int y ) {
	imageCloseup( Rect( iConerOfCloseup( x, imageCloseup.cols ), iConerOfCloseup( y, imageCloseup.rows ), imageWhole.rows, imageWhole.rows ) ).copyTo( imageShow( Rect( 0, 0, imageWhole.rows, imageWhole.rows ) ) );
}

// 表示画像の生成，点・線の描写
inline void iMakeImageShow( void ) {

	// 全体画像と拡大画像のリセット
	resize( imageOriginal, imageWhole,   Size( imageWhole.cols,   imageWhole.rows   ) );
	resize( imageOriginal, imageCloseup, Size( imageCloseup.cols, imageCloseup.rows ) );

	// 線の描画
	for ( int i = 0; i < MAX_POINT; i++ ) {

		if ( i <= countPoint ) {

			// 背中に直線を描画
			if ( ( countPoint > 4 ) || ( ( countPoint == 4 ) && iCheckPoint( xCloseup[countImage][4], yCloseup[countImage][4] ) ) ) {
				iLine( imageWhole,   xWhole[countImage][4],   yWhole[countImage][4],   xWhole[countImage][3],   yWhole[countImage][3]   );
				iLine( imageCloseup, xCloseup[countImage][4], yCloseup[countImage][4], xCloseup[countImage][3], yCloseup[countImage][3] );
			}

			// 体部の後端に背中直線の垂線を描画
			if ( ( countPoint > 5 ) || ( ( countPoint == 5 ) && iCheckPoint( xCloseup[countImage][5], yCloseup[countImage][5] ) ) ) {
				iPerp( imageWhole,   xWhole[countImage][5],   yWhole[countImage][5],   xWhole[countImage][3],   yWhole[countImage][3],   xWhole[countImage][4],   yWhole[countImage][4]   );
				iPerp( imageCloseup, xCloseup[countImage][5], yCloseup[countImage][5], xCloseup[countImage][3], yCloseup[countImage][3], xCloseup[countImage][4], yCloseup[countImage][4] );
			}

			// 体部の先端に背中直線の垂線を描画
			if ( (countPoint > 6 ) || ( ( countPoint == 6 ) && iCheckPoint( xCloseup[countImage][6], yCloseup[countImage][6] ) ) ) {
				iPerp( imageWhole,   xWhole[countImage][6],   yWhole[countImage][6],   xWhole[countImage][3],   yWhole[countImage][3],   xWhole[countImage][4],   yWhole[countImage][4]   );
				iPerp( imageCloseup, xCloseup[countImage][6], yCloseup[countImage][6], xCloseup[countImage][3], yCloseup[countImage][3], xCloseup[countImage][4], yCloseup[countImage][4] );
			}

			// 腹部の下端に背中直線の平行線を描画
			/*if ( ( countPoint == 7 ) && iCheckPoint( xCloseup[countImage][7], yCloseup[countImage][7] ) ) {
				iParallel( imageWhole,   xWhole[countImage][7],   yWhole[countImage][7],   xWhole[countImage][3],   yWhole[countImage][3],   xWhole[countImage][4],   yWhole[countImage][4]   );
				iParallel( imageCloseup, xCloseup[countImage][7], yCloseup[countImage][7], xCloseup[countImage][3], yCloseup[countImage][3], xCloseup[countImage][4], yCloseup[countImage][4] );
			}*/
			if ((countPoint > 4) && iCheckPoint(xCloseup[countImage][2], yCloseup[countImage][2])) {
				//iParallel(imageWhole, xWhole[countImage][2], yWhole[countImage][2], xWhole[countImage][3], yWhole[countImage][3], xWhole[countImage][4], yWhole[countImage][4]);
				iPerp(imageWhole, xWhole[countImage][2], yWhole[countImage][2], xWhole[countImage][3], yWhole[countImage][3], xWhole[countImage][4], yWhole[countImage][4]);
				//iParallel(imageCloseup, xCloseup[countImage][2], yCloseup[countImage][2], xCloseup[countImage][3], yCloseup[countImage][3], xCloseup[countImage][4], yCloseup[countImage][4]);
				iPerp(imageCloseup, xCloseup[countImage][2], yCloseup[countImage][2], xCloseup[countImage][3], yCloseup[countImage][3], xCloseup[countImage][4], yCloseup[countImage][4]);
			}
		}
	}

	// 点の描画
	for ( int i = 0; i < MAX_POINT; i++ ) {

		if ( i < countPoint ) {
			// 探索済みの座標は緑で表示
			iCircle( xCloseup[countImage][i], yCloseup[countImage][i], CV_RGB( 0, 255, 0 ) );

		} else {

			if ( i == countPoint ) {
				// 探索中の座標は赤で表示
				if ( iCheckPoint( xCloseup[countImage][i], yCloseup[countImage][i] ) ) {
					iCircle( xCloseup[countImage][i], yCloseup[countImage][i], CV_RGB( 255, 0, 0 ) );
				}
			} else {
				// 探索をやり直した場合に，やり直し前の座標は青で表示
				if ( iCheckPoint( xCloseup[countImage][i], yCloseup[countImage][i] ) ) {
					iCircle( xCloseup[countImage][i], yCloseup[countImage][i], CV_RGB( 0, 0, 255 ) );
				}
			}
		}
	}

	// 表示画像の生成
	imageWhole.copyTo( imageShow( Rect( imageWhole.rows, 0, imageWhole.cols, imageWhole.rows ) ) );

	if ( iCheckPoint( xCloseup[countImage][countPoint], yCloseup[countImage][countPoint] ) ) {
		iCloseupCopyToShow( xCloseup[countImage][countPoint], yCloseup[countImage][countPoint] );
	} else if ( countPoint > 0 ) {
		iCloseupCopyToShow( xCloseup[countImage][ countPoint - 1 ], yCloseup[countImage][ countPoint - 1 ] );
	}
}

// データ欠損の確認
inline bool iCheckData( void ) {
	bool ret = true;

	for ( int i = 0; i < MAX_POINT; i++ ) {
		if ( !iCheckPoint( xCloseup[countImage][i], yCloseup[countImage][i] ) ) {
			ret = false;
			break;
		}
	}

	return ret;
}

// キー入力への対応
inline void iKeyResponse( int key ) {

	if ( key == 'q' ){				// 強制終了

		continueFlag   = false;							

	} //else if ( key == 2424832) {	// 座標の左←移動
	else if (key == 'a') {

		if ( xCloseup[countImage][countPoint]  > 0 ) {
			 xCloseup[countImage][countPoint] -= 1;
			 xWhole[countImage][countPoint]    = iCloseupToWhole( xCloseup[countImage][countPoint] );
		}
		
	} //else if ( key == 2555904 ) {	// 座標の右→移動
	else if (key == 'd') {
	
		if ( xCloseup[countImage][countPoint]  < imageCloseup.cols - 1 ) {
			 xCloseup[countImage][countPoint] += 1;
			 xWhole[countImage][countPoint]    = iCloseupToWhole( xCloseup[countImage][countPoint] );
		}
		
	} //else if ( key == 2490368 ) {	// 座標の上↑移動
	else if (key == 'w') {
		
		if ( yCloseup[countImage][countPoint]  > 0 ) {
			 yCloseup[countImage][countPoint] -= 1;
			 yWhole[countImage][countPoint]    = iCloseupToWhole( yCloseup[countImage][countPoint] );
		}
		
	} //else if ( key == 2621440 ) {	// 座標の下↓移動
	else if (key == 's') {
		
		if ( yCloseup[countImage][countPoint]  < imageCloseup.rows - 1 ) {
			 yCloseup[countImage][countPoint] += 1;
			 yWhole[countImage][countPoint]    = iCloseupToWhole( yCloseup[countImage][countPoint] );
		}
		
	} else if ( key == 0x0d ) {		// 座標の決定

		iNextStep();
	
	} else if ( key == 'b' ) {		// ひとつ前の座標探索に戻る
		
		if ( countPoint > 0 ) {
			countPoint--;
			iDisplay();
		}

	} else if ( key == 'r' ) {		// その画像の一点目の座標探索に戻る

		countPoint = 0;
		iDisplay();

	} else if ( key == 'N' ) {		// つぎの画像へ進む
	
		if ( iCheckData() ) {
			iNextImage( );
		} else {
			cout << "\r座標の取得が終わっていません。　　　　　　　　　　　　　　　" << flush;
		}

	} else if ( key == 'B' ) {		// 前の画像に戻る

		if ( countImage > 0 ) {
			countImage--;
			iGetImage( );
		} else {
			cout << "\r前に画像がありません。　　　　　　　　　　　　　　　　　　　" << flush;
		}
	}
}

// データの保存
inline void iSaveData( void ) {

	stringstream nameFile;
	int	k = 0;

	do {
		nameFile.str("");
		nameFile << "data" << k++ << ".txt";
	} while ( _access_s( nameFile.str().c_str(), 0 ) != ENOENT );
	

	ofstream	streamFile( nameFile.str() );

	for ( int i = 0; i < MAX_IMAGE; i++ ) {
		for ( int j = 0; j < MAX_POINT; j++ ) {
			streamFile << ( (double) xCloseup[i][j] / SCALING_C ) << "\t" << ( (double) yCloseup[i][j] / SCALING_C ) << "\t";
		}
		streamFile << "\n";
	}

	streamFile << flush;

	cout << "\n " << nameFile.str() << " にデータを保存しました。\n" << endl;
}