#include <QtTest/QTest>

#include <board/board.h>
#include <board/boardfactory.h>
#include "boardview/graphicsboard.h"

#include "boardview/boardscene.h"

class tst_BoardScene : public QObject
{
	Q_OBJECT
private slots:
	/**
	 * @brief beforeMove_data FEN strings to test the boards.
	 */
	void beforeMove_data();
	/**
	 * @brief beforeMove Perform the synchronization test beteween
	 * the backend board and the UI represented state of the board
	 * before a move is done.
	 */
	void beforeMove();

	/**
	 * @brief selfMove_data Sets the test table as: FEN string, Move string, and expected result.
	 */
	void selfMove_data();
	/**
	 * Performs the move operation and verifies the UI representation reflects that move and
	 * the squares in the backend chess board.
	 * @brief selfMove
	 */
	void selfMove();

private:
	/**
	 * @brief getGraphicsBoard Gets the GraphiBoard from BoardScene that is in charge
	 * of rendering the board to the UI
	 * @param scene The BoardScene that is in charge of maintaining the state and
	 * updating the UI by GraphicBoard
	 * @return The GraphicBoard representing the UI board
	 */
	const GraphicsBoard* getGraphicsBoard(const BoardScene& boardScene);
	/**
	 * @brief verifySynchronization Verifies the squares in the backend board
	 * matches the squares in the GraphicsBoard that will be used to display the
	 * UI board.
	 * @param boardScene The BoardScene that contains the backend board
	 * and the GraphicsBoard to check its syncrhonization.
	 */
	void verifySynchronization(const BoardScene& boardScene);
};

const GraphicsBoard* tst_BoardScene::getGraphicsBoard(const BoardScene& boardScene)
{
	for (QGraphicsItem* item : boardScene.items())
	{
		if (item->type() == GraphicsBoard::Type)
		{
			return static_cast<GraphicsBoard*>(item);
		}
	}
	return nullptr;
}

void tst_BoardScene::verifySynchronization(const BoardScene& boardScene)
{
	const Chess::Board* board = boardScene.board();

	QVERIFY(board != nullptr);

	const GraphicsBoard* graphicsBoard = getGraphicsBoard(boardScene);

	QVERIFY(graphicsBoard != nullptr);

	for (int file = 0; file < board->width(); ++file)
	{
		for (int rank = 0; rank < board->height(); ++rank)
		{
			Chess::Square square(file, rank);

			QTRY_COMPARE(graphicsBoard->pieceTypeAt(square), board->pieceAt(square));
		}
	}
}

void tst_BoardScene::beforeMove_data()
{
	QTest::addColumn<QString>("fen");

	QTest::newRow("before-self-move")
		<< QString("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
	QTest::newRow("before-opponent-move")
		<< QString("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}

void tst_BoardScene::beforeMove()
{
	// Arrange
	QFETCH(QString, fen);
	// Pointer to the private Board variable in BoardScene
	// Gets automatically clean by BoardScene since it takes responsibility
	Chess::Board* chessBoard(Chess::BoardFactory::create("standard"));

	if (chessBoard == nullptr)
		QFAIL("Failed to create standard chess board");
	QVERIFY(chessBoard->setFenString(fen));

	BoardScene boardScene;

	// Act
	boardScene.setBoard(chessBoard);
	boardScene.populate();

	// Assert
	verifySynchronization(boardScene);
}

void tst_BoardScene::selfMove_data()
{
	QTest::addColumn<QString>("fen");
	QTest::addColumn<QString>("move");
	QTest::addColumn<Chess::Result::Type>("expectedResult");

	// Values provided by ChatGPT: "Give me FEN values and move string
	// for the following scenerios: no capture white player with pending game, no capture black
	// player with pending game, capture white player with pending game, capture black player with
	// pending game, capture black player piece with winning game, capture white player piece with winning game"

	QTest::newRow("white-no-capture-pending")
		<< QString("7k/8/8/8/8/8/4P3/7K w - - 0 1")
		<< QString("e2e4")
		<< Chess::Result::NoResult;

	QTest::newRow("black-no-capture-pending")
		<< QString("7k/4p3/8/8/8/8/8/7K b - - 0 1")
		<< QString("e7e5")
		<< Chess::Result::NoResult;

	QTest::newRow("white-no-capture-win")
		<< QString("7k/8/5KQ1/8/8/8/8/8 w - - 0 1")
		<< QString("g6g7")
		<< Chess::Result::Win;

	QTest::newRow("black-no-capture-win")
		<< QString("8/8/8/8/8/5kq1/8/7K b - - 0 1")
		<< QString("g3g2")
		<< Chess::Result::Win;

	QTest::newRow("white-capture-pending")
		<< QString("7k/8/8/3p4/4P3/8/8/7K w - - 0 1")
		<< QString("e4d5")
		<< Chess::Result::NoResult;

	QTest::newRow("black-capture-pending")
		<< QString("7k/8/8/3p4/4P3/8/8/7K b - - 0 1")
		<< QString("d5e4")
		<< Chess::Result::NoResult;

	QTest::newRow("black-capture-win")
		<< QString("8/8/8/8/8/5kq1/6R1/7K b - - 0 1")
		<< QString("g3g2")
		<< Chess::Result::Win;

	QTest::newRow("white-capture-win")
		<< QString("7k/6r1/5KQ1/8/8/8/8/8 w - - 0 1")
		<< QString("g6g7")
		<< Chess::Result::Win;
}

void tst_BoardScene::selfMove()
{
	// Arrange
	QFETCH(QString, fen);
	QFETCH(QString, move);
	QFETCH(Chess::Result::Type, expectedResult);

	Chess::Board* chessBoard(Chess::BoardFactory::create("standard"));

	if(chessBoard == nullptr)
		QFAIL("Failed to create standard chess board");

	QVERIFY(chessBoard->setFenString(fen));
	BoardScene boardScene;
	boardScene.setBoard(chessBoard);
	boardScene.populate();

	Chess::Move chessMove = chessBoard->moveFromString(move);
	QVERIFY(!chessMove.isNull());

	// Act
	boardScene.makeMove(chessMove);

	// Assert
	verifySynchronization(boardScene);
	QCOMPARE(chessBoard->result().type(), expectedResult);

}

QTEST_MAIN(tst_BoardScene)

#include "tst_boardscene.moc"