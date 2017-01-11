#ifndef GUIDISPLAY_H
#define GUIDISPLAY_H


#include <QMouseEvent>
#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QString>
#include <QVector>
#include "chessBoard.hpp"
#include "defs.hpp"

class QGridLayout;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QTimer;

#ifndef DEBUG
class BoardSquare:public QLabel
{
    Q_OBJECT
public:
    BoardSquare(std::string index):fIndex(index)
    {
        setAutoFillBackground(true);
    }

    void setImage(QImage image)
    {
        setPixmap(QPixmap::fromImage(image));
        setScaledContents(true);
    }

signals:
    void squareClicked(QString);

private:
    void mousePressEvent(QMouseEvent* env)
    {
        if(env->buttons()==Qt::LeftButton)
            emit squareClicked(QString(fIndex.c_str()));
    }
    std::string fIndex;
};


namespace Ui {
class GuiDisplay;
}

class GuiDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit GuiDisplay(QWidget *parent = 0);
    ~GuiDisplay();
    bool drawGame();
    void statusScreen(std::string,bool);
    void endGameScreen(std::string,std::string);
    void initializeScreen(std::string,std::string);
    void updateScreen(std::vector<std::vector<Position>*>*);

    bool requestColor();
    int requestPlayers();
    int requestPromotion();
    int requestDifficulty();
    std::string requestCommand();
    std::string requestName(int,bool retry=false);

signals:
    void nextToPlay();

public slots:
    void receiveIndex(QString);
    void nextPlayer();
    void monitorStringSize();

private slots:
    void about();
    void startUpSession();
    void interruptGamePlay();

private:
    GuiDisplay(const GuiDisplay&)=delete;
    GuiDisplay(GuiDisplay&&)=delete;
    GuiDisplay& operator=(const GuiDisplay&)=delete;
    GuiDisplay& operator=(const GuiDisplay&&)=delete;

    void setupChessBoard();
    void setupMainBoard();
    void clearMainBoard();
    void clearChessBoard();

    inline int indexSquare(int,int);

    Ui::GuiDisplay *ui;

    //handled by constructor and destructor
    QVBoxLayout* fMain_Screen_Layout;
    QPushButton* fNewGame_Button;
    QPushButton* fAbout_Button;
    QPushButton* fExit_BUtton;

    //handled by chessBoard
    std::vector<std::vector<Position>*>* fOld_Board;
    QVector<BoardSquare*>* fSquares;
    QVector<QImage>* fWhite_Pieces;
    QVector<QImage>* fBlack_Pieces;
    QImage* NOIMAGE;

    //handled by mainBoard
    QWidget* fMain_Board;
    QGridLayout* fchessBoard;
    QVBoxLayout* fMain_Board_Layout;
    QLabel* fWhite_Status;
    QLabel* fBlack_Status;
    QPushButton* fQuit_Button;

    //handled by constructor and destructor
    std::string* fCommand;

    //handled by initializeScreen and clearMainBoard
    std::string* fPlayer1_Name;
    std::string* fPlayer2_Name;

    chessBoard* fChess_Engine;
    QTimer* fString_Time_Monitor;
    bool fStatus;
};

inline int GuiDisplay::indexSquare(int x,int y)
{
    return (y*8)+x;
}

#endif

#endif // GUIDISPLAY_H
