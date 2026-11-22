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

	void selfMove_data();
	void selfMove();

	void opponentMove_data();
	void opponentMove();

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

			Chess::Piece expected = board->pieceAt(square);
			Chess::Piece actual = graphicsBoard->pieceTypeAt(square);

			QCOMPARE(actual, expected);
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
}

void tst_BoardScene::selfMove()
{
}

void tst_BoardScene::opponentMove_data()
{
}

void tst_BoardScene::opponentMove()
{
}

QTEST_MAIN(tst_BoardScene)

#include "tst_boardscene.moc"