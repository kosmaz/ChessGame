#include "Lauretta.hpp"
#include <cmath>
using namespace std;


Lauretta::Lauretta(int color,Player* opponent,string name):Player(color,name),fMoves_Made(nullptr),fOpponent(opponent),
    fDummy1(nullptr),fDummy2(nullptr),fDummy1_Captured_Pieces(nullptr),fDummy2_Captured_Pieces(nullptr),fDummy_Turn(-1),
    fIsEnpassant(false),fIsCastling(false),fPawn_Promotion(false),fDummy_Ply_Count(0),fMax_Depth(0){}

Lauretta::Lauretta(int color,string name):Player(color,name),fMoves_Made(nullptr),fOpponent(nullptr),fDummy1(nullptr),
    fDummy2(nullptr),fDummy1_Captured_Pieces(nullptr),fDummy2_Captured_Pieces(nullptr),fDummy_Turn(-1),fIsEnpassant(false),
    fIsCastling(false),fPawn_Promotion(false),fDummy_Ply_Count(0),fMax_Depth(0){}


Lauretta::Lauretta(const Lauretta& source):Player(source),fMoves_Made(nullptr),fOpponent(nullptr),fDummy1(nullptr),fDummy2(nullptr),
    fDummy1_Captured_Pieces(nullptr),fDummy2_Captured_Pieces(nullptr),fDummy_Turn(-1),fIsEnpassant(false),fIsCastling(false),
    fPawn_Promotion(false),fDummy_Ply_Count(0),fMax_Depth(0){}


Lauretta::~Lauretta(){}


string Lauretta::requestCommand()
{
    fLast_Command=thinkAndPlay();
    return fLast_Command;
}


string Lauretta::thinkAndPlay()
{
    fDummy1=new Player(*this);
    fDummy2=new Player(*fOpponent);

#ifdef DEBUG
    NODES=0;
#endif

    fMoves_Made=new vector<Move>;
    fDummy1_Captured_Pieces=new vector<chessPiece>;
    fDummy2_Captured_Pieces=new vector<chessPiece>;

    initializeEvaluateTables();
    initializeKillerMoves();
    initializeHistoryMoves();

    (void)alphaBeta(fMax_Depth,-INF,INF,fMy_Turn);

    delete fDummy1;
    delete fDummy2;
    delete fMoves_Made;
    delete fDummy1_Captured_Pieces;
    delete fDummy2_Captured_Pieces;

    clearEvaluateTables();
    clearKillerMoves();
    clearHistoryMoves();

    return convertDigitalToCommand(fBest_Move.from,fBest_Move.to);
}


string Lauretta::convertDigitalToCommand(const chessPiece& from,const chessPiece& to)
{
    string command;
    char alpha[8]={'a','b','c','d','e','f','g','h'};
    command.push_back(alpha[from.fX]);
    command.push_back('0'+(from.fY+1));
    command.push_back(alpha[to.fX]);
    command.push_back('0'+(to.fY+1));
    return command;
}


bool Lauretta::dummyMovePiece(chessPiece& from,chessPiece to)
{
    bool valid=false;
    if(isMoveValid(from,to))
    {
        valid=true;
        if(fDummy1->myTurn()==fDummy_Turn)
        {
            for(vector<chessPiece>::iterator it=fDummy1->fPiece->begin(); it!=fDummy1->fPiece->end(); ++it)
                if(from==*it)
                {
                    chessPiece undo_move(*it);
                    to.fValue=(*it).fValue;
                    *it=to;

                    bool capture=false;
                    chessPiece undo;
                    for(vector<chessPiece>::iterator it2=fDummy2->fPiece->begin(); it2!=fDummy2->fPiece->end(); ++it2)
                        if(to==*it2)
                        {
                            undo=*it2;
                            fDummy2->fPiece->erase(it2);
                            capture=true;
                            break;
                        }

                    if((to.fY==7 && fDummy1->myTurn()==WHITE) && (*it).fValue==0)
                        fPawn_Promotion=true;
                    else if((to.fY==0 && fDummy1->myTurn()==BLACK) && (*it).fValue==0)
                        fPawn_Promotion=true;

                    if(isKingExposed())
                    {
                        *it=undo_move;
                        valid=false;
                    }
                    else
                        *it=undo_move;

                    if(capture==true)
                        fDummy2->fPiece->push_back(undo);
                    break;
                }
        }
        else
        {
            for(vector<chessPiece>::iterator it2=fDummy2->fPiece->begin(); it2!=fDummy2->fPiece->end(); ++it2)
                if(from==*it2)
                {
                    chessPiece undo_move(*it2);
                    to.fValue=(*it2).fValue;
                    *it2=to;

                    if((to.fY==7 && fDummy2->myTurn()==WHITE) && (*it2).fValue==0)
                        fPawn_Promotion=true;
                    else if((to.fY==0 && fDummy2->myTurn()==BLACK) && (*it2).fValue==0)
                        fPawn_Promotion=true;

                    bool capture=false;
                    chessPiece undo;
                    for(vector<chessPiece>::iterator it3=fDummy1->fPiece->begin(); it3!=fDummy1->fPiece->end(); ++it3)
                        if(to==*it3)
                        {
                            undo=*it3;
                            fDummy1->fPiece->erase(it3);
                            capture=true;
                            break;
                        }

                    if(isKingExposed())
                    {
                        *it2=undo_move;
                        valid=false;
                    }
                    else
                        *it2=undo_move;
                    if(capture==true)
                        fDummy1->fPiece->push_back(undo);
                    break;
                }
        }
    }
    return valid;
}


bool Lauretta::isMoveValid(chessPiece& from, chessPiece& to)
{
    Player* temp=(fDummy_Turn==fDummy1->myTurn())? fDummy1:fDummy2;
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
                    if(isMoveStraight(from,to,2) || isMoveDiagonal(from,to,2))
                        valid=true;
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
                        valid=true;
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
                    if(isMoveStraight(from,to,1) || isMoveSideways(from,to,1) || isMoveDiagonal(from,to,1))
                        valid=true;
                    break;
                }
                default:break;
            }
            break;
        }

    return valid;
}


bool Lauretta::isPieceValid(chessPiece& piece)
{
    bool valid=true;
    if(piece.fX<0 || piece.fX>7 || piece.fY<0 || piece.fY>7)
        valid=false;
    return valid;
}


bool Lauretta::isMoveDiagonal(chessPiece& from,chessPiece& to,int king)
{
    bool valid=false;
    Player* temp=(fDummy1->myTurn()==fDummy_Turn)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==fDummy_Turn)? fDummy2:fDummy1;
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
                valid=isEnpassant(from,to);
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
                valid=isEnpassant(from,to);
        }
    }

    return valid;
}


