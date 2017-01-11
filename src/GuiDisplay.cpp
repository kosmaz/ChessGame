#include "GuiDisplay.hpp"
#include <QMessageBox>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QPalette>
#include <QDialog>
#include <QSound>
#include <QTimer>
#include <QIcon>
using namespace std;

#ifndef DEBUG

GuiDisplay::GuiDisplay(QWidget* parent):QWidget(parent)
{
    setFixedHeight(200);
    setFixedWidth(200);
    setWindowIcon(QIcon(":images/ICONS/ChessBoard.ico"));
    fNewGame_Button=new QPushButton(QIcon(":/images/ICONS/new.png"),tr("NEW GAME"),this);
    fNewGame_Button->setDefault(true);
    connect(fNewGame_Button,SIGNAL(clicked()),this,SLOT(startUpSession()));
    fAbout_Button=new QPushButton(QIcon(":/images/ICONS/about.png"),tr("ABOUT"),this);
    connect(fAbout_Button,SIGNAL(clicked()),this,SLOT(about()));
    fExit_BUtton=new QPushButton(QIcon(":/images/ICONS/exit.png"),tr("EXIT"),this);
    connect(fExit_BUtton,SIGNAL(clicked()),this,SLOT(close()));

    fMain_Screen_Layout=new QVBoxLayout(this);
    fMain_Screen_Layout->addWidget(fNewGame_Button);
    fMain_Screen_Layout->addWidget(fAbout_Button);
    fMain_Screen_Layout->addWidget(fExit_BUtton);
    setLayout(fMain_Screen_Layout);

    fCommand=new string("    ");
    NOIMAGE=new QImage(":/images/ICONS/NOIMAGE.png");
    fStatus=false;

}


GuiDisplay::~GuiDisplay()
{
    delete fNewGame_Button;
    delete fAbout_Button;
    delete fExit_BUtton;
    delete fMain_Screen_Layout;
    delete fCommand;
    delete NOIMAGE;
}


bool GuiDisplay::drawGame()
{
    bool draw=false;
    int ret=QMessageBox::question(fMain_Board,tr(""),tr("Do you want to draw the game?"),QMessageBox::Yes | QMessageBox::No,QMessageBox::Yes);

    switch(ret)
    {
        case QMessageBox::Yes: draw=true;
            break;
        case QMessageBox::No: draw=false;
            break;
    }
    return draw;
}


void GuiDisplay::statusScreen(string message, bool side)
{
    message.insert(0,1,'\t');
    if(side==WHITE)
    {
        string richtext1("<pre><b><h3><i><font color=white>");
        string richtext2("</i></font>");
        string richtext3(richtext1+*fPlayer1_Name+richtext2);
        string richtext4("<font color=red>");
        string richtext5("</h3></b></font></pre>");
        string richtext6("<font color=white>");
        string richtext7("</h3></b></font></pre>");
         string richtext8;
        if(message=="\tYOUR TURN")
            richtext8=richtext6+message+richtext7;
        else
            richtext8=richtext4+message+richtext5;

        QString part1(richtext3.c_str());
        QString part2(richtext8.c_str());
        QString combine(part1+"                          "+part2);
        fWhite_Status->setText(tr(combine.toStdString().c_str()));
    }
    else
    {
        string richtext1("<pre><b><h3><i><font color=white>");
        string richtext2("</i></font>");
        string richtext3(richtext1+*fPlayer2_Name+richtext2);
        string richtext4("<font color=red>");
        string richtext5("</h3></b></font></pre>");
        string richtext6("<font color=white>");
        string richtext7("</h3></b></font></pre>");
         string richtext8;
        if(message=="\t"
                "YOUR TURN")
            richtext8=richtext6+message+richtext7;
        else
            richtext8=richtext4+message+richtext5;

        QString part1(richtext3.c_str());
        QString part2(richtext8.c_str());
        QString combine(part1+"                          "+part2);
        fBlack_Status->setText(tr(combine.toStdString().c_str()));
    }
    return;
}


void GuiDisplay::endGameScreen(string message1, string message2)
{
    QString str1(message1.c_str());
    QString str2(message2.c_str());
    QString combine(str1+"\n\n"+str2);
    QMessageBox::information(fMain_Board,tr("GAME OVER"),tr(combine.toStdString().c_str()));
    clearChessBoard();
    clearMainBoard();
    fChess_Engine->cleanUp();
    delete fChess_Engine;
    if(fString_Time_Monitor->isActive())
        fString_Time_Monitor->stop();
    delete fString_Time_Monitor;
    return;
}


