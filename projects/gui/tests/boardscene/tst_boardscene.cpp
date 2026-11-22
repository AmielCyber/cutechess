#include <QtTest/QTest>

#include "boardview/boardscene.h"

class tst_BoardScene: public QObject
{
	Q_OBJECT
private slots:
	void beforeMove_data();
	void beforeMove();

	void selfMove_data();
	void selfMove();

	void opponentMove_data();
	void opponentMove();
};

void tst_BoardScene::beforeMove_data(){

}

void tst_BoardScene::beforeMove()
{

}

void tst_BoardScene::selfMove_data(){

}

void tst_BoardScene::selfMove()
{

}

void tst_BoardScene::opponentMove_data(){

}

void tst_BoardScene::opponentMove()
{

}

QTEST_MAIN(tst_BoardScene)

#include "tst_boardscene.moc"