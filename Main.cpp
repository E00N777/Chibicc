#include <iostream>
#include <cstdlib>

int main(int argc, char** argv)
{
    if(argc!=2)
    {
        std::cerr<<"[Fatal Error]:Invalid number of arguments\n"<<std::endl;
        std::exit(1);
    }

    char *p = argv[1];

    std::cout<<"    .globl main\n";
    std::cout<<"main:\n";
    //Continuing to use strtol from C standard lib for simple lexical analysis
    std::cout<<"    mov $"<<std::strtol(p, &p ,10)<<", %rax\n";
    while (*p) {
        if(*p=='+')
        {
            p++;
            std::cout<<"    add $"<<std::strtol(p, &p ,10)<<", %rax\n";
            continue;
        }
        if(*p=='-')
        {
            p++;
            std::cout<<"    sub $"<<std::strtol(p, &p ,10)<<", %rax\n";
            continue;
        }
         std::cerr<<"[Fatal Error]:Invalid operator\n"<<std::endl;
         return 1;
    }
    std::cout<<"    ret\n";

    return 0;

}