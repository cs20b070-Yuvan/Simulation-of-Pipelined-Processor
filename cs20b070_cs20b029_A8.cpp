#include <bits/stdc++.h>
#include <iostream>
#include <fstream>

using namespace std;

//declaring global variables so that they can be updated
//by all the functions declared below
int instruction_cache[128][16];//256B = (128*16)bits/8
int data_cache[256][8];//256B = 256*8
vector<int> DataArray(256,0); //used to store data as an int
//for accessing its int form and make changes
int register_file[16][8];

//variables to be printed in Output.txt (Statistics)
int NumOfIns = 0;
int ArithmeticIns = 0;
int LogicalIns = 0;
int DataTransferIns = 0;
int ControlTransferIns = 0;
int HaltIns = 0;
int PC = 0;

//read source operands from the RF and the outputs of the general
int LMD;
int dataHazard = 0; //boolean variable
int Hazards = 0;
int Branch = 0;//detect branch and decrement
int RAWHazards = 0;
int ControlHazards = 0;
//Result is temporarily stored in ALUOutput register
int ALUoutput;
int Halt = 0;
//We maintain array of # of registers of = 1 entries
//for checking purpose of Data hazards
vector<int> RAWcheck(16,1);
vector<int> registers(16,0);

//Queues
queue<vector<int>> IF;
queue<vector<int>> ID;
queue<vector<int>> EX;
queue<vector<int>> ME;
queue<vector<int>> WB;

//utilize functions
//Hexadecimal To Binary
void convert(string hexa,int* a)
{
    long int i = 0;
    while (hexa[i])
    {
        switch (hexa[i])
        {
        case '0':
        {
            a[i * 4 + 0] = 0;
            a[i * 4 + 1] = 0;
            a[i * 4 + 2] = 0;
            a[i * 4 + 3] = 0;
        }
        break;
        case '1':
        {
            a[i * 4 + 0] = 0;
            a[i * 4 + 1] = 0;
            a[i * 4 + 2] = 0;
            a[i * 4 + 3] = 1;
        }
        break;
        case '2':
            //cout << "0010";
            a[i * 4 + 0] = 0;
            a[i * 4 + 1] = 0;
            a[i * 4 + 2] = 1;
            a[i * 4 + 3] = 0;
            break;
        case '3':
            //cout << "0011";
            a[i * 4 + 0] = 0;
            a[i * 4 + 1] = 0;
            a[i * 4 + 2] = 1;
            a[i * 4 + 3] = 1;
            break;
        case '4':
            //cout << "0100";
            a[i * 4 + 0] = 0;
            a[i * 4 + 1] = 1;
            a[i * 4 + 2] = 0;
            a[i * 4 + 3] = 0;
            break;
        case '5':
            //cout << "0101";
            a[i * 4 + 0] = 0;
            a[i * 4 + 1] = 1;
            a[i * 4 + 2] = 0;
            a[i * 4 + 3] = 1;
            break;
        case '6':
            //cout << "0110";
            a[i * 4 + 0] = 0;
            a[i * 4 + 1] = 1;
            a[i * 4 + 2] = 1;
            a[i * 4 + 3] = 0;
            break;
        case '7':
            //cout << "0111";
            a[i * 4 + 0] = 0;
            a[i * 4 + 1] = 1;
            a[i * 4 + 2] = 1;
            a[i * 4 + 3] = 1;
            break;
        case '8':
            //cout << "1000";
            a[i * 4 + 0] = 1;
            a[i * 4 + 1] = 0;
            a[i * 4 + 2] = 0;
            a[i * 4 + 3] = 0;
            break;
        case '9':
            //cout << "1001";
            a[i * 4 + 0] = 1;
            a[i * 4 + 1] = 0;
            a[i * 4 + 2] = 0;
            a[i * 4 + 3] = 1;
            break;
        case 'A':
        case 'a':
            //cout << "1010";
            a[i * 4 + 0] = 1;
            a[i * 4 + 1] = 0;
            a[i * 4 + 2] = 1;
            a[i * 4 + 3] = 0;
            break;
        case 'B':
        case 'b':
            //cout << "1011";
            a[i * 4 + 0] = 1;
            a[i * 4 + 1] = 0;
            a[i * 4 + 2] = 1;
            a[i * 4 + 3] = 1;
            break;
        case 'C':
        case 'c':
            //cout << "1100";
            a[i * 4 + 0] = 1;
            a[i * 4 + 1] = 1;
            a[i * 4 + 2] = 0;
            a[i * 4 + 3] = 0;
            break;
        case 'D':
        case 'd':
            //cout << "1101";
            a[i * 4 + 0] = 1;
            a[i * 4 + 1] = 1;
            a[i * 4 + 2] = 0;
            a[i * 4 + 3] = 1;
            break;
        case 'E':
        case 'e':
            //cout << "1110";
            a[i * 4 + 0] = 1;
            a[i * 4 + 1] = 1;
            a[i * 4 + 2] = 1;
            a[i * 4 + 3] = 0;
            break;
        case 'F':
        case 'f':
            //cout << "1111";
            a[i * 4 + 0] = 1;
            a[i * 4 + 1] = 1;
            a[i * 4 + 2] = 1;
            a[i * 4 + 3] = 1;
            break;
        }
        i++;
    }
}

