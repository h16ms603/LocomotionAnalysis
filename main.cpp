#include "main.h"

using namespace std;
using namespace cv;

int main( int argc, char **argv ) {

	cout << " 点の移動：　　　上…↑　\n" <<
			" 　　　　　  ←…左　　　右…→\n" <<
			" 　　　　　  　　下…↓\n" <<
			" [Enter] 座標の決定\t [q] 強制終了\n" <<
			" [n] 次の点へ進む\t [b] 一つ前の点に戻る\t[r] その画像の一点目に戻る\n" <<
			" [N] 次の画像へ進む\t [B] 一つ前の画像に戻る\n" << endl;

	// ウィンドウの生成
	String	windowName	= "Source and Closeup";
	namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	moveWindow(  windowName, 0, 0 );
	iGetImage( );
	imshow( windowName, imageShow );
	
	// マウス入力への対応
	setMouseCallback( windowName, iMouseCallBack );

	while ( continueFlag ) {
		
		// 画像生成 + 表示
		iMakeImageShow( );
		imshow( windowName, imageShow );
		int key = waitKey( 1 );

		// キー入力への対応
		iKeyResponse( key );
	}

	// データの書き込み
	iSaveData( );

	return	EXIT_SUCCESS;
}