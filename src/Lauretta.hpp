//Lauretta is the name of the A.I in use for the algorithm generation
#ifndef LAURETTA_HPP
#define LAURETTA_HPP

#include "Player.hpp"
#include "Human.hpp"
#include "defs.hpp"


struct Move
{
    Move():ply(0),piece(-1),victim(-1),score(0),promotedTo(-1),side(-1),capture(false),iscastling(false),isKingChecked(false),
        isPawnPromoted(false),isEnpassantCapture(false){}

    Move(const Move& source):from(source.from),to(source.to),ply(source.ply),piece(source.piece),victim(source.victim),
        score(source.score),promotedTo(source.promotedTo),side(source.side),capture(source.capture),
        iscastling(source.iscastling),isKingChecked(source.isKingChecked),isPawnPromoted(source.isPawnPromoted),
        isEnpassantCapture(source.isEnpassantCapture),enpassantPiece(source.enpassantPiece){}

    Move(Move&& source):from(std::move(source.from)),to(std::move(source.to)),ply(std::move(source.ply)),
        piece(std::move(source.piece)),victim(std::move(source.victim)),score(std::move(source.score)),
        promotedTo(std::move(source.promotedTo)),side(std::move(source.side)),capture(std::move(source.capture)),
        iscastling(std::move(source.iscastling)),isKingChecked(std::move(source.isKingChecked)),
        isPawnPromoted(std::move(isPawnPromoted)),isEnpassantCapture(std::move(source.isEnpassantCapture)),
        enpassantPiece(source.enpassantPiece){}

    Move& operator=(const Move& source)
    {
        this->from=source.from;
        this->to=source.to;
        this->ply=source.ply;
        this->piece=source.piece;
        this->victim=source.victim;
        this->score=source.score;
        this->promotedTo=source.promotedTo;
        this->side=source.side;
        this->capture=source.capture;
        this->iscastling=source.iscastling;
        this->isKingChecked=source.isKingChecked;
        this->isPawnPromoted=source.isPawnPromoted;
        this->isEnpassantCapture=source.isEnpassantCapture;
        this->enpassantPiece=source.enpassantPiece;
        return *this;
    }

    Move& operator=(Move&& source)
    {
        this->from=std::move(source.from);
        this->to=std::move(source.to);
        this->ply=std::move(source.ply);
        this->piece=std::move(source.piece);
        this->victim=std::move(source.victim);
        this->score=std::move(source.score);
        this->promotedTo=std::move(source.promotedTo);
        this->side=std::move(source.side);
        this->capture=std::move(source.capture);
        this->iscastling=std::move(source.iscastling);
        this->isKingChecked=std::move(source.isKingChecked);
        this->isPawnPromoted=std::move(source.isPawnPromoted);
        this->isEnpassantCapture=std::move(source.isEnpassantCapture);
        this->enpassantPiece=std::move(source.enpassantPiece);
        return *this;
    }

    bool operator==(const Move& source)
    {
        if(this->from==source.from)
            if(this->to==source.to)
                if(this->ply==source.ply)
                    if(this->piece==source.piece)
                        if(this->promotedTo==source.promotedTo)
                            if(this->side==source.side)
                                if(this->capture==source.capture)
                                    if(this->iscastling==source.iscastling)
                                        if(this->isKingChecked==source.isKingChecked)
                                            if(this->isPawnPromoted==source.isPawnPromoted)
                                                if(this->isEnpassantCapture==source.isEnpassantCapture)
                                                    return true;
        return false;
    }


    chessPiece from;
    chessPiece to;
    int ply;
    int piece;
    int victim;
    int score;
    int promotedTo;
    bool side;
    bool capture;
    bool iscastling;
    bool isKingChecked;
    bool isPawnPromoted;    
    bool isEnpassantCapture;
    chessPiece enpassantPiece;
};

class Lauretta : public Player
{
    public:
        Lauretta(int,Player*,std::string name="Lauretta");
        Lauretta(int,std::string name="Lauretta");
        Lauretta(const Lauretta&);
        ~Lauretta();
        std::string requestCommand();
        inline int requestPromotion();
        inline void setOpponent(Player*);
        inline void setDifficulty(int);
        virtual bool checkCommand(std::string&){return false;}

    private:
        Lauretta(Lauretta&&)=delete;
        Lauretta& operator=(const Lauretta&)=delete;
        Lauretta& operator=(const Lauretta&&)=delete;

        std::string thinkAndPlay();
        std::string convertDigitalToCommand(const chessPiece&,const chessPiece&);

        bool dummyMovePiece(chessPiece&,chessPiece);
        bool isMoveValid(chessPiece&,chessPiece&);
        bool isPieceValid(chessPiece&);

