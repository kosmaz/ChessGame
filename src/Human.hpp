#ifndef HUMAN_HPP
#define HUMAN_HPP

#include "Player.hpp"
#include "defs.hpp"

class GuiDisplay;

class Human : public Player
{
    public:
        Human(int,std::string,GuiDisplay*);
        Human(const Human&);
        ~Human();
        std::string requestCommand();
        inline int requestPromotion();
        inline void setOpponent(Player*){}
        inline void setDifficulty(int){}
        inline void setName(std::string str){fName=str;}
        bool checkCommand(std::string&);

    private:
        Human(Human&&)=delete;
        Human& operator=(const Human&)=delete;
        Human& operator=(const Human&&)=delete;

        GuiDisplay* fDisplay;
};

#endif //HUMAN_HPP
