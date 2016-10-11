
#include <windows.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "file.h"
#include "calculate.h"

// �����\�����[�h
#define AUTO

// ����ۑ����[�h
//#define VIDEO

/***************************************************************************************************/
int main( void ) {
	
	bool showFlag;

	std::stringstream fileName;

	cv::Mat sourceImage;
	cv::Mat destinationImage;
	cv::Mat stickImage;
	
	// �R���\�[���̐ݒ�
	char consoleName[] = "Stick Diagram";
	SetConsoleTitleA( consoleName );
	Sleep( 20 );
	MoveWindow( FindWindowA(NULL, consoleName), 0, 0, 640, 480, true );
	std::cout << std::endl;

	// �E�B���h�E�̐ݒ�
	std::string windowName = "Source Image";
	cv::namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	cv::moveWindow( windowName, 650, 0 );
#ifdef VIDEO
	// �o�͓���̃I�[�v��
	cv::VideoWriter writer.open("StickDiagram.avi", CV_FOURCC('X', 'V', 'I', 'D'), RATE_FRAME, cv::Size(2 * WIDTH, HEIGHT));
	if ( !writer.isOpened() ) {
		std::cerr << " Error : ����̕ۑ����ł��܂���D" << std::endl;
	}
#endif
	// �f�[�^�̓ǂݍ���
	iReadCoordinate();
	iReadBody();

	// �R���\�[���̕\��
	std::cout << " > " << "�t�@�C���F" << " data" << fileNumber << ".txt" << " ������W�f�[�^���擾�D" << "\n";
	std::cout << " > " << "�t�H���_�F" << NAME_FOLDER << " ����g�̃f�[�^���擾�D" << "\n";
	std::cout << std::endl;

	// �f�[�^�̒T��
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

	// �R���\�[���̕\��
	std::cout << " > �o�b�^�������n�ʂ��痣�ꂽ�Ƃ��̉摜��I�����Ă��������D" << "\n";
	std::cout << std::endl;
	std::cout << " [��] �O�摜�̕\��" << "\t" << " [��] ���摜�̕\��" << "\n";
	std::cout << " [Enter] �摜�̌���" << "\t" << " [Esc] �����I��" << "\n";
	std::cout << std::endl;

	// �J�E���^�̃��Z�b�g
	imageCount = beginCount;
	showFlag = true;

	/* �����摜�̑I�� */
	while ( showFlag ) {

		// �摜�̎擾
		fileName.str("");
		fileName << std::setw(3) << std::setfill('0') << imageCount << ".bmp";
		sourceImage = cv::imread( fileName.str() );

		// �R���\�[���̕\��
		std::cout << " > �摜�ԍ��F" << imageCount << "\r" << std::flush;

		// �摜�̕\��
		cv::imshow( windowName, sourceImage );

		// �L�[�̓���
		int key = cv::waitKey(1);
		switch ( key ) {
		case 2424832:     // �������L�[�őO��
			if ( imageCount > beginCount ) {
				imageCount -= 1;
			}
			else {
				imageCount = beginCount;
			}
			break;
		case 2555904:     // �E�����L�[�Ŏ���
			if ( imageCount < endCount ) {
				imageCount += 1;
			}
			else {
				imageCount = endCount;
			}
			break;
		case 13:     // Enter�L�[�Ō���
			showFlag = false;
			liftoffCount = imageCount;
			break;
		case 27:     // Esc�L�[�ŏI��
			return 0;
		default:
			break;
		}
	}

	// �R���\�[���̕\��
	std::cout << " > " << liftoffCount << " ���ڂ𗣗������Ƃ��̉摜�Ɍ���D" << "\n";
	std::cout << std::endl;

	// �E�B���h�E�̔j��
	cv::destroyWindow( windowName );

	// �̏��̎擾
	iGetInformation( dataMass, dataLength );

	// �����N�E�W���C���g�̎擾
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
	
	// �E�B���h�E�̐ݒ�
	windowName = "Left:Source Image, Right:Stick Diagram";
	cv::namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	cv::moveWindow( windowName, 540, 0 );

	// �R���\�[���̕\��
	std::cout << " > " << beginCount << " ���� �` " << endCount << " ���ڂ܂ł̉摜��\��" << "\n";
	std::cout << std::endl;
#ifdef AUTO
	std::cout << "----- �����\�����[�h --------------------------------------------" << "\n";
	std::cout << std::endl;
#else
	std::cout << "----- �蓮�\�����[�h --------------------------------------------" << "\n";
	std::cout << std::endl;
	std::cout << " [��] �O�摜�̕\��" << "\t" << " [��] ���摜�̕\��" << "\n";
#endif
	std::cout << " [Enter] �f�[�^�̕ۑ�" << "\t" << " [Esc] �����I��" << "\n";
	std::cout << std::endl;

	// �J�E���^�̃��Z�b�g
	showFlag = true;
	imageCount = beginCount;

	/* ���C�����[�v */
	while ( showFlag ) {

		// �摜�̎擾
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
		
		// ���̕`��
		if ( imageCount < liftoffCount ) {
			cv::line( stickImage, cv::Point(0, HEIGHT - (int)jointPointY[liftoffCount - 1][2]),
				cv::Point(WIDTH, HEIGHT - (int)jointPointY[liftoffCount - 1][2]), CV_RGB(200, 200, 200), 2, 8, 0 );
		}
		for ( int i = 0; i < NUMBER_LINK; i++ ) {
			cv::line( stickImage, cv::Point((int)linkPointX[imageCount][i], HEIGHT - (int)linkPointY[imageCount][i]), 
				cv::Point((int)linkPointX[imageCount][i + NUMBER_LINK], HEIGHT - (int)linkPointY[imageCount][i + NUMBER_LINK]), CV_RGB(0, 255, 0), 2, 8, 0 );
		}
		
		// �_�̕`��
		for ( int i = 0; i < NUMBER_JOINT; i++ ) {
			cv::circle( stickImage, cv::Point((int)jointPointX[imageCount][i], HEIGHT - (int)jointPointY[imageCount][i]), 5, CV_RGB(0, 0, 255), -1, 8, 0 );
		}
		for ( int i = 0; i < NUMBER_LINK; i++ ) {
			cv::circle( stickImage, cv::Point((int)linkPositionX[imageCount][i], HEIGHT - (int)linkPositionY[imageCount][i]), 4, CV_RGB(255, 0, 0), -1, 8, 0 );
		}
		cv::circle( stickImage, cv::Point((int)positionX[imageCount], HEIGHT - (int)positionY[imageCount]), 5, CV_RGB(255, 127, 0), -1, 8, 0 );

		// �摜�̌���
		cv::hconcat( sourceImage, stickImage, destinationImage );
#ifdef VIDEO
		// ����̕ۑ�
		writer << destinationImage;
#endif
		// �R���\�[���̕\��
		std::cout << " > �摜�ԍ��F" << imageCount << "\r" << std::flush;

		// �摜�̕\��
		cv::imshow( windowName, destinationImage );

		// �L�[�̓���
		int key = cv::waitKey(1);
		switch ( key ) {
		case 2424832:     // �������L�[�őO��
			if ( imageCount > beginCount ) {
				imageCount -= 1;
			}
			else {
				imageCount = beginCount;
			}
			break;
		case 2555904:     // �E�����L�[�Ŏ���
			if ( imageCount < endCount ) {
				imageCount += 1;
			}
			else {
				imageCount = endCount;
			}
			break;
		case 13:     // Enter�L�[�ŕۑ�
			showFlag = false;
			break;
		case 27:     // Esc�L�[�ŏI��
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

	// �f�[�^�̎擾
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
	// �f�[�^�̕ۑ�
	iSaveData();
	return 0;
} 
/***************************************************************************************************/