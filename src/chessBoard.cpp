#include "chessBoard.hpp"
#include "GuiDisplay.hpp"
#include <QSound>
#include <QTimer>
using namespace std;


chessBoard::chessBoard(GuiDisplay* screen):fTurn(WHITE),fPly_Count(0),fFifty_Moves(0),fSession(false),fPlayer1(nullptr),
    fPlayer2(nullptr),fMove_History(nullptr),fCaptured_Piece_Player1(nullptr),fCaptured_Piece_Player2(nullptr),fBoard(nullptr),
    fCheck(false),fStatus(false),fScreen(screen){}


chessBoard::~chessBoard()
{
    if(fPlayer1!=nullptr)
        delete fPlayer1;
    if(fPlayer2!=nullptr)
        delete fPlayer2;
    if(fMove_History!=nullptr)
        delete fMove_History;
    if(fCaptured_Piece_Player1!=nullptr)
        delete fCaptured_Piece_Player1;
    if(fCaptured_Piece_Player2!=nullptr)
        delete fCaptured_Piece_Player2;
}


void chessBoard::initialize()
{
    initializePlayers();
    initializeGuiDisplay();
    initializeBoard();
    initializeSession();
    return;
}


void chessBoard::cleanUp()
{
    if(fPlayer1!=nullptr)
    {
        delete fPlayer1;
        fPlayer1=nullptr;
    }

    if(fPlayer2!=nullptr)
    {
        delete fPlayer2;
        fPlayer2=nullptr;
    }
    if(fMove_History!=nullptr)
    {
        delete fMove_History;
        fMove_History=nullptr;
    }
    if(fCaptured_Piece_Player1!=nullptr)
    {
        delete fCaptured_Piece_Player1;
        fCaptured_Piece_Player1=nullptr;
    }

    if(fCaptured_Piece_Player2!=nullptr)
    {
        delete fCaptured_Piece_Player2;
        fCaptured_Piece_Player2=nullptr;
    }

    if(fBoard!=nullptr)
    {
        for(int i=0; i<(int)fBoard->size(); ++i)
            delete fBoard->at(i);
        delete fBoard;
        fBoard=nullptr;
    }
    return;
}


void chessBoard::initializePlayers()
{
    char choice='\0';
#ifdef DEBUG
#if defined (__linux__)
    system("clear");
#elif defined (_WIN32) || defined (_WIN64)
    system("cls");
#endif

    cout<<"Choose from the list given below\n";
    cout<<"1."<<'\t'<<"Human VS Human\n";
    cout<<"2."<<'\t'<<"Human VS A.I\n";
    cout<<"3."<<'\t'<<"A.I VS A.I\n";
    cin>>choice;
    while(choice<'1' || choice>'3')
        cin>>choice;
#else
    int ret=fScreen->requestPlayers();
    if(ret==1)
        choice='1';
    else if(ret==2)
        choice='2';
    else
        choice='3';
#endif
    switch(choice)
    {
    case '1':
        {
            fPlayer1=new Human(requestColor(),requestName(1),fScreen);
            fPlayer2=new Human((fPlayer1->myTurn()==0)? 1:0,requestName(2),fScreen);
            break;
        }
    case '2':
        {
            fPlayer1=new Human(requestColor(),requestName(0),fScreen);
            fPlayer2=new Lauretta((fPlayer1->myTurn()==0)? 1:0,fPlayer1);
#ifdef DEBUG
            char choice='\0';
            cout<<"Please choose the difficulty of this game session\n";
            cout<<"1.\tBEGINEER\n";
            cout<<"2.\tINTERMEDIATE\n";
            cout<<"3.\tADVANCED\n";
            cout<<"4.\tEXPERT\n";
            cin>>choice;
            while(choice<'1' || choice>'4')
                cin>>choice;

            switch(choice)
            {
                case '1': fPlayer2->setDifficulty(2);
                    break;
                case '2': fPlayer2->setDifficulty(4);
                    break;
                case '3': fPlayer2->setDifficulty(6);
                    break;
                case '4': fPlayer2->setDifficulty(7);
                    break;
            }
#else
            fPlayer2->setDifficulty(fScreen->requestDifficulty());
#endif
            break;
        }
    case '3':
        {
            fPlayer1=new Lauretta(WHITE,"Lauretta 1");
            fPlayer2=new Lauretta(BLACK,fPlayer1,"Lauretta 2");
            fPlayer1->setOpponent(fPlayer2);
            fPlayer1->setDifficulty(3);
            fPlayer2->setDifficulty(3);
        }
    default: break;
    }

#ifdef DEBUG
    if(choice=='2')
        cin.ignore();
#endif
    return;
}


void chessBoard::initializeBoard()
{
    fBoard=new vector<vector<Position>*>;

    for(int i=0; i<8; ++i)
    {
        vector<Position>* temp=new vector<Position>;
        for(int j=0; j<8; ++j)
            temp->push_back(Position(false,-1));
        fBoard->push_back(temp);
    }
    updateBoard();
    return;
}


void chessBoard::initializeGuiDisplay()
{
#ifndef DEBUG
    Player* white=(fPlayer1->myTurn()==WHITE)? fPlayer1:fPlayer2;
    Player* black=(fPlayer1->myTurn()==BLACK)? fPlayer1:fPlayer2;
    fScreen->initializeScreen(white->myName(),black->myName());
#endif
    return;
}


void chessBoard::initializeSession()
{
    fMove_History=new vector<string>;
    fCaptured_Piece_Player1=new vector<chessPiece>;
    fCaptured_Piece_Player2=new vector<chessPiece>;
    fSession=true;
    return;
}


