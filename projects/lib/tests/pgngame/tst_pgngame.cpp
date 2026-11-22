#include <QtTest/QTest>

#include <QFile>
#include <QTemporaryDir>

#include <pgngame.h>
#include <pgnstream.h>
#include <board/board.h>

class tst_PgnGame: public QObject {
	Q_OBJECT

private slots:
	void saveGameState_data();
	void saveGameState();
private:
	QString finalFen(const PgnGame& game);
};


QString tst_PgnGame::finalFen(const PgnGame& game){
	return QString();
}

void tst_PgnGame::saveGameState_data()
{
}

void tst_PgnGame::saveGameState(){
}

QTEST_MAIN(tst_PgnGame)
#include "tst_pgngame.moc"
