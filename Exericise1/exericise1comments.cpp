#include <iostream>
#include <string>

using namespace std;

int main() {
    // Declare variables
    string userName;
    int userAge;

    // Get user input
    cout << "Enter your name: ";
    getline(cin, userName);
    cout << "Enter your age: ";
    cin >> userAge;

    // Display user information
    cout << "Hello, " << userName << "! You are " << userAge << " years old." << endl;

    return 0;
}