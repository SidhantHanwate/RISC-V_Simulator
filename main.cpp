#include <bits/stdc++.h>
using namespace std;

/*
GLOBAL VARIABLES DESCRIPTION

m     : maps the values to corresponding registers. key: registers :: value:value_of_registers
tokens: converts entire code from asm file to string tokens
jt    : maps the func_name to its location in the code
checks: stores lines with opcodes; these instructions contribute to cycle count in pipelining

f_staller : 2D vector which stores stages of pipeling for the case without forwarding 
wf_staller: 2D vector which stores stages of pipeling for the case with forwarding 
f_steps   : 1D vector for storing individual rows of stages(without forwarding)
wf_steps  : 1D vector for storing individual rows of stages(with forwarding)
*/

unordered_map<string, int> m;
vector<vector<string>> tokens;
unordered_map<string, int> jt;
vector<vector<string>> checks;

int Mem[4096];
map<string, int> last;

vector<vector<string>> f_staller;
vector<string> f_steps;

vector<vector<string>> wf_staller;
vector<string> wf_steps;

bool branch = false;
bool wbranch = false;

void reg_name_check(string reg, int i)
{
    if (!(reg[0] == 'x' && stoi(reg.substr(1, 2)) >= 0 && stoi(reg.substr(1, 2)) < 32)) // if any register name is invalid, throw error
    {
      cout << "line "<<i+1<<": "<<reg<<" is an invalid register name!\n\n";
      exit(1);
    }
}
void syntax_error_checker()
{

    /*
    0-based
    instr_type=1 -- to check reg at column 1
    instr_type=2 -- to check reg at column 1, 2
    instr_type=3 -- to check reg at column 1, 2, 3
    instr_type=4 -- to check reg at column 1, 3
    */
    
    for (int i = 0; i < tokens.size(); i++)
    {
        if(tokens[i].size()!=0)
        {
            int instr_type=0;
            if (tokens[i][0] == "lw"||tokens[i][0]=="sw")
            {
                instr_type=4;
            }
            else if (tokens[i][0] == "li")
            {
                instr_type=1;
            }
            else if (tokens[i][0] == "add"||tokens[i][0] == "sub"||tokens[i][0] == "mul"||tokens[i][0] == "div")
            {
                instr_type=3;
            }
            else if (tokens[i][0] == "addi"||tokens[i][0] == "bne"||tokens[i][0] == "beq"||tokens[i][0] == "bgt"||tokens[i][0] == "blt")
            {
                instr_type=2;
            }
            else if (tokens[i][0] == "jal")
            {
                //??
            }
            if(instr_type!=4)
            {
                int j=1;
                while(instr_type--)
                {
                    reg_name_check(tokens[i][j], i);
                    j++;
                }
            }
            else if(instr_type==4)
            {
                reg_name_check(tokens[i][1], i);
                reg_name_check(tokens[i][3], i);
            }
        }
    }

}
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

//checks if previous row in f_staller had a stall at same column 
void stChecker(int i, int k)
{
    for (int j = k; j < f_staller[i - 1].size(); j++)
    {
        if (f_staller[i - 1][j].compare("ST") == 0)
        {
            f_steps.push_back("ST");
        }
        else
            break;
    }
}

//checks if previous row in wf_staller had a stall at same column 
void wstChecker(int i, int k)
{
    for (int j = k; j < wf_staller[i - 1].size(); j++)
    {
        if (wf_staller[i - 1][j].compare("ST") == 0)
        {
            wf_steps.push_back("ST");
        }
        else
            break;
    }
}

