#include <bits/stdc++.h>
using namespace std;

/*
m     : maps the values to corresponding registers. key: registers :: value:value_of_registers
x2    : stack pointer
tokens: converts entire code from asm file to string tokens
jt    : maps the func_name to its location in the code
*/

unordered_map<string, int> m;
vector<vector<string>> tokens;
unordered_map<string, int> jt;
int mem_int[2048];
char mem_strings[2048];

// creating this function to store all the locations in the code
void jump()
{
    for (int i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].size() != 0)
        {
            if (tokens[i][0].at(tokens[i][0].length() - 1) == ':')
            {
                jt[tokens[i][0]] = i;
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
            if (tokens[i][j].compare("li") == 0)
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
            else if (tokens[i][j] == "bgt")
            {
                if (m[tokens[i][j + 1]] > m[tokens[i][j + 2]])
                {
                    string fun_name = tokens[i][j + 3];
                    fun_name += ":";
                    i = jt[fun_name];
                    j = 0;
                }
            }
            else if (tokens[i][j] == "blt")
            {
                if (m[tokens[i][j + 1]] < m[tokens[i][j + 2]])
                {
                    string fun_name = tokens[i][j + 3];
                    fun_name += ":";
                    i = jt[fun_name];
                    j = 0;
                }
            }
            // else if (tokens[i][j] == "lw")
            // {
            //     int p = (stoi(tokens[i][j + 2]) + m[tokens[i][j + 3]]);
            //     m[tokens[i][j + 1]];
            // else if (tokens[i][j] == "sw")
            // {
            // }
        }
        if (flag == true)
            break;
    }
}

// This function is for assigning the values of all the registers
void assign()
{
    for (int i = 0; i < 32; i++)
    {
        string str = "x";
        str += to_string(i);
        m[str] = 0;
    }
}

// This function will display all the values of the registers
void display_reg()
{
    for (int i = 0; i < 32; i++)
    {
        string str = "x";
        str += to_string(i);
        cout << str << " " << m[str] << endl;
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
        char *token = strtok(p, " ,()");
        while (token)
        {
            Tokenizer.push_back(token);
            token = strtok(NULL, " ,");
        }
        tokens.push_back(Tokenizer);
    }
}

// interacts with the user to perform certain task
void user_interaction()
{
    cout << "Choose from the below mentioned options:\n 1) Display all register values \n 2) Print calue of a particular register \n";
    int inp;
    cin >> inp;
    switch (inp)
    {
    case 1:
        display_reg();
        break;

    case 2:
        string reg;
        cin >> reg;
        if (reg[0] == 'x' && reg.substr(1, 2) >= "0" && reg.substr(1, 2) < "32") // check if the register is value
        {
            cout << "value stored in " << reg << " is: " << m[reg] << endl;
        }
        else
        {
            cout << "Invalid register!\n";
        }
        break;
    }
}

int main()
{

    form_tokens();
    jump();
    assign();
    recog_instr();
    user_interaction();
    return 0;
}
