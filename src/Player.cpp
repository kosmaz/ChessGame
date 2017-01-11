#include "Player.hpp"
using namespace std;


Player::Player(int color,string name):fPiece(nullptr),fLast_Command(""),fKing(nullptr),fName(name),fMy_Turn(color),
                                      fKing_Checked(false),fKing_Moved(false),fLeft_Rook_Moved(false),fRight_Rook_Moved(false)
{
    if(color==0)
        fKing=new chessPiece(5,0,5);
    else
        fKing=new chessPiece(5,7,5);
    generatePiece();
}


Player::Player(const Player& source):fPiece(new vector<chessPiece>(*source.fPiece)),fLast_Command(source.fLast_Command),
    fKing(new chessPiece(*source.fKing)),fName(source.fName),fEnPassant(source.fEnPassant),fMy_Turn(source.fMy_Turn),
    fKing_Checked(source.fKing_Checked),fKing_Moved(source.fKing_Moved),fLeft_Rook_Moved(source.fLeft_Rook_Moved),
    fRight_Rook_Moved(source.fRight_Rook_Moved){}


Player::~Player()
{
    delete fPiece;
    delete fKing;
}


chessPiece* Player::king()
{
    for(vector<chessPiece>::iterator it=fPiece->begin(); it!=fPiece->end(); ++it)
        if((*it).fValue==5)
        {
            *fKing=*it;
            break;
        }
    return fKing;
}


void Player::generatePiece()
{
    fPiece=new vector<chessPiece>;

    for(int j=0; j<8; ++j)
        if(fMy_Turn==0)
            fPiece->push_back(chessPiece(j,1,0));

        else
            fPiece->push_back(chessPiece(j,6,0));

    int k=0;
    if(fMy_Turn==0)
    {
        fPiece->push_back(chessPiece(k+0,0,3));
        fPiece->push_back(chessPiece(k+1,0,1));
        fPiece->push_back(chessPiece(k+2,0,2));
    }

    else
    {
        fPiece->push_back(chessPiece(k+0,7,3));
        fPiece->push_back(chessPiece(k+1,7,1));
        fPiece->push_back(chessPiece(k+2,7,2));
    }

    k=5;
    if(fMy_Turn==0)
    {
        fPiece->push_back(chessPiece(k+0,0,2));
        fPiece->push_back(chessPiece(k+1,0,1));
        fPiece->push_back(chessPiece(k+2,0,3));
    }

    else
    {
        fPiece->push_back(chessPiece(k+0,7,2));
        fPiece->push_back(chessPiece(k+1,7,1));
        fPiece->push_back(chessPiece(k+2,7,3));
    }

    if(fMy_Turn==0)
    {
        fPiece->push_back(chessPiece(3,0,4));
        fPiece->push_back(chessPiece(4,0,5));
    }
    else
    {
        fPiece->push_back(chessPiece(3,7,4));
        fPiece->push_back(chessPiece(4,7,5));
    }
    return;
}
