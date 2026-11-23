#include <QtTest/QTest>

#include <QCoreApplication>
#include <QIODevice>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>
#include <QSignalSpy>

#include <board/boardfactory.h>
#include <chessengine.h>
#include <chessgame.h>
#include <engineconfiguration.h>
#include <pgngame.h>
#include <timecontrol.h>

/**
 * @brief The MockEngineIODevice class Mocks the MockEngineIODevice
 * that a ChessEngine will use.
 * Ask EDU ChatGPT: "Build me a MockChessEngine for ChessEngine.h"
 */
class MockEngineIODevice: public QIODevice
{
public:
	MockEngineIODevice()
	{
		open(QIODevice::ReadWrite);
	}

	bool isSequential() const override
	{
		return true;
	}

	void simulateCrash()
	{
		// ChessEngine connects to this signal to ChessPlayer::onCrashed()
		emit readChannelFinished();
	}

protected:
	qint64 readData(char* data, qint64 maxSize) override
	{
		Q_UNUSED(data);
		Q_UNUSED(maxSize);

		return 0;
	}
	qint64 writeData(const char* data, qint64 maxSize) override
	{
		Q_UNUSED(data);
		// Mock every command to be successfully written.
		return maxSize;
	}
};

/**
 * @brief The MockChessEngine class Mock class for ChessEngine in
 * order to avoid having a live engine running.
 * Ask EDU ChatGPT: "Build me a MockChessEngine for ChessEngine.h"
 */
class MockChessEngine: public ChessEngine
{
public:
	explicit MockChessEngine(bool respondToPing = true)
		: m_device(new MockEngineIODevice()), m_respondToPing(respondToPing)
	{
		setDevice(m_device);
		addVariant("standard");
	}
	void setRespondToPing(bool respond){m_respondToPing = respond;}
	void simulateCrash(){m_device->simulateCrash();}
	QString protocol() const override{return "mock";}
	void makeMove(const Chess::Move& move) override{Q_UNUSED(move);}
protected:
	void startProtocol() override
	{
		QTimer::singleShot(0, this, [this](){
			onProtocolStart();
		});

	}
	void startGame() override{}
	void startThinking() override{
		if(!m_respondToPing)
			return;
		// White moves, necessary for black to go next
		QTimer::singleShot(0, this, [this]()
		{
			const Chess::Move move = board()->moveFromGenericMove(
				Chess::GenericMove(Chess::Square(4,1), Chess::Square(4,3), 0)
				);

			emitMove(move);
		});

	}
	void parseLine(const QString& line) override {Q_UNUSED(line);}
	bool sendPing() override
	{
		if(m_respondToPing)
		{
			QTimer::singleShot(0, this, [this](){
				pong();
			});
		}
		return true;
	}
	void sendStop() override {}
	void sendQuit() override {}
	void sendOption(const QString& name, const QVariant& value) override
	{
		Q_UNUSED(name);
		Q_UNUSED(value);
	}

private:
	MockEngineIODevice* m_device;
	bool m_respondToPing;

};

class tst_ChessEngine: public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void frozenEngineForfeitsGame_data();
	void frozenEngineForfeitsGame();
	void crashedEngineForfeitsGame_data();
	void crashedEngineForfeitsGame();
private:
	void startEngine(MockChessEngine& engine, const QString& name);
	QTemporaryDir m_settingsDir;
};

void tst_ChessEngine::initTestCase()
{
	QVERIFY(m_settingsDir.isValid());

	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings::setPath(
		QSettings::IniFormat,
		QSettings::UserScope,
		m_settingsDir.path()
		);

	QSettings settings;
	settings.beginGroup("Timing");
	settings.setValue("PingTimeout", 100);
	settings.setValue("QuitTimeout", 100);
	settings.setValue("IdleTimeout", 100);
	settings.setValue("ProtocolStartTimeout", 100);
	settings.endGroup();
	settings.sync();
}
void tst_ChessEngine::startEngine(MockChessEngine& engine, const QString& name)
{
	EngineConfiguration config;
	config.setName(name);
	config.setTimeoutScale(1.0);
	engine.applyConfiguration(config);
	engine.start();

}
void tst_ChessEngine::frozenEngineForfeitsGame_data()
{
	QTest::addColumn<Chess::Side>("frozenSide");
	QTest::addColumn<Chess::Side>("expectedWinner");
	QTest::addColumn<Chess::Side>("expectedLoser");

	QTest::newRow("white-engine-freezes")
		<<Chess::Side(Chess::Side::White)
		<<Chess::Side(Chess::Side::Black)
		<<Chess::Side(Chess::Side::White);

	QTest::newRow("black-engine-freezes")
		<<Chess::Side(Chess::Side::Black)
		<<Chess::Side(Chess::Side::White)
		<<Chess::Side(Chess::Side::Black);

}

void tst_ChessEngine::frozenEngineForfeitsGame()
{
	// Arrange
	QFETCH(Chess::Side, frozenSide);
	QFETCH(Chess::Side, expectedWinner);
	QFETCH(Chess::Side, expectedLoser);
	PgnGame pgn;

	MockChessEngine whiteEngine;
	MockChessEngine blackEngine;

	// Act
	// Set engine to not respond when called (simulates frozen)
	if(frozenSide == Chess::Side::White)
		whiteEngine.setRespondToPing(false);
	else
		blackEngine.setRespondToPing(false);

	startEngine(whiteEngine,  "White Engine");
	startEngine(blackEngine, "Black Engine");

	// Assert that both players are idle before frozen engine starts game
	QTRY_COMPARE(whiteEngine.state(), ChessPlayer::Idle);
	QTRY_COMPARE(blackEngine.state(), ChessPlayer::Idle);

	Chess::Board* board = Chess::BoardFactory::create("standard");
	QVERIFY(board != nullptr);

	ChessGame game(board, &pgn);

	game.setPlayer(Chess::Side::White, &whiteEngine);
	game.setPlayer(Chess::Side::Black, &blackEngine);

	// Go ahead in time when timeout is reached so we dont wait
	TimeControl timeControl("60");
	QVERIFY(timeControl.isValid());
	game.setTimeControl(timeControl);

	// Spy to check if game is finished
	QSignalSpy finishedSpy(
		&game,
		&ChessGame::finished
		);

	game.start();


	// Assert
	// Asserts the game has ended after the chess engine freezes
	QTRY_VERIFY_WITH_TIMEOUT(game.isFinished(), 1000);
	QTRY_COMPARE_WITH_TIMEOUT(finishedSpy.count(), 1, 1000);
	QVERIFY(game.isFinished());

	// Assert game ended due to a chess engine froze
	QCOMPARE(
		game.result().type(),
		Chess::Result::StalledConnection
		);

	// Asserts the frozen chess engine game is marked as a lost
	QCOMPARE(
		game.result().winner(),
		expectedWinner
		);
	QCOMPARE(
		game.result().loser(),
		expectedLoser
		);
}
void tst_ChessEngine::crashedEngineForfeitsGame_data()
{

}
void tst_ChessEngine::crashedEngineForfeitsGame()
{

}
QTEST_MAIN(tst_ChessEngine)
#include "tst_chessengine.moc"