#ifndef CHESSBOARD_HPP
#define CHESSBOARD_HPP

#include <QObject>
#include <vector>
#include <string>
#include "Lauretta.hpp"
#include "Human.hpp"
#include "defs.hpp"

class GuiDisplay;

struct Position
{
    Position(bool col,int val):color(col),value(val){}
    bool color;
    int value;
};


class chessBoard : public QObject
{
    Q_OBJECT

    public:
        chessBoard(GuiDisplay*);
        ~chessBoard();
        void initialize();
        void cleanUp();
        inline bool whoseTurn();
        inline Player* player();
        inline bool isPlayerInCheck();
        inline void setIsPlayerInCheck(bool);

signals:
        void played();

public slots:
        void moderator();

    private:

        chessBoard(const chessBoard&)=delete;
        chessBoard(chessBoard&&)=delete;
        chessBoard& operator=(const chessBoard&)=delete;
        chessBoard& operator=(const chessBoard&&)=delete;

        void initializePlayers();   //create players
        void initializeBoard();
        void initializeGuiDisplay();
        void initializeSession();

        void updateBoard();
        void updateScreen();
        int requestColor();
        std::string requestName(int);
        std::vector<chessPiece> convertCommandToDigital(std::string);

        bool movePiece(chessPiece,chessPiece,bool dummy=false,bool castling=false);
        bool isMoveValid(chessPiece&,chessPiece&,bool real=true);
        bool isPieceValid(chessPiece&);

        bool isMoveDiagonal(chessPiece&,chessPiece&, int king=0, bool real=true);
        bool isMoveStraight(chessPiece&,chessPiece&,int pawn=0,bool real=true);
        bool isMoveSideways(chessPiece&,chessPiece&, int king=0, bool real=true);
        bool isMoveKnight(chessPiece&,chessPiece&);

        bool isStalemate();
        bool isKingExposed();
        bool isKingChecked();
        bool canProtectKing(chessPiece&);
        bool isCastling(chessPiece&,bool);
        bool isPieceCaptured(chessPiece);
        bool isKingCheckMated(chessPiece&);
        bool hasKingValidMove(bool check=true);
        bool isEnpassant(chessPiece&,chessPiece&,bool);

        int fTurn;
        int fPly_Count;
        int fFifty_Moves;
        bool fSession;

        Player* fPlayer1;
        Player* fPlayer2;

        std::vector<std::string>* fMove_History;
        std::vector<chessPiece>* fCaptured_Piece_Player1;
        std::vector<chessPiece>* fCaptured_Piece_Player2;

        std::vector<std::vector<Position>*>* fBoard; //-1 for empty space

#ifndef DEBUG
        bool fCheck;
        bool fStatus;
        GuiDisplay* fScreen;
#endif
};


inline bool chessBoard::whoseTurn()
{
    return fTurn;
}


inline Player* chessBoard::player()
{
    return (fPlayer1->myTurn()==fTurn)? fPlayer1:fPlayer2;
}


inline bool chessBoard::isPlayerInCheck()
{
    return fCheck;
}


inline void chessBoard::setIsPlayerInCheck(bool check)
{
    fCheck=check;
}

#endif //CHESSBOARD_HPP
