#include <QtTest/QTest>

#include <QFile>
#include <QTemporaryDir>

#include <pgngame.h>
#include <pgnstream.h>
#include <board/board.h>
#include <memory>
/**
 * @brief The tst_PgnGame class
 * PGN Value for Alexander Alekhine vs Oscar Tenner is from:
 * https://www.chessgames.com/perl/chessgame?gid=1076122
 * PGN Value for Jens Hohmeister vs Peter Wolff is from:
 * https://www.chessgames.com/perl/chessgame?gid=1259990
 */

class tst_PgnGame: public QObject {
	Q_OBJECT

private slots:
	/**
	 * @brief saveGameState_data
	 * The moves in a pgngame in QStringList,
	 * The QString result from the pgngame,
	 * The QString expected value written in the pgn file.
	 */
	void saveGameState_data();
	/**
	 * @brief saveGameState
	 * Test if the current game state in pgngame has the same value in the
	 * written file.
	 */
	void saveGameState();
private:
	/**
	 * @brief addMoves Adds moves to a game state in pgngame that
	 * has the passed move string performed sequentially.
	 * @param game The pgngame state to add the moves to.
	 * @param moveStrings The moves to be added in the pgngame state.
	 */
	void addMoves(PgnGame& game, const QStringList& moveStrings);
	/**
	 * @brief compareMoves Asserts that the pgngame from a saved file
	 * has the same game history as the expected moves
	 * @param actual The PgnGame state from a saved file.
	 * @param expected The original PgnGame state before saving the file.
	 */
	void compareMoves(const PgnGame& actual, const PgnGame& expected);

};

void tst_PgnGame::addMoves(PgnGame& game, const QStringList& moveStrings)
{
	std::unique_ptr<Chess::Board> board(game.createBoard());
	QVERIFY(board != nullptr);

	// Iterate through all moves and add them to pgngame
	for(const QString& moveString: moveStrings){
		const Chess::Move move =  board->moveFromString(moveString);
		QVERIFY2(
			!move.isNull(),
			qPrintable(QString("Invalid move %1").arg(moveString))
			);
		const PgnGame::MoveData moveData = {
			board->key(),
			board->genericMove(move),
			moveString,
			QString()
		};
		game.addMove(moveData, false);
		board->makeMove(move);
	}
}


void tst_PgnGame::compareMoves(const PgnGame& actual, const PgnGame& expected)
{
	QCOMPARE(actual.moves().size(), expected.moves().size());
	for(size_t i = 0; i < expected.moves().size(); ++i){
		QCOMPARE(
			actual.moves().at(i).moveString,
			expected.moves().at(i).moveString
			);
	}
}