//creates wf_staller(described as above)
void with_forwarding(int i)
{
    int c, ct_stall = 0, ct_stall_max = 0;
    int space = i;
    string sp = "  ";
    if (i == 0)
    {
        wf_steps.push_back("IF");
        wf_steps.push_back("ID");
        wf_steps.push_back("EX");
        wf_steps.push_back("ME");
        wf_steps.push_back("WB");
        wf_staller.push_back(wf_steps);
        return;
    }
    wf_steps.clear();
    while (space--)//next line begins with an added space at beginning
    {
        wf_steps.push_back(sp);
    }
    for (int j = wf_steps.size(); j < wf_staller[i - 1].size(); j++)
    {
        if (wf_staller[i - 1][j].compare("ST") == 0 || wf_staller[i - 1][j].compare(sp) == 0 || wf_staller[i - 1][j].compare("IF") == 0)
        {
            wf_steps.push_back(sp);
        }
        else
            break;
    }
    // call wstchecker 'before' each stage
    if (wbranch == true)
    {
        wf_steps.push_back(sp);
        wbranch = false;
    }

    wf_steps.push_back("IF");

    if (checks[i][0] == "blt" || checks[i][0] == "bgt" || checks[i][0] == "beq" || checks[i][0] == "bne")
    {
        wbranch = true;
        c = 1;
        while (c < checks[i].size() - 1)
        {
            int k = wf_steps.size() - 1;
            if (checks[i - 1][1] == checks[i][c]) // if last line has the current token as the destination register
            {
                while (k < wf_staller[i - 1].size() && wf_staller[i - 1][k++] != "EX")//wait till execution stage(EX)
                {
                    ct_stall++;
                }
            }
            else if (i - 2 >= 0 && checks[i - 2][1] == checks[i][c]) // if second last line has the current token as the destination register
            {
                while (k < wf_staller[i - 2].size() && wf_staller[i - 2][k++] != "EX")//wait till execution stage(EX)
                {
                    ct_stall++;
                }
            }
            ++c;
            ct_stall_max = max(ct_stall_max, ct_stall);
            ct_stall = 0;
        }
        while (ct_stall_max--)
            wf_steps.push_back("ST");
    }

    wstChecker(i, wf_steps.size());
    wf_steps.push_back("ID");
    if (checks[i - 1][0] == "lw")
    {
        wf_steps.push_back("ST");
    }
    wstChecker(i, wf_steps.size());
    wf_steps.push_back("EX");
    wstChecker(i, wf_steps.size());
    wf_steps.push_back("ME");
    wstChecker(i, wf_steps.size());
    wf_steps.push_back("WB");
    wf_staller.push_back(wf_steps);
    wf_steps.clear();
}

