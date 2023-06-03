#include <iostream>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>

#include "z21/z21.h"


using namespace boost;
namespace po = boost::program_options;



int main(int argc, char* argv[]) {

    std::string input_file;
    std::string public_key_file;
    std::string output_dir;

    try {
        po::options_description desc("Allowed options");
        desc.add_options();
//                ("help,?", "produce help message")
//                ("input-file,i", po::value<std::string>(&input_file)->required(), "input file")
//                ("public-key,p", po::value<std::string>(&public_key_file)->required(), "public RSA key file")
//                ("output-dir,o", po::value<std::string>(&output_dir)->required(), "output directory");

        po::positional_options_description p;
        p.add("input-file", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << "Usage: options_description [options]" << std::endl << desc;
            return 0;
        }
    }
    catch(std::exception& e)
    {
        std::cout <<  e.what() << std::endl;
        return 1;
    }

    std::cout << "Hello, World!" << std::endl;

    Z21 z21;
    z21.connect();
    z21.listen();

    return 0;
}
