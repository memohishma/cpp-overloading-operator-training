1.

#include<iostream>
using namespace std;
//(*=,/=,+=,-=,%=)
int main()
{
    int x=3;
    int y=4;
    int z=5;
    int c=3;
    x*=2;
    cout<<x<<endl;//6
    y+=2;
    cout<<y<<endl;//6
    z%=10;
    cout<<z<<endl;//5
    c/=1;
    cout<<c<<endl;//3
    
    return 0;
}
==========================================================================================================================================
2.
#include<iostream>
using namespace std;
class Test
{
    private: 
      int x=0;
    public:
    Test()
    {
        cout<<"enter the value of x : ";
        cin>>x;
    }
    void Sum()
    {
        x+=5;
    }
    void print()
    {
     cout<<"The value of x is : "<<x<<endl;   
    }
};
int main()
{
    Test ob1;
    ob1.Sum();
    ob1.print();
    
    
    return 0;
}
========================================================================================================================================
3.
#include<iostream>
using namespace std;
class Test
{
    private: 
      int x=0;
    public:
    Test()
    {
        cout<<"enter the value of x : ";
        cin>>x;
    }
    void print()
    {
     cout<<"The value of x is : "<<x<<endl;   
    }
    void operator +=(int)
    {
        x+=12;
    }
};
int main()
{
    Test ob1;//x=10
    ob1 += 12;
    ob1.print();//x=x+5=10+5=15
    
    return 0;
}
=========================================================================================================================================
4.
#include<iostream>
using namespace std;
class Test
{
    private: 
      int x=0;
    public:
    Test()
    {
        cout<<"enter the value of x : ";
        cin>>x;
    }
    void print()
    {
     cout<<"The value of x is : "<<x<<endl;   
    }
    void operator +=(int y)
    {
        x+=y;
    }
};
int main()
{
    Test ob1;//x=12
    ob1 += 12;
    ob1.print();//x=x+y=10+12=22
    
    return 0;
}
=======================================================================================================================================
5.
#include<iostream>
using namespace std;
class Test
{
    private: 
      int x=0;
    public:
    Test()
    {
        cout<<"enter the value of x : ";
        cin>>x;
    }
    void print()
    {
     cout<<"The value of x is : "<<x<<endl;   
    }
    void operator +=(int y)
    {
        x+=y;
    }
    void operator -=(int y)
    {
        x-=y;
    }
};
int main()
{
    Test ob1;//10
    ob1 += 10;
    ob1.print();//20
    ob1-= 12;
    ob1.print();//8
    
    
    return 0;
}
========================================================================================================================================
6.
#include<iostream>
using namespace std;
class Count
{
    private: 
      int value=0;
      int x=0;
    public:
    Count() : value (20) , x(2)
    {
        cout<<"Hello"<<endl;
        cout<<"the value of x is :"<<x<<endl;
        cout<<"the value is "<<value<<endl;
    }
    
};
int main()
{
   Count ob1;
   //will print hello,2,20
   
    return 0;
}
