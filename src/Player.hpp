#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>
#include <vector>
#include "defs.hpp"


class chessPiece
{
    public:
        chessPiece():fX(0),fY(0),fValue(0){}

        chessPiece(int x, int y,int val=-1):fX(x),fY(y),fValue(val){}

        chessPiece(const chessPiece& pos):fX(pos.fX),fY(pos.fY),fValue(pos.fValue){}

        chessPiece(chessPiece&& pos):fX(std::move(pos.fX)),fY(std::move(pos.fY)),fValue(std::move(pos.fValue)){}

        ~chessPiece(){fX=0; fY=0; fValue=0;}

        chessPiece& operator=(const chessPiece& second)
        {
            (*this).fX=second.fX;
            (*this).fY=second.fY;
            (*this).fValue=second.fValue;
            return *this;
        }

        chessPiece& operator=(chessPiece&& second)
        {
            (*this).fX=std::move(second.fX);
            (*this).fY=std::move(second.fY);
            (*this).fValue=std::move(second.fValue);
            return *this;
        }

        bool operator==(const chessPiece& second)
        {
            return ((second.fY==(*this).fY) && (second.fX==(*this).fX));
        }

        bool operator!=(const chessPiece& second)
        {
            return !((second.fY==(*this).fY) && (second.fX==(*this).fX));
        }
        int fX,fY,fValue;
};


struct enPassant
{
    enPassant():valid(false),ply(-1){}
    enPassant(const enPassant& source):piece(source.piece),valid(source.valid),ply(source.ply){}
    chessPiece piece;
    bool valid;
    int ply;
};


class Player
{
    public:
#ifdef DEBUG
        int NODES=0;
#endif
        std::vector<chessPiece>* fPiece;

        Player(int,std::string);
        Player(const Player&);
        virtual ~Player();

        chessPiece* king();
        inline int myTurn(){return fMy_Turn;}
        virtual std::string requestCommand(){return "";}
        virtual inline int requestPromotion(){return -1;}
        virtual inline void setOpponent(Player*){}
        virtual inline void setDifficulty(int){}
        virtual bool checkCommand(std::string&){return false;}
        inline std::string myName(){return fName;}
        inline bool isKingChecked(){return fKing_Checked;}
        inline void setKingChecked(bool check){fKing_Checked=check;}
        inline std::string lastValidCommand(){return fLast_Command;}
        inline bool hasKingMoved(){return fKing_Moved;}
        inline void setKingMoved(bool ans){fKing_Moved=ans;}
        inline bool hasLeftRookMoved(){return fLeft_Rook_Moved;}
        inline bool hasRightRookMoved(){return fRight_Rook_Moved;}
        inline void setLeftRookMoved(bool ans){fLeft_Rook_Moved=ans;}
        inline void setRightRookMoved(bool ans){fRight_Rook_Moved=ans;}
        inline enPassant getEnpassant(){return fEnPassant;}
        inline void setEnpassant(bool yes,int ply,chessPiece piece=chessPiece(-1,-1)){fEnPassant.valid=yes;
                                                                                      fEnPassant.piece=piece;
                                                                                      fEnPassant.ply=ply;
                                                                                     }

    private:
        Player(Player&&)=delete;
        Player& operator=(const Player&)=delete;
        Player& operator=(Player&&)=delete;

        void generatePiece();

    protected:

        std::string fLast_Command;
        chessPiece* fKing;
        std::string fName;
        enPassant fEnPassant;
        int fMy_Turn;
        bool fKing_Checked;
        bool fKing_Moved;
        bool fLeft_Rook_Moved;
        bool fRight_Rook_Moved;
};

#endif //PLAYER_HPP
