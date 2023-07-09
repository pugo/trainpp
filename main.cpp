/* TrainPP
 * Copyright (C) 2023 Anders Piniesjö
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>

#include "z21/z21.h"

using namespace boost;
namespace po = boost::program_options;


int main(int argc, char* argv[])
{
    std::string z21_host;
    std::string z21_port = "21105";

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,?", "produce help message")
                ("z21-host,h", po::value<std::string>(&z21_host), "Z21 host or IP address")
                ("z21-port,p", po::value<std::string>(&z21_port), "Z21 port (default: 21105)");
//                ("output-dir,o", po::value<std::string>(&output_dir)->required(), "output directory");

        po::positional_options_description p;
//        p.add("input-file", -1);

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

    Z21 z21(z21_host, z21_port);
    z21.connect();
    z21.listen();

    return 0;
}