bool Lauretta::isMoveStraight(chessPiece& from,chessPiece& to, int pawn)
{
    bool valid=false;
    Player* temp=(fDummy1->myTurn()==fDummy_Turn)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==fDummy_Turn)? fDummy2:fDummy1;
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
        if(from.fX==to.fX && (abs(from.fY-to.fY)==2) && (from.fY==6 || from.fY==1))
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
    }

    return valid;
}


bool Lauretta::isMoveSideways(chessPiece& from,chessPiece& to, int king)
{
    bool valid=false;
    Player* temp=(fDummy1->myTurn()==fDummy_Turn)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==fDummy_Turn)? fDummy2:fDummy1;
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
            valid=isCastling(to);
    }

    return valid;
}


bool Lauretta::isMoveKnight(chessPiece& from,chessPiece& to)
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


bool Lauretta::isKingExposed()
{
    bool valid=false;
    Player* temp=(fDummy1->myTurn()==fDummy_Turn)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==fDummy_Turn)? fDummy2:fDummy1;
    fDummy_Turn^=1;
    for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
        if(isMoveValid(*it,*temp->king()))
        {
            valid=true;
            break;
        }
    fDummy_Turn^=1;
    return valid;
}


bool Lauretta::isKingChecked()
{
    Player* temp=(fDummy1->myTurn()==fDummy_Turn)? fDummy2:fDummy1;
    fDummy_Turn^=1;
    bool valid=isKingExposed();
    fDummy_Turn^=1;
    temp->setKingChecked(valid);
    return valid;
}


bool Lauretta::isCastling(chessPiece& to)
{
    Player* temp=(fDummy1->myTurn()==fDummy_Turn)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==fDummy_Turn)? fDummy2:fDummy1;
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
            fDummy_Turn^=1;
            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if(((*it).fX==5 && (*it).fY==0) || ((*it).fX==6 && (*it).fY==0) || dummyMovePiece(*it,chessPiece(5,0)) || dummyMovePiece(*it,chessPiece(6,0)))
                {
                    valid=false;
                    break;
                }
            fDummy_Turn^=1;
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
            fDummy_Turn^=1;
            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if(((*it).fX==2 && (*it).fY==0) || ((*it).fX==3 && (*it).fY==0) || dummyMovePiece(*it,chessPiece(2,0)) || dummyMovePiece(*it,chessPiece(3,0)))
                {
                    valid=false;
                    break;
                }
            fDummy_Turn^=1;
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
            fDummy_Turn^=1;
            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if(((*it).fX==5 && (*it).fY==7) || ((*it).fX==6 && (*it).fY==7) || dummyMovePiece(*it,chessPiece(5,7)) || dummyMovePiece(*it,chessPiece(6,7)))
                {
                    valid=false;
                    break;
                }
            fDummy_Turn^=1;
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
            fDummy_Turn^=1;
            for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                if(((*it).fX==2 && (*it).fY==7) || ((*it).fX==3 && (*it).fY==7) || dummyMovePiece(*it,chessPiece(2,7)) || dummyMovePiece(*it,chessPiece(3,7)))
                {
                    valid=false;
                    break;
                }
            fDummy_Turn^=1;
        }
    }
    if(valid==true)
        fIsCastling=true;
    return valid;
}


bool Lauretta::canProtectKing(chessPiece& destination)
{
    bool valid=false;
    if(isKingChecked())
    {
        Player* temp=(fDummy1->myTurn()==fDummy_Turn)? fDummy1:fDummy2;
        Player* temp2=(fDummy1->myTurn()==fDummy_Turn)? fDummy2:fDummy1;

        fDummy_Turn^=1;
        for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
            if(dummyMovePiece(*it,destination))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
                                if(dummyMovePiece(*it3,*it2))
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
            fDummy_Turn^=1;
        }
    }
    return valid;
}


bool Lauretta::hasKingValidMove(bool check)
{
    bool valid=false;

    if(isKingChecked() || check==false)
    {
        Player* temp=(fDummy1->myTurn()==fDummy_Turn)? fDummy2:fDummy1;

        vector<chessPiece> possible_king_moves;
        possible_king_moves.push_back(chessPiece(temp->king()->fX-1,temp->king()->fY+1));
        possible_king_moves.push_back(chessPiece(temp->king()->fX-1,temp->king()->fY+0));
        possible_king_moves.push_back(chessPiece(temp->king()->fX-1,temp->king()->fY-1));
        possible_king_moves.push_back(chessPiece(temp->king()->fX+0,temp->king()->fY+1));
        possible_king_moves.push_back(chessPiece(temp->king()->fX+0,temp->king()->fY-1));
        possible_king_moves.push_back(chessPiece(temp->king()->fX+1,temp->king()->fY+1));
        possible_king_moves.push_back(chessPiece(temp->king()->fX+1,temp->king()->fY+0));
        possible_king_moves.push_back(chessPiece(temp->king()->fX+1,temp->king()->fY-1));

        for(unsigned it=0; it<possible_king_moves.size(); ++it)
            if((possible_king_moves[it].fX<0 || possible_king_moves[it].fX>7) || (possible_king_moves[it].fY<0 || possible_king_moves[it].fY>7))
            {
                possible_king_moves.erase(possible_king_moves.begin()+it);
                --it;
            }

        int moves=0;
        fDummy_Turn^=1;
        for(vector<chessPiece>::iterator it=possible_king_moves.begin(); it!=possible_king_moves.end(); ++it)
            if(dummyMovePiece(*temp->king(),*it))
            {
                ++moves;
                break;
            }
        fDummy_Turn^=1;
        if(moves)
            valid=true;
    }
    return valid;
}


bool Lauretta::isKingCheckMated(chessPiece& destination)
{
    bool valid=false;
    if(isKingChecked())
        if(!hasKingValidMove() && !canProtectKing(destination))
                valid=true;
    return valid;
}