void chessBoard::moderator()
{
#ifdef DEBUG
    fTurn=WHITE;
#endif

    Player* temp=(fTurn==fPlayer1->myTurn())? fPlayer1:fPlayer2;
    vector<chessPiece> move_command=convertCommandToDigital(temp->requestCommand());

#ifdef DEBUG
    while(fSession)
    {
#endif
        if(movePiece(move_command.at(0),move_command.at(1)))
        {
#ifndef DEBUG
            if(fStatus)
            {
                fScreen->statusScreen("",WHITE);
                fScreen->statusScreen("",BLACK);
                fStatus=false;
            }
#endif
            fMove_History->push_back(temp->lastValidCommand());
            if(isKingCheckMated(move_command.at(1)))
            {
                if(fTurn==fPlayer1->myTurn())
                {
#ifdef DEBUG
                    cout<<"CHECKMATE!!!\n";
                    cout<<fPlayer1->myName()+" WINS"<<endl;
#else
                    QSound::play(":/sounds/checkmate.wav");
                    fSession=false;
                    fScreen->endGameScreen("CHECKMATE!!!",fPlayer1->myName()+" WINS");
#endif
                }
                else
                {
#ifdef DEBUG
                    cout<<"CHECKMATE!!!\n";
                    cout<<fPlayer2->myName()+" WINS"<<endl;
#else
                    QSound::play(":/sounds/checkmate.wav");
                    fSession=false;
                    fScreen->endGameScreen("CHECKMATE!!!",fPlayer2->myName()+" WINS");
#endif
                }
            }
            else if(isStalemate())
            {
#ifdef DEBUG
                cout<<"STALEMATE"<<endl;
#else
                QSound::play(":/sounds/stalemate.wav");
                fSession=false;
                fScreen->endGameScreen("STALEMATE","");
#endif
            }
            else if(fFifty_Moves==50)
            {
                char choice;
#ifdef DEBUG
                cout<<"Do you want to draw the game?(y/n)\n";
                cin>>choice;
                while('y'!=tolower(choice) && 'n'!=tolower(choice))
                    cin>>choice;
#else
                if(fScreen->drawGame())
                    choice='y';
                else
                    choice='n';
#endif
                switch(tolower(choice))
                {
                    case 'y':
                    {
#ifdef DEBUG
                        cout<<"DRAW"<<endl;
#else
                        fSession=false;
                        fScreen->endGameScreen("DRAW","");
#endif
                        break;
                    }
                    case 'n':
                    {
                        fFifty_Moves=0;
                        break;
                    }
                }
            }
            else if(isKingChecked())
            {
#ifdef DEBUG
                cout<<"CHECK"<<endl;
#else
               QSound::play(":/sounds/check.wav");
               fScreen->statusScreen("CHECK",fTurn^1);               
               fStatus=true;
               fCheck=true;
#endif
            }
            ++fPly_Count;
            fTurn^=1;
        }
        else
        {
#ifdef DEBUG
            cout<<"INVALID MOVE"<<endl;
#if defined (_WIN32) || defined (_WIN64)
            system("pause");
#elif defined (__linux__)
            cin.get();
#endif
#else
            if(fPly_Count!=0)
            {
                fScreen->statusScreen("INVALID MOVE",fTurn);
                fStatus=true;
            }
#endif            
        }
#ifdef DEBUG
        cout<<temp->NODES<<endl;
        if(fPlayer1->myName()=="Lauretta 1" && fPlayer2->myName()=="Lauretta 2")
            cin.get();
#endif

#ifdef DEBUG
        temp=(fTurn==fPlayer1->myTurn())? fPlayer1:fPlayer2;
        if(fSession)
            move_command=convertCommandToDigital(temp->requestCommand());
    }
#else
       if(fSession)
           QTimer::singleShot(1000,this,SIGNAL(played()));
#endif
    return;
}


void chessBoard::updateBoard()
{
    for(vector<chessPiece>::iterator it=fPlayer1->fPiece->begin(); it!=fPlayer1->fPiece->end(); ++it)
        fBoard->at((*it).fX)->at((*it).fY).value=(*it).fValue,fBoard->at((*it).fX)->at((*it).fY).color=fPlayer1->myTurn();

    for(vector<chessPiece>::iterator it2=fPlayer2->fPiece->begin(); it2!=fPlayer2->fPiece->end(); ++it2)
        fBoard->at((*it2).fX)->at((*it2).fY).value=(*it2).fValue,fBoard->at((*it2).fX)->at((*it2).fY).color=fPlayer2->myTurn();

    updateScreen();
    return;
}


void chessBoard::updateScreen()
{
#ifdef DEBUG
#if defined (__linux__)
    system("clear");
#elif defined (_WIN32) || defined (_WIN64)
    system("cls");
#endif
    cout<<"  ";
    for(char start='A'; start<='H'; ++start)
        cout<<"   "<<start<<"  ";
    cout<<endl<<"  ";
    for(int x=0; x<49; ++x)
        cout<<'-';
    cout<<endl;
    for(int i=7; i>=0; --i)
    {
        cout<<i+1<<" |";
        for(int j=0; j<8; ++j)
            if(fBoard->at(j)->at(i).color==WHITE)
            {
                switch(fBoard->at(j)->at(i).value)
                {
                    case -1:cout<<"     |";
                        break;
                    case 0:cout<<"  P  |";
                        break;
                    case 1:cout<<"  N  |";
                        break;
                    case 2:cout<<"  B  |";
                        break;
                    case 3:cout<<"  R  |";
                        break;
                    case 4:cout<<"  Q  |";
                        break;
                    case 5:cout<<"  K  |";
                        break;
                }
            }
            else
            {
                switch(fBoard->at(j)->at(i).value)
                {
                    case -1:cout<<"     |";
                        break;
                    case 0:cout<<"  p  |";
                        break;
                    case 1:cout<<"  n  |";
                        break;
                    case 2:cout<<"  b  |";
                        break;
                    case 3:cout<<"  r  |";
                        break;
                    case 4:cout<<"  q  |";
                        break;
                    case 5:cout<<"  k  |";
                        break;
                }
            }
        cout<<endl<<"  ";
        for(int y=0; y<49; ++y)
            cout<<'-';
        cout<<endl;
    }
    cout<<' ';
    cout<<endl;
    cout<<endl<<endl;
#else
    fScreen->updateScreen(fBoard);
#endif

    return;
}


int chessBoard::requestColor()
{
    int color;
#ifdef DEBUG
#if defined (__linux__)
    system("clear");
#elif defined (_WIN32) || defined (_WIN64)
    system("cls");
#endif
    char choice='\0';
    cout<<"Please choose your chess piece color below\n";
    cout<<"1."<<'\t'<<"WHITE CHESS PIECE\n";
    cout<<"2."<<'\t'<<"BLACK CHESS PIECE\n";
    cin>>choice;
    while(choice!='1' && choice!='2')
        cin>>choice;
    switch(choice)
    {
        case '1':color=WHITE;
            break;
        case '2':color=BLACK;
            break;
        default:break;
    }
#else
    color=fScreen->requestColor();
#endif

#ifdef DEBUG
    cin.ignore();
#endif
    return color;
}


string chessBoard::requestName(int player)
{
    string name;
#ifdef DEBUG
#if defined (__linux__)
    system("clear");
#elif defined (_WIN32) || defined (_WIN64)
    system("cls");
#endif
#endif
    if(player==0)
    {
#ifdef DEBUG
        cout<<"Please type in your display name for this game session below\n";
        cout<<"$NAME:>";
        getline(cin,name);
        while(name=="")
        {
            cout<<"$NAME:>";
            getline(cin,name);
        }
#else
        name=fScreen->requestName(player);
#endif
    }
    else
    {
#ifdef DEBUG
        cout<<"Please type in your display name for Player "<<player<<" below\n";
        cout<<"$NAME:>";
        getline(cin,name);
        while(name=="")
        {
            cout<<"$NAME:>";
            getline(cin,name);
        }
#else
        name=fScreen->requestName(player);
#endif
        if(player==2)
        {
            while(name==fPlayer1->myName())
            {
#ifdef DEBUG
                cout<<"The same name cannot be used for both players.\n";
                cout<<"Please input a new name for player 2\n";
                cout<<"$NAME:>";
                getline(cin,name);
                while(name=="")
                {
                    cout<<"$NAME:>";
                    getline(cin,name);
                }
#else
        name=fScreen->requestName(player,true);
#endif
            }
        }

    }
    return name;
}


