#include <iostream>
using namespace std;

int main()
{
    int n;
    int m;
    cout << "Enter a number:";
    cin >> n;
    if (n < 0)
    {
        cout << "Negative number!";
    }
    else if (n == 0)
    {
        cout << "Zero!";
    }
    else
    {
        cout << "Positive number!";
    }

    if (n % 2 == 0)
    {
        cout << " Even!";
    }
    else
    {
        cout << " Odd!";
    }

    if (n > 100)
    {
        cout << " Large!";
    }
    else
    {
        cout << " Small or medium!";
    }

    return 0;
}