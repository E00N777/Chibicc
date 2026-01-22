#include <iostream>

int main(int argc, char** argv)
{
    if(argc!=2)
    {
        std::cerr<<"[Fatal Error]:Invalid number of arguments\n"<<std::endl;
        std::exit(1);
    }

    std::cout<<"    .globl main\n";
    std::cout<<"main:\n";
    std::cout<<"    mov $"<<std::atoi(argv[1])<<", %rax\n";
    std::cout<<"    ret\n";

    return 0;

}