void GuiDisplay::initializeScreen(string player1, string player2)
{
    fPlayer1_Name=new string(player1);
    fPlayer2_Name=new string(player2);
    setupChessBoard();
    setupMainBoard();
    return;
}


void GuiDisplay::updateScreen(std::vector<std::vector<Position>*>* board)
{
    for(int i=0; i<8; ++i)
        for(int j=0; j<8; ++j)
            if(board->at(i)->at(j).value!=fOld_Board->at(i)->at(j).value || board->at(i)->at(j).color!=fOld_Board->at(i)->at(j).color)
            {
                if(board->at(i)->at(j).color==WHITE)
                {
                    switch(board->at(i)->at(j).value)
                    {
                        case -1: fSquares->at(indexSquare(i,j))->setImage(*NOIMAGE);
                            break;
                        case 0: fSquares->at(indexSquare(i,j))->setImage(fWhite_Pieces->at(0));
                            break;
                        case 1: fSquares->at(indexSquare(i,j))->setImage(fWhite_Pieces->at(1));
                            break;
                        case 2: fSquares->at(indexSquare(i,j))->setImage(fWhite_Pieces->at(2));
                            break;
                        case 3: fSquares->at(indexSquare(i,j))->setImage(fWhite_Pieces->at(3));
                            break;
                        case 4: fSquares->at(indexSquare(i,j))->setImage(fWhite_Pieces->at(4));
                            break;
                        case 5: fSquares->at(indexSquare(i,j))->setImage(fWhite_Pieces->at(5));
                            break;
                    }
                }
                else
                {
                    switch(board->at(i)->at(j).value)
                    {
                        case -1: fSquares->at(indexSquare(i,j))->setImage(*NOIMAGE);
                            break;
                        case 0: fSquares->at(indexSquare(i,j))->setImage(fBlack_Pieces->at(0));
                            break;
                        case 1: fSquares->at(indexSquare(i,j))->setImage(fBlack_Pieces->at(1));
                            break;
                        case 2: fSquares->at(indexSquare(i,j))->setImage(fBlack_Pieces->at(2));
                            break;
                        case 3: fSquares->at(indexSquare(i,j))->setImage(fBlack_Pieces->at(3));
                            break;
                        case 4: fSquares->at(indexSquare(i,j))->setImage(fBlack_Pieces->at(4));
                            break;
                        case 5: fSquares->at(indexSquare(i,j))->setImage(fBlack_Pieces->at(5));
                            break;
                    }
                }
                if(fOld_Board->at(i)->at(j).value!=-2 && board->at(i)->at(j).value!=-1)
                    QSound::play(":/sounds/move.wav");
                fOld_Board->at(i)->at(j).value=board->at(i)->at(j).value;
                fOld_Board->at(i)->at(j).color=board->at(i)->at(j).color;
            }
    return;
}


bool GuiDisplay::requestColor()
{
    bool color=WHITE;
    QMessageBox reqcol(this);
#if defined (__linux__)
    QPushButton* white=reqcol.addButton(tr("WHITE"),QMessageBox::NoRole);
    QPushButton* black=reqcol.addButton(tr("BLACK"),QMessageBox::YesRole);
#elif defined (__WIN32__) || defined (__WIN64__)
    QPushButton* white=reqcol.addButton(tr("WHITE"),QMessageBox::YesRole);
    QPushButton* black=reqcol.addButton(tr("BLACK"),QMessageBox::NoRole);
#endif

    if(reqcol.layout())
        delete reqcol.layout();
    QVBoxLayout layout(&reqcol);
    layout.addWidget(white);
    layout.addWidget(black);
    reqcol.exec();

    if(reqcol.clickedButton()==white)
        color=WHITE;
    else if(reqcol.clickedButton()==black)
        color=BLACK;

    return color;
}


int GuiDisplay::requestPlayers()
{
    int players=0;
    QMessageBox reqpla(this);
    QPushButton* hh=reqpla.addButton(tr("HUMAN VS HUMAN"),QMessageBox::YesRole);
    QPushButton* ha=reqpla.addButton(tr("HUMAN VS A.I"),QMessageBox::AcceptRole);
    QPushButton* aa=reqpla.addButton(tr("A.I VS A.I"),QMessageBox::NoRole);

    if(reqpla.layout())
        delete reqpla.layout();
    QVBoxLayout layout(&reqpla);
    layout.addWidget(hh);
    layout.addWidget(ha);
    layout.addWidget(aa);
    reqpla.exec();

    if(reqpla.clickedButton()==hh)
        players=1;
    else if(reqpla.clickedButton()==ha)
        players=2;
    else if(reqpla.clickedButton()==aa)
        players=3;

    return players;
}


