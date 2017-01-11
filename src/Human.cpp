#include "GuiDisplay.hpp"
#include "Human.hpp"
using namespace std;

Human::Human(int color, string name, GuiDisplay* display):Player(color,name),fDisplay(display){}

Human::Human(const Human& source):Player(source){}


Human::~Human(){}


string Human::requestCommand()
{
    string command;
#ifdef DEBUG
    while(!checkCommand(command))
        getline(cin,command);
    if(command.size()==4)
    {
        for(int i=0; i<4; ++i)
            if(isalpha(command.at(i)))
                if(isupper(command.at(i)))
                    command.at(i)=tolower(command.at(i));
    }
#else
    command=fDisplay->requestCommand();
#endif
    fLast_Command=command;
    return command;
}


int Human::requestPromotion()
{
    int promote=-1;
#ifdef DEBUG
    char handle;
    cout<<"which Piece should this pawn be promoted to?\n\n";
    cout<<"1.\tKnight\n";
    cout<<"2.\tBishop\n";
    cout<<"3.\tRook\n";
    cout<<"4.\tQueen\n";
    cin>>handle;
    while(!isdigit(handle) || (handle-49)>4 || (handle-49)<1)
        cin>>handle;
    promote=handle-49;
#else
    promote=fDisplay->requestPromotion();
#endif
    return promote;
}


bool Human::checkCommand(string& command)
{
    bool valid=false;
#ifdef DEBUG
    if(command.size()==4)
    {
        if(isalpha(command.at(0)) && isdigit(command.at(1)))
        {
            if((toupper(command.at(0))<='H' && toupper(command.at(0))>='A') && toupper(command.at(2))<='H' &&
                    toupper(command.at(2))>='A')
                if(isdigit(command.at(1)) && isdigit(command.at(3)))
                    if((command.at(1)<='8' && command.at(3)>='1') && (command.at(1)<='8' && command.at(3)>='1'))
                        valid=true;
        }
        else if((command.size()==4) && toupper(command.at(0))=='Q' && toupper(command.at(1))=='U' && toupper(command.at(2))=='I'
                            && toupper(command.at(3))=='T')
                                valid=true;
    }
#else
    chessPiece from((int)command.at(0)-'a',(int)command.at(1)-49,-1);
    for(vector<chessPiece>::iterator it=fPiece->begin(); it!=fPiece->end(); ++it)
        if(*it==from)
        {
            valid=true;
            break;
        }
#endif
    return valid;
}