bool Lauretta::isEnpassant(chessPiece& from,chessPiece& to)
{
    bool valid=false;
    Player* temp=(fDummy1->myTurn()==fDummy_Turn)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==fDummy_Turn)? fDummy2:fDummy1;

    if(temp->myTurn()==0)
    {
        if((temp2->getEnpassant().piece.fY==4 && from.fY==4) && temp2->getEnpassant().valid==true &&
                (fDummy_Ply_Count-temp2->getEnpassant().ply==0) && abs(temp2->getEnpassant().piece.fX-from.fX)==1)
        {
            if(to==chessPiece(temp2->getEnpassant().piece.fX,temp2->getEnpassant().piece.fY+1))
            {
                valid=true;
                bool capture=false;
                chessPiece undo;
                for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                    if(temp2->getEnpassant().piece==*it)
                    {
                        undo=*it;
                        temp2->fPiece->erase(it);
                        capture=true;
                        break;
                    }
                for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                    if(from==*it)
                    {
                        chessPiece undo_move(*it);
                        to.fValue=(*it).fValue;
                        *it=to;

                        if(isKingExposed())
                        {
                            if(capture==true)
                                temp2->fPiece->push_back(undo);
                            valid=false;
                        }
                        else if(capture==true)
                            temp2->fPiece->push_back(undo);
                        *it=undo_move;
                        break;
                    }
            }
        }
    }
    else
    {
        if((temp2->getEnpassant().piece.fY==3 && from.fY==3) && temp2->getEnpassant().valid==true &&
            (fDummy_Ply_Count-temp2->getEnpassant().ply==0) && abs(temp2->getEnpassant().piece.fX-from.fX)==1)
        {
            if(to==chessPiece(temp2->getEnpassant().piece.fX,temp2->getEnpassant().piece.fY-1))
            {
                valid=true;
                bool capture=false;
                chessPiece undo;
                for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
                    if(temp2->getEnpassant().piece==*it)
                    {
                        undo=*it;
                        temp2->fPiece->erase(it);
                        capture=true;
                        break;
                    }
                for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
                    if(from==*it)
                    {
                        chessPiece undo_move(*it);
                        to.fValue=(*it).fValue;
                        *it=to;

                        if(isKingExposed())
                        {
                            if(capture==true)
                                temp2->fPiece->push_back(undo);
                            valid=false;
                        }
                        else if(capture==true)
                            temp2->fPiece->push_back(undo);
                        *it=undo_move;
                        break;
                    }
            }
        }
    }

    bool found=false;
    for(vector<chessPiece>::iterator it=temp2->fPiece->begin(); it!=temp2->fPiece->end(); ++it)
        if(temp2->getEnpassant().piece==*it)
        {
            found=true;
            break;
        }

    if(valid==true && found==true)
        fIsEnpassant=true;
    return valid;
}


void Lauretta::makeMove(Move& move)
{
    ++fDummy_Ply_Count;
    Player* temp=(fDummy1->myTurn()==move.side)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==move.side)? fDummy2:fDummy1;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        if(*it==move.from)
        {
            switch(move.piece)
            {
                case 0:
                {
                    *it=move.to;
                    it->fValue=move.piece;
                    if(move.isEnpassantCapture==true)
                        removePiece(move.enpassantPiece,temp2);
                    else if(move.capture==true)
                        removePiece(move.to,temp2);
                    else if(abs(move.from.fY-move.to.fY)==2)
                        temp->setEnpassant(true,fDummy_Ply_Count,*it);

                    if(move.isPawnPromoted==true)
                        it->fValue=move.promotedTo;
                    break;
                }

                case 3:
                {
                    *it=move.to;
                    it->fValue=move.piece;
                    if(move.capture==true)
                        removePiece(move.to,temp2);

                    if(move.from.fX==0 && !temp->hasLeftRookMoved())
                        temp->setLeftRookMoved(true);
                    else if(move.from.fX==7 && !temp->hasRightRookMoved())
                        temp->setRightRookMoved(true);
                    break;
                }

                case 5:
                {
                    *it=move.to;
                    it->fValue=move.piece;
                    if(move.capture==true)
                        removePiece(move.to,temp2);

                    if(move.from.fX==4 && !temp->hasKingMoved())
                        temp->setKingMoved(true);

                    if(move.iscastling==true)
                    {
                        if(move.to.fX==6)
                        {
                            for(vector<chessPiece>::iterator it2=temp->fPiece->begin(); it2!=temp->fPiece->end(); ++it2)
                                if(it2->fX==7 && it2->fValue==3)
                                {
                                    it2->fX=5;
                                    temp->setRightRookMoved(true);
                                    break;
                                }
                        }
                        else if(move.to.fX==2)
                        {
                            for(vector<chessPiece>::iterator it2=temp->fPiece->begin(); it2!=temp->fPiece->end(); ++it2)
                                if(it2->fX==0 && it2->fValue==3)
                                {
                                    it2->fX=3;
                                    temp->setLeftRookMoved(true);
                                    break;
                                }
                        }
                    }
                    break;
                }

                case 1:
                {
                    /*fall through to case 2 since there are no special cases
                     *in moving a Knight, a Bishop and a Queen
                     */
                }

                case 2:
                {
                    /*fall through to case 4 since there are no special cases
                     *in moving a Knight, a Bishop and a Queen
                     */
                }

                case 4:
                {
                    *it=move.to;
                    it->fValue=move.piece;
                    if(move.capture==true)
                        removePiece(move.to,temp2);
                    break;
                }
            }
            break;
        }
    move.isKingChecked=isKingChecked();
    fMoves_Made->push_back(move);
    return;
}


void Lauretta::takeMove()
{
    Move move=fMoves_Made->at(fMoves_Made->size()-1);

    --fDummy_Ply_Count;

    Player* temp=(fDummy1->myTurn()==move.side)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==move.side)? fDummy2:fDummy1;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        if(*it==move.to)
        {
            switch(move.piece)
            {
                case 0:
                {
                    *it=move.from;
                    it->fValue=move.piece;
                    if(move.isEnpassantCapture==true)
                        restorePiece(temp2);
                    else if(move.capture==true)
                        restorePiece(temp2);
                    else if(abs(move.from.fY-move.to.fY)==2)
                        temp->setEnpassant(false,fDummy_Ply_Count);

                    if(move.isPawnPromoted==true)
                        it->fValue=move.piece;
                    break;
                }

                case 3:
                {
                    *it=move.from;
                    it->fValue=move.piece;
                    if(move.capture==true)
                        restorePiece(temp2);

                    if(move.from.fX==0 && temp->hasLeftRookMoved())
                        temp->setLeftRookMoved(false);
                    else if(move.from.fX==7 && temp->hasRightRookMoved())
                        temp->setRightRookMoved(false);
                    break;
                }

                case 5:
                {
                    *it=move.from;
                    it->fValue=move.piece;
                    if(move.capture==true)
                        restorePiece(temp2);

                    if(move.from.fX==4 && temp->hasKingMoved())
                        temp->setKingMoved(false);

                    if(move.iscastling==true)
                    {
                        if(move.to.fX==6)
                        {
                            for(vector<chessPiece>::iterator it2=temp->fPiece->begin(); it2!=temp->fPiece->end(); ++it2)
                                if(it2->fX==5 && it2->fValue==3)
                                {
                                    it2->fX=7;
                                    temp->setRightRookMoved(false);
                                    break;
                                }
                        }
                        else if(move.to.fX==2)
                        {
                            for(vector<chessPiece>::iterator it2=temp->fPiece->begin(); it2!=temp->fPiece->end(); ++it2)
                                if(it2->fX==3 && it2->fValue==3)
                                {
                                    it2->fX=0;
                                    temp->setLeftRookMoved(false);
                                    break;
                                }
                        }
                    }
                    break;
                }

                case 1:
                {
                    /*fall through to case 2 since there are no special cases
                     *in moving a Knight, a Bishop and a Queen
                     */
                }

                case 2:
                {
                    /*fall through to case 4 since there are no special cases
                     *in moving a Knight, a Bishop and a Queen
                     */
                }

                case 4:
                {
                    *it=move.from;
                    it->fValue=move.piece;
                    if(move.capture==true)
                        restorePiece(temp2);
                    break;
                }
            }
            break;
        }
    if(move.isKingChecked==true)
        temp2->setKingChecked(false);
    fMoves_Made->pop_back();
    return;
}