//creates f_staller(described as above)
void without_forwarding(int i)
{
    int c, ct_stall = 0, ct_stall_max = 0;
    int space = i;
    string sp = "  ";
    if (i == 0)
    {
        f_steps.push_back("IF");
        f_steps.push_back("ID");
        f_steps.push_back("EX");
        f_steps.push_back("ME");
        f_steps.push_back("WB");
        f_staller.push_back(f_steps);
        return;
    }
    f_steps.clear();

    while (space--) //each line begins with an added space at the beginning
    {
        f_steps.push_back(sp);
    }

    for (int j = f_steps.size(); j < f_staller[i - 1].size(); j++)
    {
        if (f_staller[i - 1][j].compare("ST") == 0 || f_staller[i - 1][j].compare(sp) == 0 || f_staller[i - 1][j].compare("IF") == 0)
        {
            f_steps.push_back(sp);
        }
        else
            break;
    }

    // call stchecker 'before' each stage
    if (branch == true)
    {
        f_steps.push_back(sp);
        branch = false;
    }
    f_steps.push_back("IF");

    // for branch instructions
    if (checks[i][0] == "blt" || checks[i][0] == "bgt" || checks[i][0] == "beq" || checks[i][0] == "bne")
    {
        branch = true;
        c = 1;
        while (c < checks[i].size() - 1)
        {
            int k = f_steps.size() - 1;
            if (checks[i - 1][1] == checks[i][c]) // if last line has the current token as the destination register
            {
                while (k < f_staller[i - 1].size() && f_staller[i - 1][k++] != "WB")//wait till write back stage(WB) is completed
                {
                    ct_stall++;
                }
            }
            else if (i - 2 >= 0 && checks[i - 2][1] == checks[i][c]) // if second last line has the current token as the destination register
            {
                while (k < f_staller[i - 2].size() && f_staller[i - 2][k++] != "WB")//wait till write back stage(WB) is completed
                {
                    ct_stall++;
                }
            }
            ++c;
            ct_stall_max = max(ct_stall_max, ct_stall);
            ct_stall = 0;
        }
        while (ct_stall_max--)
            f_steps.push_back("ST");
    }

    stChecker(i, f_steps.size());
    f_steps.push_back("ID");
    c = 2, ct_stall = 0;
    ct_stall_max = 0;
    while (c < checks[i].size())
    {
        int k = f_steps.size() - 1;
        if (checks[i - 1][1] == checks[i][c]) // if last line has the current token as the destination register
        {
            while (k < f_staller[i - 1].size() && f_staller[i - 1][k++] != "WB")
            {
                ct_stall++;
            }
        }
        else if (i - 2 >= 0 && checks[i - 2][1] == checks[i][c]) // if second last line has the current token as the destination register
        {
            while (k < f_staller[i - 2].size() && f_staller[i - 2][k++] != "WB")
            {
                ct_stall++;
            }
        }
        ++c;
        ct_stall_max = max(ct_stall_max, ct_stall);
        ct_stall = 0;
    }
    while (ct_stall_max--)
        f_steps.push_back("ST");
    stChecker(i, f_steps.size());
    f_steps.push_back("EX");
    stChecker(i, f_steps.size());
    f_steps.push_back("ME");
    stChecker(i, f_steps.size());
    f_steps.push_back("WB");

    f_staller.push_back(f_steps);
    f_steps.clear();
}

// This function will recognize all the instruction in the code in the provided file and execute them
void recog_instr()
{
    int maint = 0;
    bool flag = false;
    for (int i = 0; i < tokens.size(); i++)
    {
        for (int j = 0; j < tokens[i].size(); j++)
        {
            if (tokens[i][j] == "lw")
            {
                int address = stoi(tokens[i][j + 2]) + m[tokens[i][j + 3]];
                m[tokens[i][j + 1]] = Mem[address];
            }
            else if (tokens[i][j].compare("sw") == 0)
            {
                int address = stoi(tokens[i][j + 2]) + m[tokens[i][j + 3]];
                Mem[address] = m[tokens[i][j + 1]];
            }
            else if (tokens[i][j].compare("li") == 0)
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
                if(m[tokens[i][j+3]]==0)
                {
                    cout<<"line "<<i+1<<": MATH ERROR! division by zero\n";
                    exit(1);
                }
                m[tokens[i][j + 1]] = m[tokens[i][j + 2]] / m[tokens[i][j + 3]];
            }
            else if (tokens[i][j] == "addi")
            {
                m[tokens[i][j + 1]] = m[tokens[i][j + 2]] + stoi(tokens[i][j + 3]);
            }
            else if (tokens[i][j] == "bne")
            {
                if (m[tokens[i][j + 1]] != m[tokens[i][j + 2]])
                {
                    checks.push_back(tokens[i]);
                    with_forwarding(maint);
                    without_forwarding(maint);
                    maint++;
                    string str = tokens[i][j + 3];
                    str += ":";
                    i = jt[str];//jump to given location if the branch is taken
                    j = 0;
                }
            }
            else if (tokens[i][j] == "beq")
            {
                if (m[tokens[i][j + 1]] == m[tokens[i][j + 2]])
                {
                    checks.push_back(tokens[i]);
                    with_forwarding(maint);
                    without_forwarding(maint);
                    maint++;
                    string str = tokens[i][j + 3];//jump to given location if the branch is taken
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
                    checks.push_back(tokens[i]);
                    without_forwarding(maint);
                    with_forwarding(maint);
                    maint++;
                    string fun_name = tokens[i][j + 3];//jump to given location if the branch is taken
                    fun_name += ":";
                    i = jt[fun_name];
                    j = 0;
                }
            }
            else if (tokens[i][j] == "blt")
            {
                if (m[tokens[i][j + 1]] < m[tokens[i][j + 2]])
                {
                    checks.push_back(tokens[i]);
                    without_forwarding(maint);
                    with_forwarding(maint);
                    maint++;
                    string fun_name = tokens[i][j + 3];//jump to given location if the branch is taken
                    fun_name += ":";
                    i = jt[fun_name];
                    j = 0;
                }
            }
        }
        if (flag == true)
            break;
        if (tokens[i].size() <= 1)
            continue;
        checks.push_back(tokens[i]);
        without_forwarding(maint);
        with_forwarding(maint);
        maint++;
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
            token = strtok(NULL, " ,()");
        }
        tokens.push_back(Tokenizer);
    }
}