int GuiDisplay::requestPromotion()
{
    int promote=0;    
    QMessageBox reqpro(fMain_Board);
#if defined (__linux__)
    QPushButton* knight=reqpro.addButton(tr("KNIGHT"),QMessageBox::YesRole);
    QPushButton* bishop=reqpro.addButton(tr("BISHOP"),QMessageBox::NoRole);
    QPushButton* rook=reqpro.addButton(tr("ROOK"),QMessageBox::AcceptRole);
    QPushButton* queen=reqpro.addButton(tr("QUEEN"),QMessageBox::RejectRole);
#elif defined (__WIN32__) || (__WIN64__)
    QPushButton* knight=reqpro.addButton(tr("KNIGHT"),QMessageBox::NoRole);
    QPushButton* bishop=reqpro.addButton(tr("BISHOP"),QMessageBox::RejectRole);
    QPushButton* rook=reqpro.addButton(tr("ROOK"),QMessageBox::YesRole);
    QPushButton* queen=reqpro.addButton(tr("QUEEN"),QMessageBox::AcceptRole);
#endif

    if(reqpro.layout())
        delete reqpro.layout();
    QVBoxLayout layout(&reqpro);
    layout.addWidget(knight);
    layout.addWidget(bishop);
    layout.addWidget(rook);
    layout.addWidget(queen);
    reqpro.exec();

    if(reqpro.clickedButton()==knight)
        promote=1;
    else if(reqpro.clickedButton()==bishop)
        promote=2;
    else if(reqpro.clickedButton()==rook)
        promote=3;
    else if(reqpro.clickedButton()==queen)
        promote=4;

    return promote;
}


int GuiDisplay::requestDifficulty()
{
    int difficulty=0;
    QMessageBox reqdif(this);
#if defined (__linux__)
    QPushButton* beginner=reqdif.addButton(tr("BEGINNER"),QMessageBox::RejectRole);
    QPushButton* intermediate=reqdif.addButton(tr("INTERMEDIATE"),QMessageBox::AcceptRole);
    QPushButton* advanced=reqdif.addButton(tr("ADVANCED"),QMessageBox::NoRole);
    QPushButton* expert=reqdif.addButton(tr("EXPERT"),QMessageBox::YesRole);
#elif defined (_WIN32) || defined (_WIN64)
    QPushButton* beginner=reqdif.addButton(tr("BEGINNER"),QMessageBox::AcceptRole);
    QPushButton* intermediate=reqdif.addButton(tr("INTERMEDIATE"),QMessageBox::NoRole);
    QPushButton* advanced=reqdif.addButton(tr("ADVANCED"),QMessageBox::RejectRole);
    QPushButton* expert=reqdif.addButton(tr("EXPERT"),QMessageBox::YesRole);
#endif

    if(reqdif.layout())
        delete reqdif.layout();
    QVBoxLayout layout(&reqdif);
    layout.addWidget(beginner);
    layout.addWidget(intermediate);
    layout.addWidget(advanced);
    layout.addWidget(expert);
    reqdif.exec();

    if(reqdif.clickedButton()==beginner)
        difficulty=2;
    else if(reqdif.clickedButton()==intermediate)
        difficulty=4;
    else if(reqdif.clickedButton()==advanced)
        difficulty=6;
    else if(reqdif.clickedButton()==expert)
        difficulty=7;

    return difficulty;
}


std::string GuiDisplay::requestCommand()
{
    return *fCommand;
}


std::string GuiDisplay::requestName(int player,bool retry)
{
    string name;

    QDialog dialog(this);
    QLabel label(&dialog);
    QLineEdit edit(&dialog);
    QPushButton ok(tr("OK"),&dialog);
    ok.setFixedWidth(50);
    edit.setMaxLength(15);
    edit.setFixedWidth(200);
    connect(&ok,SIGNAL(clicked()),&dialog,SLOT(accept()));

    QVBoxLayout layout;
    dialog.setLayout(&layout);
    dialog.layout()->addWidget(&label);
    dialog.layout()->addWidget(&edit);
    dialog.layout()->addWidget(&ok);

    if(player==2 && retry)
    {
        label.setText(tr("The same name cannot be used for both players.\nPlease input a new name for player 2"));
        while(!name.size() || name.substr(0,8)=="Lauretta")
        {
            dialog.exec();
            name=edit.text().toStdString();
        }
    }
    else if(player==0)
    {
        label.setText(tr("Please type in your display name\nfor this game session below"));
        while(!name.size() || name.substr(0,8)=="Lauretta")
        {
            dialog.exec();
            name=edit.text().toStdString();
        }
    }
    else
    {
        QString str("Please type in your display name for Player ");
        str.append(player+'0');
        str.append(" below");
        label.setText(tr(str.toStdString().c_str()));
        while(!name.size() || name.substr(0,8)=="Lauretta")
        {
            dialog.exec();
            name=edit.text().toStdString();
        }

    }
    return name;
}