vector<chessPiece> chessBoard::convertCommandToDigital(string command)
{
    vector<chessPiece> location;
    if(command=="quit")
    {
        fSession=false;
        return location;
    }
    location.push_back(chessPiece((int)command.at(0)-'a',(int)command.at(1)-49,-1));
    location.push_back(chessPiece((int)command.at(2)-'a',(int)command.at(3)-49,-1));
    return location;
}


bool chessBoard::movePiece(chessPiece from, chessPiece to, bool dummy, bool castling)
{
    Player* move_now=(fPlayer1->myTurn()==fTurn)? fPlayer1:fPlayer2;
    if((move_now->myName()=="Lauretta 1" || move_now->myName()=="Lauretta 2" || castling==true) && dummy==false)
    {
        ++fFifty_Moves;
        for(vector<chessPiece>::iterator it=move_now->fPiece->begin(); it!=move_now->fPiece->end(); ++it)
            if(from==*it)
            {                
                bool capture=isPieceCaptured(to);
                if(capture==true)
                    fFifty_Moves=0;

                switch(it->fValue)
                {
                    case 0:
                    {
                        //enpassant capture
                        if((from.fX!=to.fX) && !capture)
                        {
                            (void)isPieceCaptured(chessPiece(to.fX,from.fY));
                            fFifty_Moves=0;
                            fBoard->at(to.fX)->at(from.fY).value=-1;
                        }
                        break;
                    }
                    case 3:
                    {
                        if(move_now->myTurn()==WHITE)
                        {
                            if((*it).fX==0 && (*it).fY==0 && !move_now->hasLeftRookMoved())
                                move_now->setLeftRookMoved(true);
                            else if((*it).fX==7 && (*it).fY==0 && !move_now->hasRightRookMoved())
                                move_now->setRightRookMoved(true);
                        }
                        else if(move_now->myTurn()==BLACK)
                        {
                            if((*it).fX==0 && (*it).fY==7 && !move_now->hasLeftRookMoved())
                                move_now->setLeftRookMoved(true);
                            else if((*it).fX==7 && (*it).fY==7 && !move_now->hasRightRookMoved())
                                move_now->setRightRookMoved(true);
                        }
                        break;
                    }

                    case 5:
                    {
                        if(!move_now->hasKingMoved())
                        {
                            //castling move
                            if(abs(from.fX-to.fX)==2)
                            {
                                if(from.fX>to.fX)
                                {
                                    for(vector<chessPiece>::iterator it2=move_now->fPiece->begin(); it2!=move_now->fPiece->end(); ++it2)
                                        if(it2->fValue==3 && it2->fX==0)
                                        {
                                            it2->fX=3;
                                            move_now->setLeftRookMoved(true);
                                            fBoard->at(0)->at(it2->fY).value=-1;
                                            break;
                                        }
                                }
                                else
                                {
                                    for(vector<chessPiece>::iterator it2=move_now->fPiece->begin(); it2!=move_now->fPiece->end(); ++it2)
                                        if(it2->fValue==3 && it2->fX==7)
                                        {
                                            it2->fX=5;
                                            move_now->setRightRookMoved(true);
                                            fBoard->at(7)->at(it2->fY).value=-1;
                                            break;
                                        }
                                }
                            }
                            move_now->setKingMoved(true);
                        }
                        break;
                    }
                }

                to.fValue=(*it).fValue;
                *it=to;

                if((to.fY==7 && move_now->myTurn()==0) && (*it).fValue==0)
                    (*it).fValue=move_now->requestPromotion();
                else if((to.fY==0 && move_now->myTurn()==1) && (*it).fValue==0)
                    (*it).fValue=move_now->requestPromotion();

                fBoard->at(from.fX)->at(from.fY).value=-1;
                updateBoard();
                break;
            }
        return true;
    }

    bool valid=false;
    if(isMoveValid(from,to,dummy^true))
    {
        valid=true;
        if(dummy==false)
        {
            ++fFifty_Moves;

            if(fTurn==fPlayer1->myTurn())
            {
                for(vector<chessPiece>::iterator it=fPlayer1->fPiece->begin(); it!=fPlayer1->fPiece->end(); ++it)
                    if(from==*it)
                    {
                        chessPiece undo_move(*it);
                        to.fValue=(*it).fValue;
                        *it=to;

                        if((to.fY==7 && fPlayer1->myTurn()==WHITE) && (*it).fValue==0)
                            (*it).fValue=fPlayer1->requestPromotion();
                        else if((to.fY==0 && fPlayer1->myTurn()==BLACK) && (*it).fValue==0)
                            (*it).fValue=fPlayer1->requestPromotion();

                        bool capture=isPieceCaptured(to);
                        if(isKingExposed())
                        {
                            *it=undo_move;
                            if(capture==true)
                            {
                                fPlayer2->fPiece->push_back(fCaptured_Piece_Player1->at(fCaptured_Piece_Player1->size()-1));
                                fCaptured_Piece_Player1->pop_back();
                            }
                            valid=false;
                        }
                        else
                        {
                            if(capture==true)
                                fFifty_Moves=0;
                            fBoard->at(from.fX)->at(from.fY).value=-1;
                            updateBoard();
                        }
                        break;
                    }
            }
            else
            {
                for(vector<chessPiece>::iterator it2=fPlayer2->fPiece->begin(); it2!=fPlayer2->fPiece->end(); ++it2)
                    if(from==*it2)
                    {
                        chessPiece undo_move(*it2);
                        to.fValue=(*it2).fValue;
                        *it2=to;

                        if((to.fY==0 && fPlayer2->myTurn()==BLACK) && (*it2).fValue==0)
                            (*it2).fValue=fPlayer2->requestPromotion();
                        else if((to.fY==7 && fPlayer2->myTurn()==WHITE) && (*it2).fValue==0)
                            (*it2).fValue=fPlayer2->requestPromotion();

                        bool capture=isPieceCaptured(to);
                        if(isKingExposed())
                        {
                            *it2=undo_move;
                            if(capture==true)
                            {
                                fPlayer1->fPiece->push_back(fCaptured_Piece_Player2->at(fCaptured_Piece_Player2->size()-1));
                                fCaptured_Piece_Player2->pop_back();
                            }
                            valid=false;
                        }
                        else
                        {
                            if(capture==true)
                                fFifty_Moves=0;
                            fBoard->at(from.fX)->at(from.fY).value=-1;
                            updateBoard();
                        }
                        break;
                    }
            }
        }
        else
        {
            if(fTurn==fPlayer1->myTurn())
            {
                for(vector<chessPiece>::iterator it=fPlayer1->fPiece->begin(); it!=fPlayer1->fPiece->end(); ++it)
                    if(from==*it)
                    {
                        chessPiece undo_move(*it);
                        to.fValue=(*it).fValue;
                        *it=to;
                        if((to.fY==7 && fPlayer1->myTurn()==0) && (*it).fValue==0)
                            (*it).fValue=4;
                        bool capture=isPieceCaptured(to);
                        if(isKingExposed())
                        {
                            *it=undo_move;
                            if(capture==true)
                            {
                                fPlayer2->fPiece->push_back(fCaptured_Piece_Player1->at(fCaptured_Piece_Player1->size()-1));
                                fCaptured_Piece_Player1->pop_back();
                            }
                            valid=false;
                        }
                        else
                        {
                            *it=undo_move;
                            if(capture==true)
                            {
                                fPlayer2->fPiece->push_back(fCaptured_Piece_Player1->at(fCaptured_Piece_Player1->size()-1));
                                fCaptured_Piece_Player1->pop_back();
                            }
                        }
                        break;
                    }
            }
            else
            {
                for(vector<chessPiece>::iterator it2=fPlayer2->fPiece->begin(); it2!=fPlayer2->fPiece->end(); ++it2)
                    if(from==*it2)
                    {
                        chessPiece undo_move(*it2);
                        to.fValue=(*it2).fValue;
                        *it2=to;
                        if((to.fY==0 && fPlayer2->myTurn()==1) && (*it2).fValue==0)
                            (*it2).fValue=4;
                        bool capture=isPieceCaptured(to);
                        if(isKingExposed())
                        {
                            *it2=undo_move;
                            if(capture==true)
                            {
                                fPlayer1->fPiece->push_back(fCaptured_Piece_Player2->at(fCaptured_Piece_Player2->size()-1));
                                fCaptured_Piece_Player2->pop_back();
                            }
                            valid=false;
                        }
                        else
                        {
                            *it2=undo_move;
                            if(capture==true)
                            {
                                fPlayer1->fPiece->push_back(fCaptured_Piece_Player2->at(fCaptured_Piece_Player2->size()-1));
                                fCaptured_Piece_Player2->pop_back();
                            }
                        }
                        break;
                    }
            }
        }
    }
    if(valid==false && dummy==false)
        --fFifty_Moves;

    return valid;
}