void Lauretta::pickNextMove(vector<Move>::iterator move, vector<Move>* list)
{
    for(vector<Move>::iterator it=move; it!=list->end(); ++it)
        if(move->score<it->score)
        {
            Move temp(*it);
            *it=*move;
            *move=temp;
        }
    return;
}


void Lauretta::removePiece(chessPiece& piece, Player* player)
{
    for(vector<chessPiece>::iterator it=player->fPiece->begin(); it!=player->fPiece->end(); ++it)
        if(piece==*it)
        {
            if(player->myTurn()==fDummy1->myTurn())
            {
                fDummy2_Captured_Pieces->push_back(*it);
                fDummy1->fPiece->erase(it);
            }
            else
            {
                fDummy1_Captured_Pieces->push_back(*it);
                fDummy2->fPiece->erase(it);
            }
            break;
        }
    return;
}


void Lauretta::restorePiece(Player* player)
{
    if(player->myTurn()==fDummy1->myTurn())
    {
        fDummy1->fPiece->push_back(fDummy2_Captured_Pieces->at(fDummy2_Captured_Pieces->size()-1));
        fDummy2_Captured_Pieces->pop_back();
    }
    else
    {
        fDummy2->fPiece->push_back(fDummy1_Captured_Pieces->at(fDummy1_Captured_Pieces->size()-1));
        fDummy1_Captured_Pieces->pop_back();
    }

    return;
}