void decToHexa(int n,char *a)
{
    // counter for hexadecimal number array
    int i = 0;
    while (i < 2)
    {
        // temporary variable to store remainder
        int temp = 0;

        // storing remainder in temp variable.
        temp = n % 16;

        // check if temp < 10
        if (temp < 10)
        {
            a[i] = temp + 48;
            i++;
        }
        else
        {
            a[i] = temp + 55;
            i++;
        }

        n = n / 16;
    }
    return;
}

int HexatoDecimal(int* a,int j)   //Conversion of  4 digit binary to 1 digit Hexa
{
    int t = 1,x = 0;
    for(int i = j; i >= j-3; i--)
    {
        x += a[i]*t;
        t *= 2;
    }
    return x;
}

//5 stage pipe line

void fetch()
{
    if(!Halt)
    {
        int inst[16]; //2B fixed length instruction
        for (int i = 0; i < 16; i++)
        {
            inst[i] = instruction_cache[PC][i]; //fetch from Instruction cache
        }
        PC++; //update PC
        //Then we are advancing by 2 Bytes (16 bits)

        //Take the decimal value of instruction and keep it into queue

        vector<int> v(4);
        for(int i = 0; i<4; i++)
        {
            v[i] = HexatoDecimal(inst,3+4*i);
        }

        if(v[0] == 15)
            Halt = 1;
        IF.push(v);
    }

}

