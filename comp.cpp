#include <vector>
#include <string>
#include <iostream>

using namespace std;

class Solution { 
private:
public:
    int compress(vector<char>& chars) {
        int size = chars.size();
        string str; 
        int count = 1;
        for (int i = 0; i < size ; i++) {
            char c = chars[i];
            while (chars[i] == chars[i + 1] && i < size - 1) {
                count++;
                i++;
            }

            str.push_back(c);
            if (count > 1) {
                str.push_back(count + '0');
            }

            count = 1;
        }

        return str.length();
    }
};

