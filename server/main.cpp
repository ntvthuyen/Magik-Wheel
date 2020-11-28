#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickView>
#include <guicontroller.h>
#include "database.h"
#include <nlohmann/json.hpp>
#include <bits/stdc++.h>
#include "gamecontroller.h"
#include <chrono>
#include <thread>


using namespace std;

using json = nlohmann::json;
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds


void GameLogicTest() {

    // 0-index player
    vector<int> playerOrder{0, 2, 1, 3};

    Database database = Database("/home/lightkeima/Courses/cs493/Magik-Wheel/server/database/database.txt");
    // Number of player
    int N = 4;

    GameController gameController = GameController(N, database);

    // Test get masked kw
    string maskedKw = gameController.getMaskedKeyword();
    cout << "Masked KW: " << maskedKw << endl;

    // Test get hint
    string hint  = gameController.getHint();
    cout << "Hint: " << hint << endl;

    // Test player guess keyword from the first turn

    int idx = 0;

    int currentPlayer = playerOrder[idx];

    int status = gameController.processPlayerAnswer(currentPlayer, "Test keyword");

    cout << "Trying to guess keyword at the begining! Return status (should = -1) = " << status << endl;

    // Trying guess characters
    status = gameController.processPlayerAnswer(currentPlayer, gameController.getKeyword()[0]);

    cout << "Correct character guess. Status (should = 1) = " << status << endl;
    cout << "There are " << gameController.getCorrectCharGuessPosition().size() << " occurrence" << endl;

    maskedKw = gameController.getMaskedKeyword();
    cout << "Masked KW: " << maskedKw << endl;

    cout << "Score for all players" << endl;
    vector<int> scores = gameController.getPlayerScore();

    for (int i = 0; i < N; i++) {
        cout << "Player " << i << ": " << scores[i] << endl;
    }

    // Next player
    idx++;

    currentPlayer = playerOrder[idx];

    // Try to guess keyword and fail

    status = gameController.processPlayerAnswer(currentPlayer, "Test keyword");

    cout << "Trying to guess keyword and fail! Return status (should = 0) = " << status << endl;
    cout << "This player is " << (gameController.isDisqualifiedPlayer(currentPlayer) ? "disqualified" : "not disqualifed") << endl;

    // Guess correct keyword
    idx++;

    currentPlayer = playerOrder[idx];

    status = gameController.processPlayerAnswer(currentPlayer, gameController.getKeyword());
    cout << "Trying to guess keyword and succeed! Return status (should = 1) = " << status << endl;
    cout << "The keyword is " << gameController.getKeyword() << endl;

    cout << "Score for all players" << endl;
    scores = gameController.getPlayerScore();
    for (int i = 0; i < N; i++) {
        cout << "Player " << i << ": " << scores[i] << endl;
    }

    cout << "The game is " << (gameController.isEndGame() ? "finisehd" : "not finished") << endl;

    // Restart the game
    sleep_until(system_clock::now() + seconds(1));
    gameController.restart();

    cout << "New keyword and hint" << endl;
    cout << gameController.getMaskedKeyword() << endl;
    cout << gameController.getHint() << endl;

    // Check game is not end
    cout << "The game is " << (gameController.isEndGame() ? "finisehd" : "not finished") << endl;

    cout << "Scores after restart" << endl;
    scores = gameController.getPlayerScore();
    for (int i = 0; i < N; i++) {
        cout << "Player " << i << ": " << scores[i] << endl;
    }
}

int main(int argc, char *argv[])
{


    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    QQmlEngine xengine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QQmlComponent component(&xengine,
            url);
    QObject *object = component.create();


    GUIController guiController(object);

    vector<int> playerOrder{0, 2, 1, 3};

    Database database = Database("/home/lightkeima/Courses/cs493/Magik-Wheel/server/database/database.txt");
    // Number of player
    int N = 4;

    GameController gameController = GameController(N, database);
    guiController.CreatePlayerList(N);

    // Test get masked kw
    string maskedKw = gameController.getMaskedKeyword();
    cout << "Masked KW: " << gameController.getKeyword() << endl;
    guiController.SetWord(gameController.getKeyword());
    // Test get hint
    string hint  = gameController.getHint();
    cout << "Hint: " << hint << endl;
     guiController.ShowHint(hint);
    // Test player guess keyword from the first turn

    int idx = 0;

    int currentPlayer = playerOrder[idx];
    guiController.MarkPlayer(currentPlayer);
    int status = gameController.processPlayerAnswer(currentPlayer, 'a');
    vector<int> position_list(gameController.getCorrectCharGuessPosition());
    for(unsigned int i = 0; i < position_list.size(); ++i){
        cout << "aaaa" << i;
        guiController.ShowResultAtPosition(position_list[i]);
    }
    guiController.FlipCharacter('a');

    /*
    cout << "Trying to guess keyword at the begining! Return status (should = -1) = " << status << endl;
    // Trying guess characters
    status = gameController.processPlayerAnswer(currentPlayer, gameController.getKeyword()[0]);

    cout << "Correct character guess. Status (should = 1) = " << status << endl;
    cout << "There are " << gameController.getCorrectCharGuessPosition().size() << " occurrence" << endl;

    maskedKw = gameController.getMaskedKeyword();
    cout << "Masked KW: " << maskedKw << endl;

    cout << "Score for all players" << endl;
    vector<int> scores = gameController.getPlayerScore();

    for (int i = 0; i < N; i++) {
        cout << "Player " << i << ": " << scores[i] << endl;
    }

    // Next player
    idx++;

    currentPlayer = playerOrder[idx];

    // Try to guess keyword and fail

    status = gameController.processPlayerAnswer(currentPlayer, "Test keyword");

    cout << "Trying to guess keyword and fail! Return status (should = 0) = " << status << endl;
    cout << "This player is " << (gameController.isDisqualifiedPlayer(currentPlayer) ? "disqualified" : "not disqualifed") << endl;

    // Guess correct keyword
    idx++;

    currentPlayer = playerOrder[idx];

    status = gameController.processPlayerAnswer(currentPlayer, gameController.getKeyword());
    cout << "Trying to guess keyword and succeed! Return status (should = 1) = " << status << endl;
    cout << "The keyword is " << gameController.getKeyword() << endl;

    cout << "Score for all players" << endl;
    scores = gameController.getPlayerScore();
    for (int i = 0; i < N; i++) {
        cout << "Player " << i << ": " << scores[i] << endl;
    }

    cout << "The game is " << (gameController.isEndGame() ? "finisehd" : "not finished") << endl;

    // Restart the game
    sleep_until(system_clock::now() + seconds(1));
    gameController.restart();

    cout << "New keyword and hint" << endl;
    cout << gameController.getMaskedKeyword() << endl;
    cout << gameController.getHint() << endl;

    // Check game is not end
    cout << "The game is " << (gameController.isEndGame() ? "finisehd" : "not finished") << endl;

    cout << "Scores after restart" << endl;
    scores = gameController.getPlayerScore();
    for (int i = 0; i < N; i++) {
        cout << "Player " << i << ": " << scores[i] << endl;
    }
*/
    return app.exec();

}