std::vector<Move>* Lauretta::generateMoves(bool side,bool capture)
{
    vector<Move>* movelist=new vector<Move>;
    Player* temp=(fDummy1->myTurn()==side)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==side)? fDummy2:fDummy1;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        switch(it->fValue)
        {
            case 0:
            {
                vector<chessPiece> possible_moves;
                if(temp->myTurn()==0)
                {
                    possible_moves.push_back(chessPiece(it->fX,it->fY+1));
                    possible_moves.push_back(chessPiece(it->fX,it->fY+2));
                    possible_moves.push_back(chessPiece(it->fX-1,it->fY+1));
                    possible_moves.push_back(chessPiece(it->fX+1,it->fY+1));
                }
                else
                {
                    possible_moves.push_back(chessPiece(it->fX,it->fY-1));
                    possible_moves.push_back(chessPiece(it->fX,it->fY-2));
                    possible_moves.push_back(chessPiece(it->fX-1,it->fY-1));
                    possible_moves.push_back(chessPiece(it->fX+1,it->fY-1));
                }
                for(unsigned i=0; i<possible_moves.size(); ++i)
                    if(!isPieceValid(possible_moves.at(i)))
                    {
                        possible_moves.erase(possible_moves.begin()+i);
                        --i;
                    }

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                {
                    fIsEnpassant=false;
                    fPawn_Promotion=false;
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                    {
                        Move move;
                        move.ply=fDummy_Ply_Count+1;
                        move.from=*it;
                        move.to=possible_moves.at(it2);
                        move.piece=it->fValue;
                        move.side=side;
                        move.isEnpassantCapture=fIsEnpassant;

                        if(move.isEnpassantCapture==true)
                        {
                            move.victim=0;
                            move.score=moveScore(move.piece,move.victim);
                            move.enpassantPiece=temp2->getEnpassant().piece;
                        }
                        else
                        {
                            fDummy_Turn^=1;
                            for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                                if(*it3==possible_moves.at(it2))
                                {
                                    move.victim=it3->fValue;
                                    move.score=moveScore(move.piece,move.victim);
                                    move.capture=true;
                                    break;
                                }
                            fDummy_Turn^=1;

                            if(fPawn_Promotion==true)
                            {
                                for(int i=4; i>=1; --i)
                                {
                                    Move promote(move);
                                    promote.promotedTo=i;
                                    if(!move.capture && !move.isEnpassantCapture && !capture)
                                    {
                                        if(fKiller_Moves->at(0)->at(move.ply-1)==promote)
                                            move.score=900000;
                                        else if(fKiller_Moves->at(1)->at(move.ply-1)==promote)
                                            move.score=800000;
                                        else
                                            move.score=fHistory_Moves->at(indexSquare(promote.from))->at(indexSquare(promote.to));
                                    }
                                    movelist->push_back(promote);
                                }
                            }
                        }
                        if(!move.capture && !move.isEnpassantCapture && !fPawn_Promotion && !capture)
                        {
                            if(fKiller_Moves->at(0)->at(move.ply-1)==move)
                                move.score=900000;
                            else if(fKiller_Moves->at(1)->at(move.ply-1)==move)
                                move.score=800000;
                            else
                                move.score=fHistory_Moves->at(indexSquare(move.from))->at(indexSquare(move.to));
                        }
                        if(!fPawn_Promotion)
                            movelist->push_back(move);
                    }
                }
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
                for(unsigned i=0; i<possible_moves.size(); ++i)
                    if(!isPieceValid(possible_moves.at(i)))
                    {
                        possible_moves.erase(possible_moves.begin()+i);
                        --i;
                    }

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                    {
                        Move move;
                        move.ply=fDummy_Ply_Count+1;
                        move.from=*it;
                        move.to=possible_moves.at(it2);
                        move.piece=it->fValue;
                        move.side=side;

                        fDummy_Turn^=1;
                        for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                            if(*it3==possible_moves.at(it2))
                            {
                                move.victim=it3->fValue;
                                move.score=moveScore(move.piece,move.victim);
                                move.capture=true;
                                break;
                            }
                        fDummy_Turn^=1;
                        if(!move.capture && !capture)
                        {
                            if(fKiller_Moves->at(0)->at(move.ply-1)==move)
                                move.score=900000;
                            else if(fKiller_Moves->at(1)->at(move.ply-1)==move)
                                move.score=800000;
                            else
                                move.score=fHistory_Moves->at(indexSquare(move.from))->at(indexSquare(move.to));
                        }
                        movelist->push_back(move);
                    }
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

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                    {
                        Move move;
                        move.ply=fDummy_Ply_Count+1;
                        move.from=*it;
                        move.to=possible_moves.at(it2);
                        move.piece=it->fValue;
                        move.side=side;

                        fDummy_Turn^=1;
                        for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                            if(*it3==possible_moves.at(it2))
                            {
                                move.victim=it3->fValue;
                                move.score=moveScore(move.piece,move.victim);
                                move.capture=true;
                                break;
                            }
                        fDummy_Turn^=1;
                        if(!move.capture && !capture)
                        {
                            if(fKiller_Moves->at(0)->at(move.ply-1)==move)
                                move.score=900000;
                            else if(fKiller_Moves->at(1)->at(move.ply-1)==move)
                                move.score=800000;
                            else
                                move.score=fHistory_Moves->at(indexSquare(move.from))->at(indexSquare(move.to));
                        }
                        movelist->push_back(move);
                    }
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

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                    {
                        Move move;
                        move.ply=fDummy_Ply_Count+1;
                        move.from=*it;
                        move.to=possible_moves.at(it2);
                        move.piece=it->fValue;
                        move.side=side;

                        fDummy_Turn^=1;
                        for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                            if(*it3==possible_moves.at(it2))
                            {
                                move.victim=it3->fValue;
                                move.score=moveScore(move.piece,move.victim);
                                move.capture=true;
                                break;
                            }
                        fDummy_Turn^=1;
                        if(!move.capture && !capture)
                        {
                            if(fKiller_Moves->at(0)->at(move.ply-1)==move)
                                move.score=900000;
                            else if(fKiller_Moves->at(1)->at(move.ply-1)==move)
                                move.score=800000;
                            else
                                move.score=fHistory_Moves->at(indexSquare(move.from))->at(indexSquare(move.to));
                        }
                        movelist->push_back(move);
                    }
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

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                    {
                        Move move;
                        move.ply=fDummy_Ply_Count+1;
                        move.from=*it;
                        move.to=possible_moves.at(it2);
                        move.piece=it->fValue;
                        move.side=side;

                        fDummy_Turn^=1;
                        for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                            if(*it3==possible_moves.at(it2))
                            {
                                move.victim=it3->fValue;
                                move.score=moveScore(move.piece,move.victim);
                                move.capture=true;
                                break;
                            }
                        fDummy_Turn^=1;
                        if(!move.capture && !capture)
                        {
                            if(fKiller_Moves->at(0)->at(move.ply-1)==move)
                                move.score=900000;
                            else if(fKiller_Moves->at(1)->at(move.ply-1)==move)
                                move.score=800000;
                            else
                                move.score=fHistory_Moves->at(indexSquare(move.from))->at(indexSquare(move.to));
                        }
                        movelist->push_back(move);
                    }
                break;
            }


            case 5:
            {
                vector<chessPiece> possible_moves;
                possible_moves.push_back(chessPiece(it->fX-1,it->fY+1));
                possible_moves.push_back(chessPiece(it->fX-1,it->fY+0));
                possible_moves.push_back(chessPiece(it->fX-1,it->fY-1));
                possible_moves.push_back(chessPiece(it->fX+0,it->fY+1));
                possible_moves.push_back(chessPiece(it->fX+0,it->fY-1));
                possible_moves.push_back(chessPiece(it->fX+1,it->fY+1));
                possible_moves.push_back(chessPiece(it->fX+1,it->fY+0));
                possible_moves.push_back(chessPiece(it->fX+1,it->fY-1));

                possible_moves.push_back(chessPiece(it->fX-2,it->fY+0));
                possible_moves.push_back(chessPiece(it->fX+2,it->fY+0));

                for(unsigned i=0; i<possible_moves.size(); ++i)
                    if(!isPieceValid(possible_moves.at(i)))
                    {
                        possible_moves.erase(possible_moves.begin()+i);
                        --i;
                    }

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                {
                    fIsCastling=false;
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                    {
                        Move move;
                        move.ply=fDummy_Ply_Count+1;
                        move.from=*it;
                        move.to=possible_moves.at(it2);
                        move.piece=it->fValue;
                        move.side=side;
                        move.iscastling=fIsCastling;

                        fDummy_Turn^=1;
                        for(vector<chessPiece>::iterator it3=temp2->fPiece->begin(); it3!=temp2->fPiece->end(); ++it3)
                            if(*it3==possible_moves.at(it2))
                            {
                                move.victim=it3->fValue;
                                move.score=moveScore(move.piece,move.victim);
                                move.capture=true;
                                break;
                            }
                        fDummy_Turn^=1;
                        if(!move.capture && !capture)
                        {
                            if(fKiller_Moves->at(0)->at(move.ply-1)==move)
                                move.score=900000;
                            else if(fKiller_Moves->at(1)->at(move.ply-1)==move)
                                move.score=800000;
                            else
                                move.score=fHistory_Moves->at(indexSquare(move.from))->at(indexSquare(move.to));
                        }
                        movelist->push_back(move);
                    }
                }
                break;
            }

            default:break;
        }
    if(capture==true)
        for(vector<Move>::iterator it4=movelist->begin(); it4!=movelist->end(); ++it4)
            if(!it4->capture && !it4->isEnpassantCapture)
            {
                movelist->erase(it4);
                --it4;
            }

    return movelist;
}


int Lauretta::alphaBeta(int depth, int alpha, int beta, bool minimizing)
{
#ifdef DEBUG
    if(depth!=0)
        ++NODES;
#endif

    fDummy_Turn=minimizing;
    int movescore=-INF;

    if(depth==0)
        return quiescenceSearch(alpha,beta,minimizing);

    vector<Move>* movelist=generateMoves(minimizing);
    if(!movelist->size())
    {
        delete movelist;
        fDummy_Turn^=1;
        if(isKingChecked())
        {
            fDummy_Turn^=1;
            return -CHECKMATE+fDummy_Ply_Count;
        }
        else
        {
            fDummy_Turn^=1;
            return 0;
        }
        fDummy_Turn^=1;
    }

    for(vector<Move>::iterator it=movelist->begin(); it!=movelist->end(); ++it)
    {
        pickNextMove(it,movelist);
        makeMove(*it);
        movescore=-alphaBeta(depth-1,-beta,-alpha,minimizing^true);
        takeMove();

        if(movescore>alpha)
        {
            if(movescore>=beta)
            {
                if(!it->capture && !it->isEnpassantCapture)
                {
                    fKiller_Moves->at(1)->at(it->ply-1)=fKiller_Moves->at(0)->at(it->ply-1);
                    fKiller_Moves->at(0)->at(it->ply-1)=*it;
                }
                delete movelist;
                return beta;
            }
            alpha=movescore;
            if(!it->capture && !it->isEnpassantCapture)
                fHistory_Moves->at(indexSquare(it->from))->at(indexSquare(it->to))+=depth;
            if(depth==fMax_Depth)
                fBest_Move=*it;
        }
    }
    delete movelist;
    return alpha;
}


