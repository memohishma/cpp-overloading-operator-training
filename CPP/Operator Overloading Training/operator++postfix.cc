1.
  #include<iostream>
using namespace std;
//++ Operator
class Test
{
private:
    int x=0;
public:
   Test()
{
  cout<<"Enter the value of x : ";
  cin>>x;    //let x=15
}
  void sum()
{
   x++;
  cout<<x<<endl;
}
   void print()
{
  cout<<"x is : "<<x<<endl;
}
};
int main()
{
Test ob1; //ob1 x=15
ob1.sum(); //then x=16
ob1.print();
  
return 0;
}
//we need to find another way to write it directly
===========================================================================================================================================================================================
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
cout<<"Enter the value of x :";
cin>>x;
}
void operator ++(int)  //you have to write "int" on it if you want to use prefix or postfix
{
  x++;//postfix
}
void print()
{
  cout<<"x is : "<<x<<endl;
}
};
int main()
{
   Test ob1;     //x=15
   ob1++;
   ob1.print();  //x is : 16
  return 0;
}
===========================================================================================================================================================================================
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
cout<<"Enter the value of x :";
cin>>x;
}
void operator ++(int)  //you have to write "int" on it if you want to use prefix or postfix
{
  x*=20;//postfix
}
void print()
{
  cout<<"x is : "<<x<<endl;
}
};
int main()
{
   Test ob1;     //x=10
   ob1++;         //10*20
   ob1.print();  //x is : 200 (its just a fuction that is called operator ++,and can do any mathmatical operation on it)
  
  return 0;
}