        bool isMoveDiagonal(chessPiece&,chessPiece&,int king=0);
        bool isMoveStraight(chessPiece&,chessPiece&,int pawn=0);
        bool isMoveSideways(chessPiece &, chessPiece &, int king=0);
        bool isMoveKnight(chessPiece&,chessPiece&);

        bool isKingExposed();
        bool isKingChecked();
        bool isCastling(chessPiece&);
        bool canProtectKing(chessPiece&);
        bool hasKingValidMove(bool check=true);
        bool isKingCheckMated(chessPiece&);
        bool isEnpassant(chessPiece&,chessPiece&);

        void makeMove(Move&);
        void takeMove();
        void pickNextMove(std::vector<Move>::iterator,std::vector<Move>*);
        void removePiece(chessPiece&,Player*);
        void restorePiece(Player*);

        std::vector<Move>* generateMoves(bool,bool capture=false);
        int alphaBeta(int,int,int,bool);
        int quiescenceSearch(int, int, bool);
        int scoreFromSidePOV(bool);
        int material(bool);
        int mobility(bool);
        void countAllPiece();
        bool isMaterialDraw();
        int endGameMaterial();

        bool isBishopPair(bool);
        bool isDoublePawn(chessPiece&,bool);
        bool isPassedPawn(chessPiece&,bool);
        bool isIsolatedPawn(chessPiece&,bool);
        bool isOpenFile(chessPiece&,bool,bool);

        inline int moveScore(int,int);
        inline int indexSquare(const chessPiece&);

        void initializeEvaluateTables();
        inline void initializeKillerMoves();
        inline void initializeHistoryMoves();
        void clearEvaluateTables();
        inline void clearKillerMoves();
        inline void clearHistoryMoves();

        std::vector<Move>* fMoves_Made;
        Move fBest_Move;
        Player* fOpponent;

        Player* fDummy1;
        Player* fDummy2;
        std::vector<chessPiece>* fDummy1_Captured_Pieces;
        std::vector<chessPiece>* fDummy2_Captured_Pieces;
        std::vector<std::vector<Move>*>* fKiller_Moves;
        std::vector<std::vector<int>*>* fHistory_Moves;
        bool fDummy_Turn;
        bool fIsEnpassant;
        bool fIsCastling;
        bool fPawn_Promotion;
        int fDummy_Ply_Count;
        int fMax_Depth;

        std::vector<int>* fPawn_Table;
        std::vector<int>* fKnight_Table;
        std::vector<int>* fBishop_Table;
        std::vector<int>* fRook_Table;
        std::vector<int>* fQueen_Table;
        std::vector<int>* fKing_Table_Begin;
        std::vector<int>* fKing_Table_End;
        std::vector<int>* fMirror_Table;
        std::vector<int>* fWhite_Piece_Count;
        std::vector<int>* fBlack_Piece_Count;
        std::vector<int>* fPassed_Pawn_Rank;
};


inline void Lauretta::setOpponent(Player* opponent)
{
    fOpponent=opponent;
    return;
}


inline void Lauretta::setDifficulty(int max_depth)
{
    fMax_Depth=max_depth;
    return;
}

inline int Lauretta::requestPromotion()
{
    return fBest_Move.promotedTo;
}


inline int Lauretta::moveScore(int attack, int victim)
{
    int attackers[]={12,10,8,6,4,2};
    int victims[]={2,4,6,8,10};
    return (attackers[attack]*victims[victim])+1000000;
}


inline int Lauretta::indexSquare(const chessPiece& square)
{
    return (square.fY*8)+square.fX;
}


inline void Lauretta::initializeKillerMoves()
{
    Move move;
    fKiller_Moves=new std::vector<std::vector<Move>*>;
    fKiller_Moves->push_back(new std::vector<Move>(fMax_Depth,move));
    fKiller_Moves->push_back(new std::vector<Move>(fMax_Depth,move));
    return;
}


inline void Lauretta::initializeHistoryMoves()
{
    fHistory_Moves=new std::vector<std::vector<int>*>;
    for(int i=0; i<64; ++i)
        fHistory_Moves->push_back(new std::vector<int>(64,0));
    return;
}


inline void Lauretta::clearKillerMoves()
{
    delete fKiller_Moves->at(1);
    delete fKiller_Moves->at(0);
    delete fKiller_Moves;
    return;
}

inline void Lauretta::clearHistoryMoves()
{
    for(int i=0; i<64; ++i)
        delete fHistory_Moves->at(i);
    delete fHistory_Moves;
    return;
}

#endif //LAURETTA_HPP