void decode()
{
    if(!IF.empty())
    {
        auto ins = IF.front();
        IF.pop();

        NumOfIns++; //number instructions increased
        ID.push(ins);

        if(ins[0] == 15)
        {
            Halt = 1;
        }
    }

    if(!ID.empty())
    {
        auto ins = ID.front();
        int opcode = ins[0];

        //If there are three operands
        if(opcode == 0 || opcode == 1 || opcode == 2 || opcode == 4 || opcode == 5 || opcode == 7)
        {
            if(RAWcheck[ins[2]] && RAWcheck[ins[3]])
            {
                //reserving the destination operand
                RAWcheck[ins[1]] = 0;
                //we can't use this operand to read in the further instructions
                auto newins = ID.front();
                //push back zero for indicating it is valid
                newins.push_back(0);
                EX.push(newins);
                ID.pop();
                if(opcode < 3)
                    ArithmeticIns++;
                else
                    LogicalIns++;
            }
            else
            {
                RAWHazards++;
                //boolean bit which indicates there is a datahazard
                dataHazard = 1;
            }
        }
        //If there are two operands
        else if(opcode == 6 || opcode == 8)//not and load
        {
            if(RAWcheck[ins[2]])
            {
                //reserving the destination operand
                RAWcheck[ins[1]] = 0;
                //we can't use this operand to read in the further instructions
                auto newins = ID.front();
                //push back zero for indicating it is valid
                newins.push_back(0);
                EX.push(newins);
                ID.pop();
                if(opcode == 6)
                    LogicalIns++;
                else
                    DataTransferIns++;
            }
            else
            {
                RAWHazards++;
                //boolean bit which indicates there is a datahazard
                dataHazard = 1;
            }
        }
        else if(opcode == 9) // store
        {
            if(RAWcheck[ins[1]] && RAWcheck[ins[2]])
            {
                //we can't use this operand to read in the further instructions
                auto newins = ID.front();
                //push back zero for indicating it is valid
                newins.push_back(0);
                EX.push(newins);
                ID.pop();

                DataTransferIns++;
            }
            else
            {
                RAWHazards++;
                //boolean bit which indicates there is a datahazard
                dataHazard = 1;
            }
        }
        else if(opcode == 3)
        {
            if(RAWcheck[ins[1]])
            {
                //reserving the destination operand
                RAWcheck[ins[1]] = 0;
                //push back zero for indicating it is valid
                auto newins = ID.front();
                newins.push_back(0);
                EX.push(newins);
               if(ID.empty())
               {
                   ID.pop();
               }


                ArithmeticIns++;
            }
            else
            {
                RAWHazards++;
                //boolean bit which indicates there is a datahazard
                dataHazard = 1;
            }
        }
        else if(opcode == 11)
        {
            if(RAWcheck[ins[1]])
            {
                auto newins = ID.front();
                ControlHazards += 2;
                Branch = 2;
                if(registers[ins[1]]==0)
                {
                    newins.push_back(1);
                    while(!IF.empty())
                        IF.pop();
                    while(!ID.empty())
                        ID.pop();
                    Halt = 0;
                }
                else
                    newins.push_back(0);

                EX.push(newins);
                if(!ID.empty()) ID.pop();

                ControlTransferIns++;
            }
            else
            {
                RAWHazards++;
                //boolean bit which indicates there is a datahazard
                dataHazard = 1;
            }
        }
        else
        {
            auto newins = ID.front();
            if(ins[0] == 10) 				 //If it is jump operation//Always stall 2 cycles
            {
                Branch = 2;
                Halt = 0;
                newins.push_back(1);
                //while(!IF.empty()) IF.pop();
                //while(!ID.empty()) ID.pop();
                ControlHazards += 2;	     //stall 2 cycles for controlHazards
            }
            else
            {
                HaltIns++;
                newins.push_back(0);
            }
            EX.push(newins);
            if(!ID.empty())ID.pop();
        }

    }

}