bool chessBoard::isMoveValid(chessPiece& from, chessPiece& to, bool real)
{
    Player* temp=(fTurn==fPlayer1->myTurn())? fPlayer1:fPlayer2;
    bool valid=false;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        if(*it==to)
            return false;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        if((*it)==from)
        {
            switch((*it).fValue)
            {
                case 0:
                {
                    if(isMoveStraight(from,to,2,real) || isMoveDiagonal(from,to,2,real))
                        valid=true;
                    if(valid==true && real==true)
                        fFifty_Moves=0;
                    break;
                }

                case 1:
                {
                    if(isMoveKnight(from,to))
                        valid=true;
                    break;
                }

                case 2:
                {
                    if(isMoveDiagonal(from,to))
                        valid=true;
                    break;
                }

                case 3:
                {
                    if(isMoveStraight(from,to) || isMoveSideways(from,to))
                    {
                        valid=true;
                        if(temp->myTurn()==WHITE && real==true && valid==true)
                        {
                            if((*it).fX==0 && (*it).fY==0 && !temp->hasLeftRookMoved())
                                temp->setLeftRookMoved(true);
                            else if((*it).fX==7 && (*it).fY==0 && !temp->hasRightRookMoved())
                                temp->setRightRookMoved(true);
                        }
                        else if(temp->myTurn()==BLACK && real==true && valid==true)
                        {
                            if((*it).fX==0 && (*it).fY==7 && !temp->hasLeftRookMoved())
                                temp->setLeftRookMoved(true);
                            else if((*it).fX==7 && (*it).fY==7 && !temp->hasRightRookMoved())
                                temp->setRightRookMoved(true);
                        }
                    }
                    break;
                }

                case 4:
                {
                    if(isMoveDiagonal(from,to) || isMoveStraight(from,to) || isMoveSideways(from,to))
                        valid=true;
                    break;
                }

                case 5:
                {
                    if(isMoveStraight(from,to,1) || isMoveSideways(from,to,1,real) || isMoveDiagonal(from,to,1))
                    {
                        valid=true;                        
                        if(valid==true && !temp->hasKingMoved() && real==true)
                            temp->setKingMoved(true);
                    }
                    break;
                }
                default:break;
            }
            break;
        }

    return valid;
}


bool chessBoard::isPieceValid(chessPiece& piece)
{
    bool valid=true;
    if(piece.fX<0 || piece.fX>7 || piece.fY<0 || piece.fY>7)
        valid=false;
    return valid;
}


bool chessBoard::isMoveDiagonal(chessPiece& from,chessPiece& to, int king, bool real)
{
    bool valid=false;
    Player* temp=(fPlayer1->myTurn()==fTurn)? fPlayer1:fPlayer2;
    Player* temp2=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;
    if(king==0)
    {
        for(int x=from.fX+1,y=from.fY+1; (x<=to.fX && y<=to.fY); ++x,++y)
            if(to.fX==x && to.fY==y)
            {
                valid=true;
                break;
            }
            else
            {
                bool block=false;
                for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                    if((*it).fX==x && (*it).fY==y)
                    {
                        valid=false;
                        block=true;
                        break;
                    }

                if(block==true)
                    break;

                for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                    if((*it).fX==x && (*it).fY==y)
                    {
                        valid=false;
                        block=true;
                        break;
                    }

                if(block==true)
                    break;
            }

        for(int x=from.fX-1,y=from.fY+1; (x>=to.fX && y<=to.fY); --x,++y)
            if(to.fX==x && to.fY==y)
            {
                valid=true;
                break;
            }
            else
            {
                bool block=false;
                for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                    if((*it).fX==x && (*it).fY==y)
                    {
                        valid=false;
                        block=true;
                        break;
                    }

                if(block==true)
                    break;

                for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                    if((*it).fX==x && (*it).fY==y)
                    {
                        valid=false;
                        block=true;
                        break;
                    }

                if(block==true)
                    break;
            }

        for(int x=from.fX-1,y=from.fY-1; (x>=to.fX && y>=to.fY); --x,--y)
            if(to.fX==x && to.fY==y)
            {
                valid=true;
                break;
            }
            else
            {
                bool block=false;
                for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                    if((*it).fX==x && (*it).fY==y)
                    {
                        valid=false;
                        block=true;
                        break;
                    }

                if(block==true)
                    break;

                for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                    if((*it).fX==x && (*it).fY==y)
                    {
                        valid=false;
                        block=true;
                        break;
                    }

                if(block==true)
                    break;
            }

        for(int x=from.fX+1,y=from.fY-1; (x<=to.fX && y>=to.fY); ++x,--y)
            if(to.fX==x && to.fY==y)
            {
                valid=true;
                break;
            }
            else
            {
                bool block=false;
                for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                    if((*it).fX==x && (*it).fY==y)
                    {
                        valid=false;
                        block=true;
                        break;
                    }

                if(block==true)
                    break;

                for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                    if((*it).fX==x && (*it).fY==y)
                    {
                        valid=false;
                        block=true;
                        break;
                    }

                if(block==true)
                    break;
            }
    }
    else if(king==1)
    {
        if(abs(from.fX-to.fX)==1 && abs(from.fY-to.fY)==1)
            valid=true;
    }
    else
    {
        if(temp->myTurn()==WHITE)
        {
            if(abs(from.fX-to.fX)==1 && from.fY-to.fY==-1)
                for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                    if(to==*(it))
                    {
                        valid=true;
                        break;
                    }
            if(valid==false)
                valid=isEnpassant(from,to,real);
        }
        else
        {
            if(abs(from.fX-to.fX)==1 && from.fY-to.fY==1)
                for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                    if(to==*(it))
                    {
                        valid=true;
                        break;
                    }
            if(valid==false)
                valid=isEnpassant(from,to,real);
        }
    }

    return valid;
}


