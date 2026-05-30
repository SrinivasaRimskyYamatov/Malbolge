#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <string>

/* ===== Malbolge op ===== */
unsigned short op(unsigned short x, unsigned short y){
    static const unsigned short p9[5]={1,9,81,729,6561};
    static const unsigned short o[9][9]={
        {4,3,3,1,0,0,1,0,0},{4,3,5,1,0,2,1,0,2},{5,5,4,2,2,1,2,2,1},
        {4,3,3,1,0,0,7,6,6},{4,3,5,1,0,2,7,6,8},{5,5,4,2,2,1,8,8,7},
        {7,6,6,7,6,6,4,3,3},{7,6,8,7,6,8,4,3,5},{8,8,7,8,8,7,5,5,4}
    };
    unsigned short r=0;
    for(int i=0;i<5;i++)
        r+=o[y/p9[i]%9][x/p9[i]%9]*p9[i];
    return r;
}

int main(int argc,char**argv){
    if(argc!=2){
        std::cerr<<"usage: mbc file.mb\n";
        return 1;
    }

    std::string src = argv[1];
    std::string base = src.substr(0, src.find_last_of('.'));

    std::ifstream in(src);
    if(!in){
        std::cerr<<"cannot open source\n";
        return 1;
    }

    /* ===== build initial memory ===== */
    std::vector<unsigned short> mem(59049);
    unsigned short i=0;
    int ch;

    while((ch=in.get())!=EOF){
        if(ch<=32 || ch>=127) continue;
        mem[i++] = ch;
        if(i>=59049){
            std::cerr<<"source too long\n";
            return 1;
        }
    }
    while(i<59049){
        mem[i] = op(mem[i-1], mem[i-2]);
        i++;
    }

    /* ===== generate C++ ===== */
    std::ofstream out(base + ".cpp");

    out << R"(#include <cstdio>

static const char xlat1[] =
"+b(29e*j1VMEKLyC})8&m#~W>qxdRp0wkrUo[D7,XTcA\"lI"
".v%{gJh4G\\-=O@5`_3i<?Z';FNQuY]szf$!BS/|t:Pn6^Ha";

static const char xlat2[] =
"5z]&gqtyfr$(we4{WP)H-Zn,[%\\3dL+Q;>U!pJS72FhOA1C"
"B6v^=I_0/8|jsb9m<.TVac`uY*MK'X~xDl}REokN:#?G\"i@";

static inline unsigned short op(unsigned short x,unsigned short y){
    static const unsigned short p9[5]={1,9,81,729,6561};
    static const unsigned short o[9][9]={
        {4,3,3,1,0,0,1,0,0},{4,3,5,1,0,2,1,0,2},{5,5,4,2,2,1,2,2,1},
        {4,3,3,1,0,0,7,6,6},{4,3,5,1,0,2,7,6,8},{5,5,4,2,2,1,8,8,7},
        {7,6,6,7,6,6,4,3,3},{7,6,8,7,6,8,4,3,5},{8,8,7,8,8,7,5,5,4}
    };
    unsigned short r=0;
    for(int i=0;i<5;i++)
        r+=o[y/p9[i]%9][x/p9[i]%9]*p9[i];
    return r;
}

static unsigned short mem[59049]={)";
    for(int k=0;k<59049;k++){
        out<<mem[k]<<",";
        if(k%16==15) out<<"\n";
    }

    out << R"(};

int main(){
    unsigned short a=0,c=0,d=0;
    int x;
    FILE* log=fopen(")" << base << R"(.asm","w");

    for(;;){
        if(mem[c]>=33 && mem[c]<=126){
            switch(xlat1[(mem[c]-33+c)%94]){
                case 'j':
                    fprintf(log,"MOV D,[D] A:%u C:%u MC:%u D:%u MD:%u\n",
                            a,c,mem[c],d,mem[d]);
                    d=mem[d];
                    break;

                case 'i':
                    fprintf(log,"MOV C,[D] A:%u C:%u MC:%u D:%u MD:%u\n",
                            a,c,mem[c],d,mem[d]);
                    c=mem[d];
                    break;

                case '*':
                    fprintf(log,"ROTR[D] A:%u C:%u MC:%u D:%u MD:%u\n",
                            a,c,mem[c],d,mem[d]);
                    a=mem[d]=mem[d]/3+mem[d]%3*19683;
                    break;

                case 'p':
                    fprintf(log,"CRZ A,[D] A:%u C:%u MC:%u D:%u MD:%u\n",
                            a,c,mem[c],d,mem[d]);
                    a=mem[d]=op(a,mem[d]);
                    break;

                case '<':
                    fprintf(log,"OUT A:%u C:%u MC:%u D:%u MD:%u\n",
                            a,c,mem[c],d,mem[d]);
                    putchar(a);
                    break;

                case '/':
                    x=getchar();
                    a=(x==EOF?59048:x);
                    break;

                case 'v':
                    fprintf(log,"EXIT A:%u C:%u MC:%u D:%u MD:%u\n",
                            a,c,mem[c],d,mem[d]);
                    fclose(log);
                    return 0;
            }
            mem[c]=xlat2[mem[c]-33];
        }
        if(++c==59049) c=0;
        if(++d==59049) d=0;
    }
}
)";
    out.close();

    /* ===== compile ===== */
    std::string cmd = "g++ " + base + ".cpp -O2 -o " + base;
    system(cmd.c_str());

    std::cout<<"generated: "<<base<<" , "<<base<<".asm\n";
    return 0;
}
