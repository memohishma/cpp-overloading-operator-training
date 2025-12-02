#include<iostream>
using namespace std;

int main()
{
  cout<<"***********Calculator***********\n\n";
  char op;
  float num1,num2;
  cout<<"Enter the first number: ";
  cin>>num1;
  cout<<"Enter operator (+,-,*,/): ";
  cin>>op;
  cout<<"Enter the second number: ";
  cin>>num2;
  
  switch(op)
  {
   case'+':cout<<num1<<"+"<<num2<<"="<<num1+num2<<endl;
   break;
   case'-':cout<<num1<<"-"<<num2<<"="<<num1-num2<<endl;
   break;
   case'*':cout<<num1<<"*"<<num2<<"="<<num1*num2<<endl;
   break;
   case'/':
   {
       if(num2==0)
       cout<<"Error";
        cout<<num1<<"/"<<num2<<"="<<num1/num2<<endl;
   }
   break;
   default:
   cout<<"Error! Not Correct\n";
   }
   cout<<"\n************\n";
   
    return 0;
}