bool chessBoard::isMoveStraight(chessPiece& from, chessPiece& to, int pawn, bool real)
{
    bool valid=false;
    Player* temp=(fPlayer1->myTurn()==fTurn)? fPlayer1:fPlayer2;
    Player* temp2=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;
    if(pawn==0)
    {
        if(from.fX==to.fX)
        {
            valid=true;
            for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                if((*it).fX==to.fX && (((*it).fY>from.fY && (*it).fY<to.fY) || ((*it).fY<from.fY && (*it).fY>to.fY)))
                {
                    valid=false;
                    break;
                }

            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if((*it).fX==to.fX && (((*it).fY>from.fY && (*it).fY<to.fY) || ((*it).fY<from.fY && (*it).fY>to.fY)))
                {
                    valid=false;
                    break;
                }
        }

    }
    else if(pawn==1)
    {
        if(from.fX==to.fX && (abs(from.fY-to.fY)==1))
            valid=true;
    }
    else
    {
        bool enpassant=false;
        if(from.fX==to.fX && (abs(from.fY-to.fY)==2) && (from.fY==6 || from.fY==1))
        {
            valid=true;
            enpassant=true;
            for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                if((*it).fX==to.fX && (((*it).fY>from.fY && (*it).fY<to.fY) || ((*it).fY<from.fY && (*it).fY>to.fY)))
                {
                    valid=false;
                    break;
                }

            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if((*it).fX==to.fX && (((*it).fY>from.fY && (*it).fY<to.fY) || ((*it).fY<from.fY && (*it).fY>to.fY)))
                {
                    valid=false;
                    break;
                }
        }
        else if(from.fX==to.fX && (abs(from.fY-to.fY)==1))
        {
            if(temp->myTurn()==WHITE && from.fY<to.fY)
                valid=true;
            else if(temp->myTurn()==BLACK && from.fY>to.fY)
                valid=true;
        }

        for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
            if(to==*(it))
            {
                valid=false;
                break;
            }
        if(enpassant==true && valid==true && real==true)
            temp->setEnpassant(true,fPly_Count,to);
    }

    return valid;
}


bool chessBoard::isMoveSideways(chessPiece& from, chessPiece& to, int king, bool real)
{
    bool valid=false;
    Player* temp=(fPlayer1->myTurn()==fTurn)? fPlayer1:fPlayer2;
    Player* temp2=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;
    if(king==0)
    {
        if(from.fY==to.fY)
        {
            valid=true;
            for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                if((*it).fY==to.fY && (((*it).fX>from.fX && (*it).fX<to.fX) || ((*it).fX<from.fX && (*it).fX>to.fX)))
                {
                    valid=false;
                    break;
                }

            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if((*it).fY==to.fY && (((*it).fX>from.fX && (*it).fX<to.fX) || ((*it).fX<from.fX && (*it).fX>to.fX)))
                {
                    valid=false;
                    break;
                }
        }

    }
    else
    {
        if(from.fY==to.fY && (abs(from.fX-to.fX)==1))
            valid=true;
        else if(from.fY==to.fY && (abs(from.fX-to.fX)==2))
            valid=isCastling(to,real);
    }

    return valid;
}


bool chessBoard::isMoveKnight(chessPiece& from, chessPiece& to)
{
    bool valid=false;
    chessPiece temp(from);

    if(chessPiece(temp.fX-2,temp.fY+1)==to)
        valid=true;
    else if(chessPiece(temp.fX+2,temp.fY+1)==to)
        valid=true;
    else if(chessPiece(temp.fX-2,temp.fY-1)==to)
        valid=true;
    else if(chessPiece(temp.fX+2,temp.fY-1)==to)
        valid=true;
    else if(chessPiece(temp.fX-1,temp.fY+2)==to)
        valid=true;
    else if(chessPiece(temp.fX-1,temp.fY-2)==to)
        valid=true;
    else if(chessPiece(temp.fX+1,temp.fY+2)==to)
        valid=true;
    else if(chessPiece(temp.fX+1,temp.fY-2)==to)
        valid=true;
    return valid;
}