// interacts with the user to perform certain task
void user_interaction()
{
    bool correct_input=false;
    while(correct_input==false)
    {
        cout << "Choose from the below mentioned options:\n 1) Display all register values \n 2) Print value of a particular register \n";
        int inp;
        cin >> inp;
        switch (inp)
        {
        case 1:
        {
            display_reg();
            correct_input=true;
            break;
        }
        case 2:
        {
            string reg;
            cin >> reg;
            if (reg[0] == 'x' && stoi(reg.substr(1, 2)) >= 0 && stoi(reg.substr(1, 2)) < 32) // check if the register is valid
            {
                cout << "Value stored in " << reg << " is: " << m[reg] << endl;
                correct_input=true;
            }
            else
            {
                cout << "Invalid register. Try again!\n\n";
            }
            break;
        }
        default: 
            cout<<"Invalid choice. Try again!\n\n";
            break;
        }
    }
}

// To check stalls in the previous row, i.e. i-1, and k is the row no.
int main()
{
    form_tokens();
    jump();
    assign();
    syntax_error_checker();
    recog_instr();
    cout<<"\nRISC-V SIMULATOR\n";
    cout<<"-----------------\n\n";
    user_interaction();

    int choice;
    bool valid_choice=false;
    while(valid_choice==false)
    {
        cout<<"\nHow do you wish to execute pipeline ?\n";
        cout << "Press 0 for Without Forwarding\nPress 1 for With Forwarding\n";
        cin >> choice;
        
        if(choice==0||choice==1)
                valid_choice=true;
        else
            cout<<"Invalid choice. Try again!\n\n";
    }

    cout<<"\n\nINSTRUCTIONS"<<endl;
    for (int i = 0; i < checks.size(); i++)
    {
        cout<<i+1<<"  ";
        for (int j = 0; j < checks[i].size(); j++)
        {
            cout
                << checks[i][j] << "  ";
        }
        cout << endl;
    }
    cout<<endl;
    
    if (choice == 0)
    {
        cout << "\nWITHOUT FORWARDING: \n" << endl;
        for (int i = 0; i < f_staller.size(); i++)
        {
            cout<<i+1<<" ";
            for (int j = 0; j < f_staller[i].size(); j++)
            {
                cout <<"|  "<< f_staller[i][j] << "  ";
            }
            cout <<"|  "<< endl;
        }
        cout<<"\nTotal cycle count: "<<f_staller[f_staller.size()-1].size()<<endl;
    }
    else
    {
        cout << "\nWITH FORWARDING: \n" << endl;
        for (int i = 0; i < wf_staller.size(); i++)
        {
            cout<<i+1<<" ";
            for (int j = 0; j < wf_staller[i].size(); j++)
            {
                cout <<"|  "<< wf_staller[i][j] << "  ";
            }
            cout <<"|  "<< endl;
        }
        cout<<"\nTotal cycle count: "<<wf_staller[wf_staller.size()-1].size()<<endl;
    }
    return 0;
}