void tst_PgnGame::saveGameState_data()
{
	QTest::addColumn<QStringList>("moves");
	QTest::addColumn<QString>("result");
	QTest::addColumn<QString>("expectedSnippet");

	// Starting games
	QTest::newRow("starting-game")
		<<QStringList{}
		<< QString("*")
		<< QString("*");
	QTest::newRow("first-move-e4")
		<<QStringList{"e4"}
		<< QString("*")
		<< QString("*");

	// Mid-Games
	QTest::newRow("mid-game-alexander-alekhine-vs-oscar-tenner")
		<<QStringList{
						 "e4", "e5",
						 "f4", "Bc5",
						 "Nf3", "d6",
						 "Nc3", "Nf6",
						 "Bc4", "Nc6",
						 "d3", "Bg4",
						 "Na4", "exf4",
						 "Nxc5", "dxc5",
	}
		<< QString("*")
		<< QString("*");
	QTest::newRow("mid-game-jens-hohmeister-vs-peter-wolf")
		<<QStringList{
			   "e4", "e5",
			   "Nf3", "Nc6",
			   "Bb5", "a6",
			   "Ba4", "Nf6",
			   "O-O", "Nxe4",
			   "Re1", "d5",
	}
		<< QString("*")
		<< QString("*");

	// Near-End-Games
	QTest::newRow("near-end-game-alexander-alekhine-vs-oscar-tenner")
		<<QStringList{
			 "e4", "e5",
			 "f4", "Bc5",
			 "Nf3", "d6",
			 "Nc3", "Nf6",
			 "Bc4", "Nc6",
			 "d3", "Bg4",
			 "Na4", "exf4",
			 "Nxc5", "dxc5",
			   "Bxf4", "Nh5",
			   "Be3", "Ne5",
			   "Nxe5", "Bxd1",
			   "Bxf7+", "Ke7",
		   }
		<< QString("*")
		<< QString("*");
	QTest::newRow("near-end-game-jens-hohmeister-vs-peter-wolf")
		<<QStringList{
			   "e4", "e5",
			   "Nf3", "Nc6",
			   "Bb5", "a6",
			   "Ba4", "Nf6",
			   "O-O", "Nxe4",
			   "Re1", "d5",
			   "Nxe5", "Bd6",
			   "Nxc6", "Bxh2+",
			   "Kxh2", "Qh4+",
	}
		<< QString("*")
		<< QString("*");

	// End-Games
	QTest::newRow("end-game-alexander-alekhine-vs-oscar-tenner")
		<<QStringList{
			 "e4", "e5",
			 "f4", "Bc5",
			 "Nf3", "d6",
			 "Nc3", "Nf6",
			 "Bc4", "Nc6",
			 "d3", "Bg4",
			 "Na4", "exf4",
			 "Nxc5", "dxc5",
			   "Bxf4", "Nh5",
			   "Be3", "Ne5",
			   "Nxe5", "Bxd1",
			   "Bxf7+", "Ke7",
			   "Bxc5+", "Kf6",
			   "O-O+", "Kxe5",
			   "Rf5#"
		   }
		<< QString("1-0")
		<< QString("1-0");

	QTest::newRow("end-game-jens-hohmeister-vs-peter-wolf")
		<<QStringList{
			   "e4", "e5",
			   "Nf3", "Nc6",
			   "Bb5", "a6",
			   "Ba4", "Nf6",
			   "O-O", "Nxe4",
			   "Re1", "d5",
			   "Nxe5", "Bd6",
			   "Nxc6", "Bxh2+",
			   "Kxh2", "Qh4+",
			   "Kg1", "Qxf2+"
	}
		<< QString("1/2-1/2")
		<< QString("1/2-1/2");
}

void tst_PgnGame::saveGameState(){
	// Arrange
	PgnGame originalGame;

	QFETCH(QStringList, moves);
	QFETCH(QString, result);
	QFETCH(QString, expectedSnippet);

	originalGame.setTag("Event", "Persistence Test");
	originalGame.setTag("White", "Bob");
	originalGame.setTag("Black", "Jake");
	originalGame.setTag("Result", result);

	// Arrange game history and game state
	addMoves(originalGame, moves);

	// Arrange temporary directory to save and read file
	QTemporaryDir tempDir;
	QVERIFY(tempDir.isValid());

	// Arrange file path
	const QString filePath = tempDir.filePath("saved-game.pgn");

	// Act
	QVERIFY(originalGame.write(filePath));


	// Assert
	// Assert file was opened.
	QFile file(filePath);
	QVERIFY(file.open(QIODevice::ReadOnly));

	// Assert file contains expected pgn contents
	const QString contents = QString::fromUtf8(file.readAll());
	QVERIFY(contents.contains(expectedSnippet));
	QVERIFY(file.seek(0));

	// Read file and convert to a pgngame to compare to the original game
	PgnStream stream(&file);
	PgnGame savedGame;

	// Assert saved game is read
	QVERIFY(savedGame.read(stream));

	// Assert saved game reflects the original game
	compareMoves(savedGame, originalGame);

	// Assert the correct result value was written
	QCOMPARE(
		savedGame.tagValue("Result"), result
		);

}

QTEST_MAIN(tst_PgnGame)
#include "tst_pgngame.moc"