bool chessBoard::isStalemate()
{
    bool valid=false;
    Player* temp=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;

    if(!hasKingValidMove(false))
    {
        unsigned all=0;
        for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
            switch(it->fValue)
            {
                case 0:
                {
                    vector<chessPiece> possible_moves;
                    if(temp->myTurn()==0)
                    {
                        possible_moves.push_back(chessPiece(it->fX,it->fY+1));
                        possible_moves.push_back(chessPiece(it->fX-1,it->fY+1));
                        possible_moves.push_back(chessPiece(it->fX+1,it->fY+1));
                    }
                    else
                    {
                        possible_moves.push_back(chessPiece(it->fX,it->fY-1));
                        possible_moves.push_back(chessPiece(it->fX-1,it->fY-1));
                        possible_moves.push_back(chessPiece(it->fX+1,it->fY-1));
                    }
                    for(unsigned it=0; it<possible_moves.size(); ++it)
                        if(!isPieceValid(possible_moves.at(it)))
                        {
                            possible_moves.erase(possible_moves.begin()+it);
                            --it;
                        }
                    unsigned individual=0;
                    fTurn^=1;
                    for(unsigned i=0; i<possible_moves.size(); ++i)
                        if(!movePiece(*it,possible_moves.at(i),true))
                            ++individual;
                    fTurn^=1;
                    if(individual==possible_moves.size())
                        ++all;
                    break;
                }

                case 1:
                {
                    vector<chessPiece> possible_moves;
                    possible_moves.push_back(chessPiece(it->fX-2,it->fY+1));
                    possible_moves.push_back(chessPiece(it->fX+2,it->fY+1));
                    possible_moves.push_back(chessPiece(it->fX-2,it->fY-1));
                    possible_moves.push_back(chessPiece(it->fX+2,it->fY-1));
                    possible_moves.push_back(chessPiece(it->fX-1,it->fY+2));
                    possible_moves.push_back(chessPiece(it->fX-1,it->fY-2));
                    possible_moves.push_back(chessPiece(it->fX+1,it->fY+2));
                    possible_moves.push_back(chessPiece(it->fX+1,it->fY-2));
                    for(unsigned it=0; it<possible_moves.size(); ++it)
                        if(!isPieceValid(possible_moves.at(it)))
                        {
                            possible_moves.erase(possible_moves.begin()+it);
                            --it;
                        }
                    unsigned individual=0;
                    fTurn^=1;
                    for(unsigned i=0; i<possible_moves.size(); ++i)
                        if(!movePiece(*it,possible_moves.at(i),true))
                            ++individual;
                    fTurn^=1;
                    if(individual==possible_moves.size())
                        ++all;
                    break;
                }

                case 2:
                {
                    vector<chessPiece> possible_moves;
                    for(int x=1,y=1; (it->fX+x<8) && (it->fY+y<8); ++x,++y)
                        possible_moves.push_back(chessPiece(it->fX+x,it->fY+y));
                    for(int x=1,y=1; (it->fX+x<8) && (it->fY-y>=0); ++x,++y)
                        possible_moves.push_back(chessPiece(it->fX+x,it->fY-y));
                    for(int x=1,y=1; (it->fX-x>=0) && (it->fY+y<8); ++x,++y)
                        possible_moves.push_back(chessPiece(it->fX-x,it->fY+y));
                    for(int x=1,y=1; (it->fX-x>=0) && (it->fY-y>=0); ++x,++y)
                        possible_moves.push_back(chessPiece(it->fX-x,it->fY-y));

                    unsigned individual=0;
                    fTurn^=1;
                    for(unsigned i=0; i<possible_moves.size(); ++i)
                        if(!movePiece(*it,possible_moves.at(i),true))
                            ++individual;
                    fTurn^=1;
                    if(individual==possible_moves.size())
                        ++all;
                    break;
                }

                case 3:
                {
                    vector<chessPiece> possible_moves;
                    for(int y=1; it->fY+y<8; ++y)
                        possible_moves.push_back(chessPiece(it->fX,it->fY+y));
                    for(int y=1; it->fY-y>=0; ++y)
                        possible_moves.push_back(chessPiece(it->fX,it->fY-y));
                    for(int x=1; it->fX-x>=0; ++x)
                        possible_moves.push_back(chessPiece(it->fX-x,it->fY));
                    for(int x=1; it->fX+x<8; ++x)
                        possible_moves.push_back(chessPiece(it->fX+x,it->fY));

                    unsigned individual=0;
                    fTurn^=1;
                    for(unsigned i=0; i<possible_moves.size(); ++i)
                        if(!movePiece(*it,possible_moves.at(i),true))
                            ++individual;
                    fTurn^=1;
                    if(individual==possible_moves.size())
                        ++all;
                    break;
                }

                case 4:
                {
                    //Diagonal moves
                    vector<chessPiece> possible_moves;
                    for(int x=1,y=1; (it->fX+x<8) && (it->fY+y<8); ++x,++y)
                        possible_moves.push_back(chessPiece(it->fX+x,it->fY+y));
                    for(int x=1,y=1; (it->fX+x<8) && (it->fY-y>=0); ++x,++y)
                        possible_moves.push_back(chessPiece(it->fX+x,it->fY-y));
                    for(int x=1,y=1; (it->fX-x>=0) && (it->fY+y<8); ++x,++y)
                        possible_moves.push_back(chessPiece(it->fX-x,it->fY+y));
                    for(int x=1,y=1; (it->fX-x>=0) && (it->fY-y>=0); ++x,++y)
                        possible_moves.push_back(chessPiece(it->fX-x,it->fY-y));

                    //Straight moves
                    for(int y=1; it->fY+y<8; ++y)
                        possible_moves.push_back(chessPiece(it->fX,it->fY+y));
                    for(int y=1; it->fY-y>=0; ++y)
                        possible_moves.push_back(chessPiece(it->fX,it->fY-y));
                    for(int x=1; it->fX-x>=0; ++x)
                        possible_moves.push_back(chessPiece(it->fX-x,it->fY));
                    for(int x=1; it->fX+x<8; ++x)
                        possible_moves.push_back(chessPiece(it->fX+x,it->fY));

                    unsigned individual=0;
                    fTurn^=1;
                    for(unsigned i=0; i<possible_moves.size(); ++i)
                        if(!movePiece(*it,possible_moves.at(i),true))
                            ++individual;
                    fTurn^=1;
                    if(individual==possible_moves.size())
                        ++all;
                    break;
                }
            }
        if(all==temp->fPiece->size()-1)
            valid=true;
    }

    return valid;
}


bool chessBoard::isKingExposed()
{
    bool valid=false;
    Player* temp=(fPlayer1->myTurn()==fTurn)? fPlayer1:fPlayer2;
    Player* temp2=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;
    fTurn^=1;
    for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
        if(isMoveValid(*it,*(temp->king()),false))
        {
            valid=true;
            break;
        }
    fTurn^=1;
    return valid;
}


bool chessBoard::isKingChecked()
{   
    Player* temp=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;
    fTurn^=1;
    bool valid=isKingExposed();
    fTurn^=1;
    temp->setKingChecked(valid);
    return valid;
}