int Lauretta::quiescenceSearch(int alpha, int beta, bool minimizing)
{
#ifdef DEBUG
    ++NODES;
#endif

    int movescore=scoreFromSidePOV(minimizing);

    if(abs(movescore)>28000)
        return movescore;

    if(movescore>=beta)
        return beta;
    if(movescore>alpha)
        alpha=movescore;

    fDummy_Turn=minimizing;
    movescore=-INF;

    vector<Move>* movelist=generateMoves(minimizing,true);
    if(movelist->size())
    {
        for(vector<Move>::iterator it=movelist->begin(); it!=movelist->end(); ++it)
        {
            pickNextMove(it,movelist);
            makeMove(*it);
            movescore=-quiescenceSearch(-beta,-alpha,minimizing^true);
            takeMove();
            if(movescore>alpha)
            {
                if(movescore>=beta)
                {
                    delete movelist;
                    return beta;
                }
                alpha=movescore;
            }
        }
    }
    return alpha;
}


int Lauretta::scoreFromSidePOV(bool side)
{
    fDummy_Turn^=1;
    if(isKingCheckMated(fMoves_Made->at(fMoves_Made->size()-1).to))
    {
        fDummy_Turn^=1;
        return -CHECKMATE+fDummy_Ply_Count;
    }
    fDummy_Turn^=1;

    countAllPiece();
    if(isMaterialDraw())
        return 0;

    int score=material(WHITE)-material(BLACK);

    if(isBishopPair(WHITE))
        score+=BISHOPPAIR;
    if(isBishopPair(BLACK))
        score-=BISHOPPAIR;

    if(!fWhite_Piece_Count->at(0))
        score+=NOPAWN;
    if(!fBlack_Piece_Count->at(0))
        score-=NOPAWN;

    /*removed mobility scoring due to time taken in generating legal moves
     * for both sides which badly affects the overall time of the alpha-beta
     * search thereby slowing the engine down.
     * Will reimpement mobility scoring when move generation has been moved
     * to bit board representation which is faster than index board representation
     *score+=mobility(WHITE)-mobility(BLACK);
     */

    Player* temp=(fDummy1->myTurn()==WHITE)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==WHITE)? fDummy2:fDummy1;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        switch(it->fValue)
        {
            case 0:
            {
                score+=fPawn_Table->at(indexSquare(*it));
                if(isDoublePawn(*it,WHITE))
                    score+=DOUBLEPAWN;
                if(isIsolatedPawn(*it,WHITE))
                    score+=ISOLATEDPAWN;
                if(isPassedPawn(*it,WHITE))
                    score+=fPassed_Pawn_Rank->at(it->fY);
                break;
            }

            case 1: score+=fKnight_Table->at(indexSquare(*it));
                break;

            case 2: score+=fBishop_Table->at(indexSquare(*it));
                break;

            case 3:
            {
                score+=fRook_Table->at(indexSquare(*it));
                if(isOpenFile(*it,WHITE,false))
                    score+=ROOKOPENFILE;
                else if(isOpenFile(*it,WHITE,true))
                    score+=ROOKSEMIOPENFILE;
                break;
            }

            case 4:
            {
                score+=fQueen_Table->at(indexSquare(*it));
                if(isOpenFile(*it,WHITE,false))
                    score+=QUEENOPENFILE;
                else if(isOpenFile(*it,WHITE,true))
                    score+=QUEENSEMIOPENFILE;
                break;
            }

            case 5:
            {
                if(!fBlack_Piece_Count->at(4) && (material(BLACK)<=endGameMaterial()))
                    score+=fKing_Table_End->at(indexSquare(*it));
                else
                    score+=fKing_Table_Begin->at(indexSquare(*it));
                break;
            }
        }

    for(vector<chessPiece>::iterator it2=temp2->fPiece->begin(); it2!=temp2->fPiece->end(); ++it2)
        switch(it2->fValue)
        {
            case 0:
            {
                score-=fPawn_Table->at(fMirror_Table->at(indexSquare(*it2)));
                if(isDoublePawn(*it2,BLACK))
                    score-=DOUBLEPAWN;
                if(isIsolatedPawn(*it2,BLACK))
                    score-=ISOLATEDPAWN;
                if(isPassedPawn(*it2,BLACK))
                    score-=fPassed_Pawn_Rank->at(7-it2->fY);
                break;
            }

            case 1: score-=fKnight_Table->at(fMirror_Table->at(indexSquare(*it2)));
                break;

            case 2: score-=fBishop_Table->at(fMirror_Table->at(indexSquare(*it2)));
                break;

            case 3:
            {
                score-=fRook_Table->at(fMirror_Table->at(indexSquare(*it2)));
                if(isOpenFile(*it2,BLACK,false))
                    score-=ROOKOPENFILE;
                else if(isOpenFile(*it2,BLACK,true))
                    score-=ROOKSEMIOPENFILE;
                break;
            }

            case 4:
            {
                score-=fQueen_Table->at(fMirror_Table->at(indexSquare(*it2)));
                if(isOpenFile(*it2,BLACK,false))
                    score-=QUEENOPENFILE;
                else if(isOpenFile(*it2,BLACK,true))
                    score-=QUEENSEMIOPENFILE;
                break;
            }

            case 5:
            {
                if(!fWhite_Piece_Count->at(4) && (material(WHITE)<=endGameMaterial()))
                    score-=fKing_Table_End->at(fMirror_Table->at(indexSquare(*it2)));
                else
                    score-=fKing_Table_Begin->at(fMirror_Table->at(indexSquare(*it2)));
                break;
            }
        }

    delete fWhite_Piece_Count;
    delete fBlack_Piece_Count;

    return (side==WHITE)? score:-score;
}


int Lauretta::material(bool side)
{
    int score=0;
    vector<int>* pieceCount=(side==WHITE)? fWhite_Piece_Count:fBlack_Piece_Count;
    score=ceil(1000+(500*pieceCount->at(4))+(300*pieceCount->at(3))+(208*pieceCount->at(2))+(207*pieceCount->at(1))+(100*pieceCount->at(0)));
    return score;
}