void execute()
{
    if(!EX.empty())
    {
        vector<int> newins = EX.front();
        EX.pop();
        int opcode = newins[0];
        if(opcode == 0)
        {
            ALUoutput = registers[newins[2]] + registers[newins[3]];
            vector<int> temp;
            temp.push_back(ALUoutput);
            temp.push_back(newins[1]);
            ME.push(temp);
        }
        else if(opcode == 1)
        {
            ALUoutput = registers[newins[2]] - registers[newins[3]];	//Sub
            vector<int> temp;
            temp.push_back(ALUoutput);
            temp.push_back(newins[1]);
            ME.push(temp);
        }
        else if(opcode == 2)
        {
            ALUoutput = registers[newins[2]] * registers[newins[3]];	//Mul
            vector<int> temp;
            temp.push_back(ALUoutput);
            temp.push_back(newins[1]);
            ME.push(temp);
        }
        else if(opcode == 3)
        {
            ALUoutput = registers[newins[1]] + 1;			//Inc
            vector<int> temp;
            temp.push_back(ALUoutput);
            temp.push_back(newins[1]);
            ME.push(temp);
        }
        else if(opcode == 4)
        {
            ALUoutput = registers[newins[2]] & registers[newins[3]];	//and
            vector<int> temp;
            temp.push_back(ALUoutput);
            temp.push_back(newins[1]);
            ME.push(temp);
        }
        else if(opcode == 5)
        {
            ALUoutput = registers[newins[2]] | registers[newins[3]]; 	//or
            vector<int> temp;
            temp.push_back(ALUoutput);
            temp.push_back(newins[1]);
            ME.push(temp);
        }
        else if(opcode == 6)
        {
            ALUoutput = ~registers[newins[2]];	//not
            vector<int> temp;
            temp.push_back(ALUoutput);
            temp.push_back(newins[1]);
            ME.push(temp);
        }
        else if(opcode == 7)
        {
            ALUoutput = registers[newins[2]] ^ registers[newins[3]];	//xor
            vector<int> temp;
            temp.push_back(ALUoutput);
            temp.push_back(newins[1]);
            ME.push(temp);
        }
        else if(opcode == 8 || opcode==9)
        {
            int x = ((newins[3] & 1<<3) == 0 ? newins[3] : (newins[3]-16));
            ALUoutput = registers[newins[2]] + x;
            vector<int> temp;
            temp.push_back(ALUoutput);	//Load store effective address calculation
            temp.push_back(newins[1]);
            temp.push_back(opcode-8);
            ME.push(temp);
        }
        else if(opcode == 10 || opcode==11)
        {
            vector<int> temp;
            if(newins[4]==1)
            {
                int unsignInt = 0;
                if(opcode==10)
                {
                    unsignInt = 16*newins[1] + newins[2]; //Jmp and branch, Target address calculation
                }
                else
                {
                    unsignInt = 16*newins[2] + newins[3];
                }
                int signInt = ((unsignInt & 1<<7) == 0 ? unsignInt : (unsignInt - 256));
                PC = PC + signInt;
                ME.push(temp);
            }
            else ME.push(temp);
        }
        else if(opcode == 15)  //Halt,Push temporary vector
        {
            vector<int> temp;
            ME.push(temp);
        }
    }
}

void memaccess()
{
    if(!ME.empty())
    {
        auto newins = ME.front();
        ME.pop();
        vector<int> temp;
        if(newins.size()==3) //If its a store  or register load operation(mem[2] == 1 is store otherwise loading in register)
        {
            if(newins[2] == 1)
            {
                DataArray[newins[0]] = registers[newins[1]];
            }
            else
            {
                LMD = DataArray[newins[0]];
                temp.push_back(LMD);
                temp.push_back(newins[1]);
                temp.push_back(0);
            }
        }

        else if(newins.size()==2) 	//Forwarding register register alu operations to write back
        {
            temp = newins;
        }
        WB.push(temp);		//Forwarding dummy vector otherwise
    }
}


void writeback()
{
    if(!WB.empty())
    {
        auto newins = WB.front();
        WB.pop();

        if(newins.size() == 2 || newins.size() == 3)
        {
            registers[newins[1]] = newins[0];
            RAWcheck[newins[1]] = 1;
        }
    }
}

