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
=========================================================================================================================================
4.
#include<iostream>
using namespace std;
class Count
{
  private:
   int value;
   
  public:
    Count()
    {
        value=5;
    }
    void display()
    {
        cout<<"value is : "<<value<<endl;
    }
    
    Count operator ++()//count2 = ++count1;
    {
        Count ob;
        ob.value = ++value;
        return ob;
    }
};
int main()
{
    Count count1;
    Count count2;
    count2 = ++count1;
    count1.display();
    count2.display();
    return 0;
}
========================================================================================================================================
5.
#include<iostream>
using namespace std;
//Count : with three objects
//result = count1 + count count2//
class Count 
{
  private:
    int value;
  public:
    Count() : value(5){}
    //result = count1 + count2
    Count operator +(Count ob)
    {
        Count result;
        result.value = value + ob.value;
        return result;
    }
    void print()//count1.print
    {
        cout<<"value is : "<<value<<endl;
    }
};
int main()
{
    Count count1;
    Count count2;
    Count result;
    result = count1 + count2;
    result.print();//x=10
    return 0;
}