int Lauretta::mobility(bool side)
{
    int mobility_score=0;
    Player* temp=(fDummy1->myTurn()==side)? fDummy1:fDummy2;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        switch(it->fValue)
        {
            case 0:
            {
                vector<chessPiece> possible_moves;
                if(temp->myTurn()==0)
                {
                    possible_moves.push_back(chessPiece(it->fX,it->fY+1));
                    possible_moves.push_back(chessPiece(it->fX,it->fY+2));
                    possible_moves.push_back(chessPiece(it->fX-1,it->fY+1));
                    possible_moves.push_back(chessPiece(it->fX+1,it->fY+1));
                }
                else
                {
                    possible_moves.push_back(chessPiece(it->fX,it->fY-1));
                    possible_moves.push_back(chessPiece(it->fX,it->fY-2));
                    possible_moves.push_back(chessPiece(it->fX-1,it->fY-1));
                    possible_moves.push_back(chessPiece(it->fX+1,it->fY-1));
                }
                for(unsigned i=0; i<possible_moves.size(); ++i)
                    if(!isPieceValid(possible_moves.at(i)))
                    {
                        possible_moves.erase(possible_moves.begin()+i);
                        --i;
                    }

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                        ++mobility_score;
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
                for(unsigned i=0; i<possible_moves.size(); ++i)
                    if(!isPieceValid(possible_moves.at(i)))
                    {
                        possible_moves.erase(possible_moves.begin()+i);
                        --i;
                    }

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                        mobility_score+=2;
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

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                        mobility_score+=2;
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

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                        ++mobility_score;
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

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                        ++mobility_score;
                break;
            }


            case 5:
            {
                vector<chessPiece> possible_moves;
                possible_moves.push_back(chessPiece(it->fX-1,it->fY+1));
                possible_moves.push_back(chessPiece(it->fX-1,it->fY+0));
                possible_moves.push_back(chessPiece(it->fX-1,it->fY-1));
                possible_moves.push_back(chessPiece(it->fX+0,it->fY+1));
                possible_moves.push_back(chessPiece(it->fX+0,it->fY-1));
                possible_moves.push_back(chessPiece(it->fX+1,it->fY+1));
                possible_moves.push_back(chessPiece(it->fX+1,it->fY+0));
                possible_moves.push_back(chessPiece(it->fX+1,it->fY-1));

                possible_moves.push_back(chessPiece(it->fX-2,it->fY+0));
                possible_moves.push_back(chessPiece(it->fX+2,it->fY+0));

                for(unsigned i=0; i<possible_moves.size(); ++i)
                    if(!isPieceValid(possible_moves.at(i)))
                    {
                        possible_moves.erase(possible_moves.begin()+i);
                        --i;
                    }

                for(unsigned it2=0; it2<possible_moves.size(); ++it2)
                    if(dummyMovePiece(*it,possible_moves.at(it2)))
                        ++mobility_score;
                break;
            }
        }
    return mobility_score;
}


void Lauretta::countAllPiece()
{
    fWhite_Piece_Count=new vector<int>(5,0);
    fBlack_Piece_Count=new vector<int>(5,0);

    Player* temp=(fDummy1->myTurn()==WHITE)? fDummy1:fDummy2;
    Player* temp2=(fDummy1->myTurn()==WHITE)? fDummy2:fDummy1;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        switch(it->fValue)
        {
            case 0: ++fWhite_Piece_Count->at(0);
                break;
            case 1: ++fWhite_Piece_Count->at(1);
                break;
            case 2: ++fWhite_Piece_Count->at(2);
                break;
            case 3: ++fWhite_Piece_Count->at(3);
                break;
            case 4: ++fWhite_Piece_Count->at(4);
                break;
        }

    for(vector<chessPiece>::iterator it2=temp2->fPiece->begin(); it2!=temp2->fPiece->end(); ++it2)
        switch(it2->fValue)
        {
            case 0: ++fBlack_Piece_Count->at(0);
                break;
            case 1: ++fBlack_Piece_Count->at(1);
                break;
            case 2: ++fBlack_Piece_Count->at(2);
                break;
            case 3: ++fBlack_Piece_Count->at(3);
                break;
            case 4: ++fBlack_Piece_Count->at(4);
                break;
        }
    return;
}


bool Lauretta::isMaterialDraw()
{
    if(!fWhite_Piece_Count->at(3) && !fBlack_Piece_Count->at(3) && !fWhite_Piece_Count->at(4) && !fBlack_Piece_Count->at(4))
    {
          if(!fWhite_Piece_Count->at(2) && !fBlack_Piece_Count->at(2))
          {
              if(fWhite_Piece_Count->at(1)<3 && fBlack_Piece_Count->at(1)<3)
                return true;
          }
          else if(!fWhite_Piece_Count->at(1) && !fBlack_Piece_Count->at(1))
          {
             if(abs(fWhite_Piece_Count->at(2)-fBlack_Piece_Count->at(2))<2)
                return true;
          }
          else if((fWhite_Piece_Count->at(1)<3 && !fWhite_Piece_Count->at(2)) || (fWhite_Piece_Count->at(2)==1 && !fWhite_Piece_Count->at(1)))
          {
            if((fBlack_Piece_Count->at(1)<3 && !fBlack_Piece_Count->at(2)) || (fBlack_Piece_Count->at(2)==1 && !fBlack_Piece_Count->at(1)))
                return true;
          }
    }
    else if(!fWhite_Piece_Count->at(4) && !fBlack_Piece_Count->at(4))
    {
        if(fWhite_Piece_Count->at(3)==1 && fBlack_Piece_Count->at(3)==1)
        {
            if((fWhite_Piece_Count->at(1)+fWhite_Piece_Count->at(2))<2 && (fBlack_Piece_Count->at(1)+ fBlack_Piece_Count->at(2))<2)
                 return true;
        }
        else if(fWhite_Piece_Count->at(3)==1 && !fBlack_Piece_Count->at(3))
        {
            if(((fWhite_Piece_Count->at(1)+fWhite_Piece_Count->at(2))==0) && (((fBlack_Piece_Count->at(1)+fBlack_Piece_Count->at(2))==1) ||
                                                                              ((fBlack_Piece_Count->at(1)+fBlack_Piece_Count->at(2))==2)))
                 return true;
        }
        else if(fBlack_Piece_Count->at(3)==1 && !fWhite_Piece_Count->at(3))
        {
            if((fBlack_Piece_Count->at(1)+fBlack_Piece_Count->at(2)==0) && (((fWhite_Piece_Count->at(1)+fWhite_Piece_Count->at(1))==1) ||
                                                                            ((fWhite_Piece_Count->at(1)+fWhite_Piece_Count->at(3))== 2)))
                 return true;
        }
    }
  return false;
}


int Lauretta::endGameMaterial()
{
    /* values can still be modified to draw a better line between
     * middlegame and endgame which helps decides the best place
     * for the king piece to stay on the board using the King_Begin_Table
     * and the King_End_Table
     */
    return ceil((2*12.70) + (2*8.17) + (4*1.98));
}