void GuiDisplay::receiveIndex(QString index)
{
    *fCommand+=index.toStdString().c_str();
    return;
}


void GuiDisplay::nextPlayer()
{
    if(fChess_Engine->whoseTurn()==WHITE && *fPlayer1_Name!="Lauretta 1" && *fPlayer1_Name!="Lauretta")
    {
        *fCommand="";
        if(fStatus)
            statusScreen("",BLACK);
        if(!fChess_Engine->isPlayerInCheck())
            statusScreen("YOUR TURN",WHITE);
        fStatus=true;
        fString_Time_Monitor->start();
    }
    else if(fChess_Engine->whoseTurn()==BLACK && *fPlayer2_Name!="Lauretta 2" && *fPlayer2_Name!="Lauretta")
    {
        *fCommand="";
        if(fStatus)
            statusScreen("",WHITE);
        if(!fChess_Engine->isPlayerInCheck())
            statusScreen("YOUR TURN",BLACK);
        fStatus=true;
        fString_Time_Monitor->start();
    }
    else
    {
        fChess_Engine->setIsPlayerInCheck(false);
        if(fStatus)
        {
            statusScreen("",fChess_Engine->whoseTurn()^1);
            QTimer::singleShot(300,this,SIGNAL(nextToPlay()));
            fStatus=false;
        }
        else
            emit nextToPlay();
    }
    return;
}


void GuiDisplay::monitorStringSize()
{
    if(fCommand->size()==4)
    {
        if(fChess_Engine->player()->checkCommand(*fCommand))
        {
            fChess_Engine->setIsPlayerInCheck(false);
            fString_Time_Monitor->stop();
            emit nextToPlay();
        }
    }
    else if(fCommand->size()>4)
        *fCommand="";
    return;
}


void GuiDisplay::about()
{
    QMessageBox::about(this,tr("About"),tr("\tCHESS GAME\n"
                                           "chess Engine used:  Lauretta  version 0.1.0.0\n\n\n"
                                           "A.I TIME TAKEN FOR EACH DIFFICULTY:\n\n"
                                           "BEGINNER MODE:\tNEGLIGIBLE\n"
                                           "INTERMEDIATE MODE:\t~20 Seconds\n"
                                           "ADVANCED MODE:\t~1 Minute 30 Seconds\n"
                                           "EXPERT MODE:\t~3 Minutes 30 Seconds\n\n\n"
                                           "Developed and marketed by -k0$m@3- Inc.\n"
                                           "Contact us:\n08139278033\nkosmaz2009@yahoo.com"));
    return;
}


void GuiDisplay::startUpSession()
{
    *fCommand="    ";
    fString_Time_Monitor=new QTimer;
    fString_Time_Monitor->setInterval(1);
    connect(fString_Time_Monitor,SIGNAL(timeout()),this,SLOT(monitorStringSize()));
    fChess_Engine=new chessBoard (this);
    fChess_Engine->initialize();
    connect(this,SIGNAL(nextToPlay()),fChess_Engine,SLOT(moderator()));
    connect(fChess_Engine,SIGNAL(played()),this,SLOT(nextPlayer()));
    QTimer::singleShot(4000,fChess_Engine,SLOT(moderator()));
    return;
}


void GuiDisplay::interruptGamePlay()
{
    clearChessBoard();
    clearMainBoard();
    fChess_Engine->cleanUp();
    delete fChess_Engine;
    if(fString_Time_Monitor->isActive())
        fString_Time_Monitor->stop();
    delete fString_Time_Monitor;
    return;
}


