#include "MyRpcApplication.h"
#include "boost/program_options.hpp"
namespace po = boost::program_options;

MyRpcApplication& MyRpcApplication::init(int argc, char** argv) {
    if(argc < 2) {
        std::cout<<"params error, please input \'-i <filepath>\'"<<std::endl;
        exit(EXIT_FAILURE);
    }
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "print help message")
        ("input,i", po::value<std::string>(), "input file path");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch(const po::error& e) {
        std::cerr<<"Error : "<<e.what()<<std::endl;
        exit(EXIT_FAILURE);
    }

    if(vm.count("help")) {
        std::cout<< desc <<std::endl;
    }    
    if(vm.count("input")) {
        std::string filePath = vm["input"].as<std::string>();
        fileUtils.load(filePath, fileInfo);
    }
    return *this;
}

std::unordered_map<std::string, std::string> MyRpcApplication::getFileInfo() {
    return fileInfo;
}

MyRpcApplication& MyRpcApplication::getInstance(){
    static MyRpcApplication application;
    return application;
}