bool Lauretta::isBishopPair(bool side)
{
    bool even=false;
    bool odd=false;
    Player* temp=(fDummy1->myTurn()==side)? fDummy1:fDummy2;
    vector<int>* pieceCount=(side==WHITE)? fWhite_Piece_Count:fBlack_Piece_Count;

    if(pieceCount->at(2)==2)
    {
        for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
            if(it->fValue==2)
            {
                if(((it->fX+it->fY)%2) && !odd)
                    odd=true;
                else if(!((it->fX+it->fY)%2) && !even)
                    even=true;
                if(even & odd)
                    break;
            }
    }
    return (even & odd);
}


bool Lauretta::isDoublePawn(chessPiece& piece, bool side)
{
    bool valid=false;
    Player* temp=(fDummy1->myTurn()==side)? fDummy1:fDummy2;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        if(it->fValue==0 && it->fX==piece.fX && *it!=piece)
        {
            if(side==WHITE && (it->fY < piece.fY))
            {
                valid=true;
                break;
            }
            else if(side==BLACK && (it->fY > piece.fY))
            {
                valid=true;
                break;
            }
        }
    return valid;
}


bool Lauretta::isPassedPawn(chessPiece& piece, bool side)
{
    bool valid=true;
    Player* temp=(fDummy1->myTurn()==side)? fDummy2:fDummy1;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        if(it->fValue==0)
        {
            if(it->fX==piece.fX || abs(it->fX-piece.fX)==1)
            {
                if(side==WHITE && (it->fY > piece.fY))
                {
                    valid=false;
                    break;
                }
                else if(side==BLACK && (it->fY < piece.fY))
                {
                    valid=false;
                    break;
                }
            }
        }
    return valid;
}


bool Lauretta::isIsolatedPawn(chessPiece& piece, bool side)
{
    bool valid=true;
    Player* temp=(fDummy1->myTurn()==side)? fDummy1:fDummy2;

    for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
        if(it->fValue==0 && *it!=piece)
        {
            if(piece.fY==1 && side==WHITE)
            {
                if(it->fY==1 && abs(piece.fX-it->fX)==1)
                {
                    valid=false;
                    break;
                }
            }
            if(piece.fY==6 && side==BLACK)
            {
                if(it->fY==6 && abs(piece.fX-it->fX)==1)
                {
                    valid=false;
                    break;
                }
            }
            else if((abs(it->fX-piece.fX)==1) && (abs(it->fY-piece.fY)==1))
            {
                valid=false;
                break;
            }
        }
    return valid;
}


bool Lauretta::isOpenFile(chessPiece& piece, bool side, bool semiOpen)
{
    bool open=true;
    Player* temp=(fDummy1->myTurn()==side)? fDummy1:fDummy2;
    if(semiOpen)
    {
        for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
            if(it->fValue==0 && it->fX==piece.fX)
            {
                open=false;
                break;
            }
    }
    else
    {
        Player* temp2=(fDummy1->myTurn()==side)? fDummy2:fDummy1;
        bool end_search=false;

        for(vector<chessPiece>::iterator it=temp->fPiece->begin(); it!=temp->fPiece->end(); ++it)
            if(it->fValue==0 && it->fX==piece.fX)
            {
                open=false;
                end_search=true;
                break;
            }

        if(!end_search)
            for(vector<chessPiece>::iterator it2=temp2->fPiece->begin(); it2!=temp2->fPiece->end(); ++it2)
                if(it2->fValue==0 && it2->fX==piece.fX)
                {
                    open=false;
                    break;
                }

    }
    return open;
}


void Lauretta::initializeEvaluateTables()
{
    fPawn_Table=new vector<int>(64,0);
    *fPawn_Table={
                    0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
                    10	,	10	,	0	,	-10	,	-10	,	0	,	10	,	10	,
                    5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
                    0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
                    5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
                    10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
                    20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
                    40	,	40	,	40	,	40	,	40	,	40	,	40	,	40
                };

    fKnight_Table=new vector<int>(64,0);
    *fKnight_Table={
                     0	,	-10	,	0	,	0	,	0	,	0	,	-10	,	0	,
                     0	,	0	,	0	,	5	,	5	,	0	,	0	,	0	,
                     0	,	0	,	10	,	10	,	10	,	10	,	0	,	0	,
                     0	,	0	,	10	,	20	,	20	,	10	,	5	,	0	,
                     5	,	10	,	15	,	20	,	20	,	15	,	10	,	5	,
                     5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
                     0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
                     0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
                   };

    fBishop_Table=new vector<int>(64,0);
    *fBishop_Table={
                    0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
                    0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
                    0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
                    0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
                    0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
                    0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
                    0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
                    0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
                  };

    fRook_Table=new vector<int>(64,0);
    *fRook_Table={
                    0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
                    0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
                    0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
                    0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
                    0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
                    0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
                    25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
                    0	,	0	,	5	,	10	,	10	,	5	,	0	,	0
                };

    fQueen_Table=new vector<int>(64,0);
    *fQueen_Table={
                    0	,	0	,	0	,	5	,	0	,	0	,	0	,	0	,
                    0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
                    0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
                    0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
                    0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
                    0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
                    0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
                    0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
                  };

    fKing_Table_Begin=new vector<int>(64,0);
    *fKing_Table_Begin={
                            0	,	5	,	5	,	-10	,	-10	,	0	,	10	,	5	,
                            -10	,	-10	,	-10	,	-10	,	-10	,	-10	,	-10	,	-10	,
                            -30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,
                            -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
                            -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
                            -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
                            -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
                            -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70
                           };

    fKing_Table_End=new vector<int>(64,0);
    *fKing_Table_End={
                        -50	,	-20	,	0	,	0	,	0	,	0	,	-20	,	-50	,
                        -20 	,	0	,	20	,	20	,	20	,	20	,	0	,	-20	,
                        0	,	20	,	40	,	40	,	40	,	40	,	20	,	0	,
                        0	,	20	,	40	,	50	,	50	,	40	,	20	,	0	,
                        0	,	20	,	40	,	50	,	50	,	40	,	20	,	0	,
                        0	,	20	,	40	,	40	,	40	,	40	,	20	,	0	,
                        -20	,	0	,	20	,	20	,	20	,	20	,	0	,	-20	,
                        -50	,	-20	,	0	,	0	,	0	,	0	,	-20	,	-50
};



    fMirror_Table=new vector<int>(64,0);
    *fMirror_Table={
                    56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
                    48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
                    40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
                    32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
                    24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
                    16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
                    8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
                    0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
                  };

    fPassed_Pawn_Rank=new vector<int>(8,0);
    *fPassed_Pawn_Rank={0, 5, 10, 20, 35, 60, 100, 200};

    return;
}


void Lauretta::clearEvaluateTables()
{
    delete fPawn_Table;
    delete fKnight_Table;
    delete fBishop_Table;
    delete fRook_Table;
    delete fQueen_Table;
    delete fKing_Table_Begin;
    delete fKing_Table_End;
    delete fMirror_Table;
    delete fPassed_Pawn_Rank;
    return;
}