void GuiDisplay::setupChessBoard()
{
    fSquares=new QVector<BoardSquare*>;
    fWhite_Pieces=new QVector<QImage>;
    fBlack_Pieces=new QVector<QImage>;

    char alpha[8]={'a','b','c','d','e','f','g','h'};
    for(int i=0; i<8; ++i)
        for(int j=0; j<8; ++j)
        {
            string _string;
            _string=alpha[j];
            _string+='0'+(i+1);
            BoardSquare* square=new BoardSquare(_string);
            connect(square,SIGNAL(squareClicked(QString)),this,SLOT(receiveIndex(QString)));

            if((i+j)%2)
            {
                QPalette odd=square->palette();
                odd.setColor(QPalette::Window,QColor(255,127,036));
                square->setPalette(odd);
            }
            else
            {
                QPalette even=square->palette();
                even.setColor(QPalette::Window,QColor(255,211,155));
                square->setPalette(even);
            }

            fSquares->push_back(square);
        }
    for(int i=0; i<6; ++i)
    {
        QString path=":/images/WHITE_CHESS_PIECES/white";
        path.append('0'+i);
        path.append(".png");
        QImage image(path);
        image=image.scaled(300,300);
        fWhite_Pieces->push_back(image);
    }

    for(int j=0; j<6; ++j)
    {
        QString path=":/images/BLACK_CHESS_PIECES/black";
        path.append('0'+j);
        path.append(".png");
        QImage image(path);
        image=image.scaled(300,300);
        fBlack_Pieces->push_back(image);
    }

    fOld_Board=new vector<vector<Position>*>;

    for(int i=0; i<8; ++i)
    {
        vector<Position>* temp=new vector<Position>;
        for(int j=0; j<8; ++j)
            temp->push_back(Position(false,-2));
        fOld_Board->push_back(temp);
    }

    return;
}


void GuiDisplay::setupMainBoard()
{
    fchessBoard=new QGridLayout;
    fchessBoard->setSpacing(0);

    for(int i=0,layout=7; i<8; ++i,--layout)
        for(int j=0; j<8; ++j)
            fchessBoard->addWidget(fSquares->at(indexSquare(j,layout)),i,j);

    string richtext1("<b><h3><i><font color=white>");
    string richtext2("</b></h3></i></font>");

    string combine1(richtext1+*fPlayer1_Name+richtext2);
    QString name1(combine1.c_str());
    fWhite_Status=new QLabel;
    fWhite_Status->setMinimumWidth(500);
    fWhite_Status->setTextFormat(Qt::RichText);
    fWhite_Status->setText(name1);

    string combine2(richtext1+*fPlayer2_Name+richtext2);
    QString name2(combine2.c_str());
    fBlack_Status=new QLabel;
    fBlack_Status->setMinimumWidth(500);
    fBlack_Status->setTextFormat(Qt::RichText);
    fBlack_Status->setText(name2);

    fQuit_Button=new QPushButton(QIcon(":/images/ICONS/exit.png"),tr("QUIT GAME"),this);
    fQuit_Button->setFixedWidth(150);
    connect(fQuit_Button,SIGNAL(clicked()),this,SLOT(interruptGamePlay()));

    fMain_Board_Layout=new QVBoxLayout;
    fMain_Board_Layout->addWidget(fBlack_Status,1);
    fMain_Board_Layout->addLayout(fchessBoard,4);
    fMain_Board_Layout->addWidget(fWhite_Status,1);
    fMain_Board_Layout->addWidget(fQuit_Button,1);

    fMain_Board=new QWidget;
    fMain_Board->setWindowFlags(Qt::SplashScreen);
    fMain_Board->setFixedHeight(600);
    fMain_Board->setFixedWidth(800);
    fMain_Board->setWindowModality(Qt::ApplicationModal);
    fMain_Board->setAutoFillBackground(true);
    QPalette color=fMain_Board->palette();
    color.setColor(QPalette::Window,QColor(0,0,0));
    fMain_Board->setPalette(color);

    if(fMain_Board->layout())
        delete fMain_Board->layout();
    fMain_Board->setLayout(fMain_Board_Layout);
    fMain_Board->setWindowIcon(QIcon(":images/ICONS/ChessBoard.png"));
    fMain_Board->show();

    return;
}


void GuiDisplay::clearMainBoard()
{
    fMain_Board->hide();
    delete fchessBoard;
    delete fQuit_Button;
    delete fWhite_Status;
    delete fBlack_Status;
    delete fPlayer1_Name;
    delete fPlayer2_Name;
    delete fMain_Board_Layout;
    delete fMain_Board;
}

void GuiDisplay::clearChessBoard()
{
    for(int i=0; i<(int)fOld_Board->size(); ++i)
        delete fOld_Board->at(i);
    delete fOld_Board;

    for(int i=0; i<64; ++i)
        delete fSquares->at(i);
    delete fSquares;

    delete fWhite_Pieces;
    delete fBlack_Pieces;
    return;
}

#endif
