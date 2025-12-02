1.
#include<iostream>
using namespace std;
//Diffenet method to write constructor
class Count
{
  private:
     int value=0;
     int x=2;
  public:
     Count() : value (20) , x(2)
     {
        cout<<"Hi"<<endl;
        cout<<"The value is : "<<value<<endl;
        cout<<"X is : "<<x<<endl;
     }
};
int main()
{
    Count ob1;
    
    return 0;
}
=========================================================================================================================================
2.
#include<iostream>
using namespace std;

class Count
{
  public:
    int value=0;
  public:
    Count():value(5)
    {}
        int operator ++()
        {
            ++value;
            return value;
        }
        void display()
        {
            cout<<"value is : "<<value<<endl;
        }
    
    
};
int main()
{
    Count count1;
    Count count2;
    
    count2.value = ++count1;
    
    count1.display();//value=6
    count2.display();//value=5
    
    
    return 0;
}
=========================================================================================================================================
3.
#include<iostream>
using namespace std;

class Count
{
  public:
    int value=0;
  public:
    Count():value(5)
    {}
        void operator ++()
        {
            ++value;
        }
        void display()
        {
            cout<<"value is : "<<value<<endl;
        }
};
int main()
{
    Count count1;
    Count count2;
    
    count2.value = ++count1.value;
    
    count1.display();//value=6
    count2.display();//value=5
  
    return 0;
}