int main()
{
    ifstream InsCache;
    //open the ICache.txt file
    InsCache.open("ICache.txt");

    for (int i = 0; i < 128; i++)
    {
        char a, b, c, d;
        InsCache >> a >> b >> c >> d;
        string ins; //ins is hexa decimal 16 bit instruction
        ins.push_back(a);
        ins.push_back(b);
        ins.push_back(c);
        ins.push_back(d);

        int* arr = new int[16];

        for(int k = 0; k<16; k++)
            arr[k] = 0;
        //convert hexa to binary
        convert(ins, arr);
        for (int j = 0; j < 16; j++)
        {
            instruction_cache[i][j] = arr[j];
        }
    }
    InsCache.close(); //close file


    ifstream Datacache;
    //open Datacache.txt
    Datacache.open("DCache.txt");
    for (int i = 0; i < 256; i = i + 2)
    {
        char a, b, c, d;
        Datacache >> a >> b >> c >> d;

        string dat; //8 bit data taken adjacent to each other
        dat.push_back(a);
        dat.push_back(b);
        dat.push_back(c);
        dat.push_back(d);

        int* arr = new int[16];

        for(int i = 0; i<16; i++)
            arr[i] = 0;
        //convert hexa to binary
        convert(dat, arr);

        for (int j = 0; j < 8; j++)
        {
            data_cache[i][j] = arr[j];
        }

        for (int j = 8; j < 16; j++)
        {
            data_cache[i + 1][j - 8] = arr[j];
        }
    }
    Datacache.close(); //close file

    //Populating the DataArray vector with the data in DCache as int
    for(int i = 0; i<256; i++)
    {
        int temp = 0;
        int k = 1;
        for(int j = 7; j>=1; j--)
        {
            temp += (data_cache[i][j]*k);
            k *= 2;
        }
        temp -= (256*(data_cache[i][0])); //2's compliment form
        DataArray[i] = temp;
    }

    ifstream RegFile;
    //open Datacache.txt
    RegFile.open("RF.txt");
    for (int i = 0; i < 16; i = i + 2)
    {
        char a, b, c, d;
        RegFile >> a >> b >> c >> d;

        string reg; //8 bit data taken adjacent to each other
        //
        reg.push_back(a);
        reg.push_back(b);
        reg.push_back(c);
        reg.push_back(d);

        int* arr = new int[16];

        for(int i = 0; i<16; i++)
            arr[i] = 0;
        //convert hexa to binary
        convert(reg, arr);

        for (int j = 0; j < 8; j++)
        {
            register_file[i][j] = arr[j];
        }

        for (int j = 8; j < 16; j++)
        {
            register_file[i + 1][j - 8] = arr[j];

        }
    }
    RegFile.close(); //close file

    //Populating the registers vector with the data in registers as int
    for(int i = 0; i<16; i++)
    {
        int temp = 0;
        int k = 1;
        for(int j = 7; j>=1; j--)
        {
            temp += register_file[i][j]*k;
            k *= 2;
        }
        temp -= 256*(register_file[i][0]); //2's compliment form
        registers[i] = temp;
    }

    int cycles = 0;
    Branch = 0;
    /***
    THE MAIN PIPELINE
    1st iteration IF
    2nd iteration ID IF
    3rd iteration EX ID IF
    ...
    ***/
    while(1) 															//loop,each iteration is one cycle
    {
        writeback();
        memaccess();
        execute();
        decode();

        if(dataHazard > 0)
        {
            dataHazard--;
            cycles++;
            continue;
        }
        if(Branch > 0)
        {
            Branch--;
            cycles++;
            continue;
        }

        fetch();
        cycles++;

        if(Halt && IF.empty() && ID.empty() && EX.empty() && ME.empty() && WB.empty())
            break;
    }

    ofstream datfile;
    datfile.open("DCacheOut.txt");											//printing output
    for(int i = 0; i < 256; i++)
    {
        int x = DataArray[i];
        char a[2];
        decToHexa(x,a);
        datfile << a[0] << a[1] << "\n";
    }
    datfile.close();

    ofstream regfile;
    regfile.open("RFOut.txt");
    for(int i = 0; i < 16; i++)
    {
        int x = registers[i];
        char a[2];
        decToHexa(x,a);
        regfile << a[0] << a[1] << "\n";
    }
    regfile.close();

    Hazards = RAWHazards + ControlHazards;

    //Statistics
    ofstream outfile;
    outfile.open("Output.txt");
    outfile << "Total number of instructions executed  : "<<NumOfIns << "\n";
    outfile << "Number of instructions in each class\n";
    outfile << "Arithmetic instructions                : "<<ArithmeticIns << "\n";
    outfile << "Logical instructions                   : "<<LogicalIns << "\n";
    outfile << "Data instructions                      : "<<DataTransferIns<< "\n";
    outfile << "Control instructions                   : "<<ControlTransferIns << "\n";
    outfile << "Halt instructions                      : "<<HaltIns << "\n";
    outfile << "Cycles Per Instruction                 : "<<(double)cycles/NumOfIns << "\n";//CPI = clock cycles/ins count
    outfile << "Total number of stalls                 : "<<Hazards << "\n";
    outfile << "Data stalls (RAW)                      : "<<RAWHazards << "\n";
    outfile << "Control stalls                         : "<<ControlHazards << "\n";
    outfile.close();

    return 0;
}