bool chessBoard::canProtectKing(chessPiece& destination)
{
    bool valid=false;
    if(isKingChecked())
    {
        Player* temp=(fPlayer1->myTurn()==fTurn)? fPlayer1:fPlayer2;
        Player* temp2=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;

        fTurn^=1;
        for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
            if(movePiece(*it,destination,true))
            {
                valid=true;
                break;
            }
        if(valid==false)
        {
            chessPiece threat;
            for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                if(*it==destination)
                {
                    threat=*it;
                    break;
                }
            switch(threat.fValue)
            {
                case 0:break;
                case 1:break;

                case 2:
                {
                    if(threat.fX<temp2->king()->fX && threat.fY<temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        chessPiece start(threat.fX+1,threat.fY+1);
                        chessPiece stop(temp2->king()->fX,temp2->king()->fY);
                        for(int i=start.fX,j=start.fY; (i<stop.fX && j<stop.fY); ++i,++j)
                            possible_blocks.push_back(chessPiece(i,j));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    else if(threat.fX>temp2->king()->fX && threat.fY<temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        chessPiece start(threat.fX+1,threat.fY+1);
                        chessPiece stop(temp2->king()->fX,temp2->king()->fY);
                        for(int i=start.fX,j=start.fY; (i>stop.fX && j<stop.fY); --i,++j)
                            possible_blocks.push_back(chessPiece(i,j));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    else if(threat.fX<temp2->king()->fX && threat.fY>temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        chessPiece start(threat.fX+1,threat.fY+1);
                        chessPiece stop(temp2->king()->fX,temp2->king()->fY);
                        for(int i=start.fX,j=start.fY; (i<stop.fX && j>stop.fY); ++i,--j)
                            possible_blocks.push_back(chessPiece(i,j));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    else if(threat.fX>temp2->king()->fX && threat.fY>temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        chessPiece start(threat.fX+1,threat.fY+1);
                        chessPiece stop(temp2->king()->fX,temp2->king()->fY);
                        for(int i=start.fX,j=start.fY; (i>stop.fX && j>stop.fY); --i,--j)
                            possible_blocks.push_back(chessPiece(i,j));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    break;
                }

                case 3:
                {
                    if(threat.fX==temp2->king()->fX)
                    {
                        vector<chessPiece> possible_blocks;
                        int start=(threat.fY<temp2->king()->fY)? threat.fY:temp2->king()->fY;
                        int stop=(start==threat.fY)? temp2->king()->fY:threat.fY;
                        for(int i=start+1; i<stop; ++i)
                            possible_blocks.push_back(chessPiece(threat.fX,i));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    else if(threat.fY==temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        int start=(threat.fX<temp2->king()->fX)? threat.fX:temp2->king()->fX;
                        int stop=(start==threat.fX)? temp2->king()->fX:threat.fX;
                        for(int i=start+1; i<stop; ++i)
                            possible_blocks.push_back(chessPiece(i,threat.fY));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    break;
                }

                case 4:
                {
                    if(threat.fX==temp2->king()->fX)
                    {
                        vector<chessPiece> possible_blocks;
                        int start=(threat.fY<temp2->king()->fY)? threat.fY:temp2->king()->fY;
                        int stop=(start==threat.fY)? temp2->king()->fY:threat.fY;
                        for(int i=start+1; i<stop; ++i)
                            possible_blocks.push_back(chessPiece(threat.fX,i));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    else if(threat.fY==temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        int start=(threat.fX<temp2->king()->fX)? threat.fX:temp2->king()->fX;
                        int stop=(start==threat.fX)? temp2->king()->fX:threat.fX;
                        for(int i=start+1; i<stop; ++i)
                            possible_blocks.push_back(chessPiece(i,threat.fY));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    else if(threat.fX<temp2->king()->fX && threat.fY<temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        chessPiece start(threat.fX+1,threat.fY+1);
                        chessPiece stop(temp2->king()->fX,temp2->king()->fY);
                        for(int i=start.fX,j=start.fY; (i<stop.fX && j<stop.fY); ++i,++j)
                            possible_blocks.push_back(chessPiece(i,j));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    else if(threat.fX>temp2->king()->fX && threat.fY<temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        chessPiece start(threat.fX+1,threat.fY+1);
                        chessPiece stop(temp2->king()->fX,temp2->king()->fY);
                        for(int i=start.fX,j=start.fY; (i>stop.fX && j<stop.fY); --i,++j)
                            possible_blocks.push_back(chessPiece(i,j));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,false))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    else if(threat.fX<temp2->king()->fX && threat.fY>temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        chessPiece start(threat.fX+1,threat.fY+1);
                        chessPiece stop(temp2->king()->fX,temp2->king()->fY);
                        for(int i=start.fX,j=start.fY; (i<stop.fX && j>stop.fY); ++i,--j)
                            possible_blocks.push_back(chessPiece(i,j));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    else if(threat.fX>temp2->king()->fX && threat.fY>temp2->king()->fY)
                    {
                        vector<chessPiece> possible_blocks;
                        chessPiece start(threat.fX+1,threat.fY+1);
                        chessPiece stop(temp2->king()->fX,temp2->king()->fY);
                        for(int i=start.fX,j=start.fY; (i>stop.fX && j>stop.fY); --i,--j)
                            possible_blocks.push_back(chessPiece(i,j));

                        for(vector<chessPiece>::iterator it2=possible_blocks.begin(); it2!=possible_blocks.end(); ++it2)
                        {
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(movePiece(*it3,*it2,true))
                                {
                                    valid=true;
                                    break;
                                }
                            if(valid==true)
                                break;
                        }
                    }
                    break;
                }

                case 5:break;
                default:break;
            }            
        }
        fTurn^=1;
    }
    return valid;
}


bool chessBoard::isCastling(chessPiece& to, bool real)
{
    Player* temp=(fPlayer1->myTurn()==fTurn)? fPlayer1:fPlayer2;
    Player* temp2=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;
    if(temp->isKingChecked()==true)
        return false;

    bool valid=false;
    if(temp->myTurn()==WHITE && !temp->hasKingMoved())
    {
        if(to.fX==6 && !temp->hasRightRookMoved())
        {
            valid=true;
            for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                if(((*it).fX==5 && (*it).fY==0) || ((*it).fX==6 && (*it).fY==0))
                {
                    valid=false;
                    break;
                }
            fTurn^=1;
            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if(((*it).fX==5 && (*it).fY==0) || ((*it).fX==6 && (*it).fY==0) || movePiece(*it,chessPiece(5,0),true) || movePiece(*it,chessPiece(6,0),true))
                {
                    valid=false;
                    break;
                }
            fTurn^=1;
            if(valid==true && real==true)
                movePiece(chessPiece(7,0),chessPiece(5,0),false,true);
        }
        else if(to.fX==2 && !temp->hasLeftRookMoved())
        {
            valid=true;
            for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                if(((*it).fX==2 && (*it).fY==0) || ((*it).fX==3 && (*it).fY==0))
                {
                    valid=false;
                    break;
                }
            fTurn^=1;
            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if(((*it).fX==2 && (*it).fY==0) || ((*it).fX==3 && (*it).fY==0) || movePiece(*it,chessPiece(2,0),true) || movePiece(*it,chessPiece(3,0),true))
                {
                    valid=false;
                    break;
                }
            fTurn^=1;
            if(valid==true && real==true)
                movePiece(chessPiece(0,0),chessPiece(3,0),false,true);
        }
    }
    else if(temp->myTurn()==BLACK && !temp->hasKingMoved())
    {
        if(to.fX==6 && !temp->hasRightRookMoved())
        {
            valid=true;
            for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                if(((*it).fX==5 && (*it).fY==7) || ((*it).fX==6 && (*it).fY==7))
                {
                    valid=false;
                    break;
                }
            fTurn^=1;
            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if(((*it).fX==5 && (*it).fY==7) || ((*it).fX==6 && (*it).fY==7) || movePiece(*it,chessPiece(5,7),true) || movePiece(*it,chessPiece(6,7),true))
                {
                    valid=false;
                    break;
                }
            fTurn^=1;
            if(valid==true && real==true)
                movePiece(chessPiece(7,7),chessPiece(5,7),false,true);
        }
        else if(to.fX==2 && !temp->hasLeftRookMoved())
        {
            valid=true;
            for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                if(((*it).fX==2 && (*it).fY==7) || ((*it).fX==3 && (*it).fY==7))
                {
                    valid=false;
                    break;
                }
            fTurn^=1;
            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if(((*it).fX==2 && (*it).fY==7) || ((*it).fX==3 && (*it).fY==7) || movePiece(*it,chessPiece(2,7),true) || movePiece(*it,chessPiece(3,7),true))
                {
                    valid=false;
                    break;
                }
            fTurn^=1;
            if(valid==true && real==true)
                movePiece(chessPiece(0,7),chessPiece(3,7),false,true);
        }
    }
    return valid;
}


bool chessBoard::isPieceCaptured(chessPiece destination)
{
    bool valid=false;
    if(fTurn==fPlayer1->myTurn())
    {
        for(vector<chessPiece>::iterator it=fPlayer2->fPiece->begin(); it!=fPlayer2->fPiece->end(); ++it)
            if(*it==destination)
            {
                fCaptured_Piece_Player1->push_back(*it);
                fPlayer2->fPiece->erase(it);
                valid=true;
                break;
            }
    }
    else
    {
        for(vector<chessPiece>::iterator it2=fPlayer1->fPiece->begin(); it2!=fPlayer1->fPiece->end(); ++it2)
            if(*it2==destination)
            {
                fCaptured_Piece_Player2->push_back(*it2);
                fPlayer1->fPiece->erase(it2);
                valid=true;
                break;
            }
    }
    return valid;
}


bool chessBoard::isKingCheckMated(chessPiece& destination)
{
    bool valid=false;
    if(isKingChecked())
        if(!hasKingValidMove() && !canProtectKing(destination))
                valid=true;
    return valid;
}


bool chessBoard::hasKingValidMove(bool check)
{
    bool valid=false;

    if(isKingChecked() || check==false)
    {
        Player* temp2=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;

        vector<chessPiece> possible_king_moves;
        possible_king_moves.push_back(chessPiece(temp2->king()->fX-1,temp2->king()->fY+1));
        possible_king_moves.push_back(chessPiece(temp2->king()->fX-1,temp2->king()->fY+0));
        possible_king_moves.push_back(chessPiece(temp2->king()->fX-1,temp2->king()->fY-1));
        possible_king_moves.push_back(chessPiece(temp2->king()->fX+0,temp2->king()->fY+1));
        possible_king_moves.push_back(chessPiece(temp2->king()->fX+0,temp2->king()->fY-1));
        possible_king_moves.push_back(chessPiece(temp2->king()->fX+1,temp2->king()->fY+1));
        possible_king_moves.push_back(chessPiece(temp2->king()->fX+1,temp2->king()->fY+0));
        possible_king_moves.push_back(chessPiece(temp2->king()->fX+1,temp2->king()->fY-1));

        for(unsigned it=0; it<possible_king_moves.size(); ++it)
            if(!isPieceValid(possible_king_moves.at(it)))
            {
                possible_king_moves.erase(possible_king_moves.begin()+it);
                --it;
            }

        int moves=0;
        fTurn^=1;
        for(vector<chessPiece>::iterator it=possible_king_moves.begin(); it!=possible_king_moves.end(); ++it)
            if(movePiece(*(temp2->king()),*it,true))
            {
                ++moves;
                break;
            }
        fTurn^=1;
        if(moves)
            valid=true;
    }
    return valid;
}


bool chessBoard::isEnpassant(chessPiece& from, chessPiece& to, bool real)
{
    bool valid=false;
    Player* temp=(fPlayer1->myTurn()==fTurn)? fPlayer1:fPlayer2;
    Player* temp2=(fPlayer1->myTurn()==fTurn)? fPlayer2:fPlayer1;

    if(temp->myTurn()==WHITE)
    {
        if((temp2->getEnpassant().piece.fY==4 && from.fY==4) && temp2->getEnpassant().valid==true &&
                (fPly_Count-temp2->getEnpassant().ply==1) && abs(temp2->getEnpassant().piece.fX-from.fX)==1)
        {
            if(to==chessPiece(temp2->getEnpassant().piece.fX,temp2->getEnpassant().piece.fY+1))
            {
                valid=true;
                if(real==true)
                {
                    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                        if(from==*it)
                        {
                            chessPiece undo_move(*it);
                            to.fValue=(*it).fValue;
                            *it=to;

                            bool capture=isPieceCaptured(temp2->getEnpassant().piece);
                            if(isKingExposed())
                            {
                                *it=undo_move;
                                if(capture==true)
                                {
                                    fPlayer2->fPiece->push_back(fCaptured_Piece_Player1->at(fCaptured_Piece_Player1->size()-1));
                                    fCaptured_Piece_Player1->pop_back();
                                }
                                valid=false;
                            }
                            else
                            {
                                *it=undo_move;
                                fBoard->at(temp2->getEnpassant().piece.fX)->at(temp2->getEnpassant().piece.fY).value=-1;
                            }
                            break;
                        }
                }
            }
        }
    }
    else
    {
        if((temp2->getEnpassant().piece.fY==3 && from.fY==3) && temp2->getEnpassant().valid==true &&
            (fPly_Count-temp2->getEnpassant().ply==1) && abs(temp2->getEnpassant().piece.fX-from.fX)==1)
        {
            if(to==chessPiece(temp2->getEnpassant().piece.fX,temp2->getEnpassant().piece.fY-1))
            {
                valid=true;
                if(real==true)
                {
                    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                        if(from==*it)
                        {
                            chessPiece undo_move(*it);
                            to.fValue=(*it).fValue;
                            *it=to;

                            bool capture=isPieceCaptured(temp2->getEnpassant().piece);
                            if(isKingExposed())
                            {
                                *it=undo_move;
                                if(capture==true)
                                {
                                    fPlayer1->fPiece->push_back(fCaptured_Piece_Player2->at(fCaptured_Piece_Player2->size()-1));
                                    fCaptured_Piece_Player2->pop_back();
                                }
                                valid=false;
                            }
                            else
                            {
                                *it=undo_move;
                                fBoard->at(temp2->getEnpassant().piece.fX)->at(temp2->getEnpassant().piece.fY).value=-1;
                            }
                            break;
                        }
                }
            }
        }
    }
    return valid;
}
