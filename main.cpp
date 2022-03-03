#include <bits/stdc++.h>
using namespace std;

/*
m     : maps the values to corresponding registers. key: registers :: value:value_of_registers
x2    : stack pointer
tokens: converts entire code from asm file to string tokens
jt    : maps the func_name to its location in the code
*/

map<string, int> m;
char *x2;
vector<vector<string>> tokens;
unordered_map<string, int> jt;

// creating this function to store all the locations in the code
void jump()
{
    for (int i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].size() != 0)
        {
            if (tokens[i][0].at(tokens[i][0].length() - 1) == ':')
            {
                jt[tokens[i][0]] = i; // j[func_name]=i;
            }
        }
    }
}

// This function will recognize all the instruction in the code in the provided file and execute them
void recog_instr()
{
    bool flag = false;
    for (int i = 0; i < tokens.size(); i++)
    {
        for (int j = 0; j < tokens[i].size(); j++)
        {
            if (tokens[i][j].compare("li") == 0 || tokens[i][j].compare("sw") == 0 || tokens[i][j].compare("lw") == 0)
            {
                m[tokens[i][j + 1]] = stoi(tokens[i][j + 2]);
            }

            else if (tokens[i][j] == "add")
            {
                m[tokens[i][j + 1]] = m[tokens[i][j + 2]] + m[tokens[i][j + 3]];
            }

            else if (tokens[i][j] == "sub")
            {
                m[tokens[i][j + 1]] = m[tokens[i][j + 2]] - m[tokens[i][j + 3]];
            }

            else if (tokens[i][j] == "mul")
            {
                m[tokens[i][j + 1]] = m[tokens[i][j + 2]] * m[tokens[i][j + 3]];
            }

            else if (tokens[i][j] == "div")
            {
                m[tokens[i][j + 1]] = m[tokens[i][j + 2]] - m[tokens[i][j + 3]];
            }

            else if (tokens[i][j] == "addi")
            {
                m[tokens[i][j + 1]] = m[tokens[i][j + 2]] + stoi(tokens[i][j + 3]);
            }

            else if (tokens[i][j] == "bne")
            {
                if (m[tokens[i][j + 1]] != m[tokens[i][j + 2]])
                {
                    string str = tokens[i][j + 3];
                    str += ":";
                    i = jt[str];
                    j = 0;
                }
            }

            else if (tokens[i][j] == "beq")
            {
                if (m[tokens[i][j + 1]] == m[tokens[i][j + 2]])
                {
                    string str = tokens[i][j + 3];
                    str += ":";
                    i = jt[str];
                    j = 0;
                }
            }

            else if (tokens[i][j] == "jal")
            {
                string func_name = tokens[i][j + 2];
                func_name += ":";
                i = jt[func_name];
                j = 0;
            }

            else if (tokens[i][j][0] == '#')
            {
                break;
            }

            else if (tokens[i][j] == "end")
            {
                flag = true;
                break;
            }
        }
        if (flag == true)
            break;
    }
}

// This function is for assigning the values of all the registers
void assign()
{
    m["x0"] = 0;
    m["x1"] = 0;
    for (int i = 3; i < 32; i++)
    {
        string str = "x";
        str += to_string(i);
        m[str] = 0;
    }
}

// This function will display all the values of the registers
void display_reg()
{
    for (auto a : m)
    {
        cout << a.first << " " << a.second << endl;
    }
}

// This function will tokenize all the rten code and store all the tokens in the globally defined vectors
// The 2d vector is made to store the lines as rows and tokens in these lines as the columns
void form_tokens()
{
    ifstream in;
    in.open("test.asm");
    while (in.eof() == 0)
    {
        vector<string> Tokenizer;
        string s;
        getline(in, s);
        char p[s.length()];
        strcpy(p, s.c_str());
        char *token = strtok(p, " ,");
        while (token)
        {
            Tokenizer.push_back(token);
            token = strtok(NULL, " ,");
        }
        tokens.push_back(Tokenizer);
    }
}

int main()
{

    form_tokens();
    jump();
    assign();
    recog_instr();
    display_reg();
    return 0;
}