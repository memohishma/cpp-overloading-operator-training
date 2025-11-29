1.
  #include<iostream>
using namespace std;
class Test 
{
    private:
    int x=0;
    public:
    Test()
    {
        cout<<"enter x :";
        cin>>x;
    }
    void operator ++(int)//postfix
    {
        x++;
    }
    void operator ++()//prefix
    {
        x++;
    }
    void print()
    {
        cout<<"x is "<<x<<endl;
    }
};
int main()
{
    Test ob1;//10
    ob1++;//postfix x=11
    ++ob1;//prefix x=12
    ob1.print();
    
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
        cout<<"enter x :";
        cin>>x;
    }
    void operator ++()//prefix
    {
        x++;
    }
    void print()
    {
        cout<<"x is "<<x<<endl;
    }
};
int main()
{
    Test ob1;//10
    ++ob1;//prefix x=11
    ob1.print();
    
    return 0;
}
=========================================================================================================================================
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
        cout<<"enter x :";
        cin>>x;
    }
  /*    void operator ++(int)//postfix : ob1++    */
    void operator ++()//prefix : ++ob1
    {
        x--;
    }
    void print()
    {
        cout<<"x is "<<x<<endl;
    }
};
int main()
{
    Test ob1;//10
    ++ob1;
    ob1.print();
    
    return 0;
}
========================================================================================================================================
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
          cout<<"enter x :";
          cin>>x;
      }
      void operator --(int)//postfix ob1--
      {
          x--;
      }
      void operator --()
      {
          --x;
      }
      void print()
      {
       cout<<"x is : "<<x<<endl;   
      }
};
int main()
{
    Test ob1; //x=12
    ob1--;    //x=11
    --ob1;    //x=10
    ob1.print();
    
    return 0;
}
========================================================================================================================================
