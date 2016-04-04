#include <string>
#include <iostream>
#include <RaptorQ.hpp>
#include <fstream>
#include <sstream>
using std::cout;
using std::string;
using T_it = typename std::string::iterator;
const int SYMBOL_SIZE = 1444;

std::string read_file(const string &filename);
std::string read_file(const string &filename)
{
    char rdbuff[4096];
    string result;
    std::ifstream infile(filename);
    while ((bool) infile)
    {
    	infile.read((char *)rdbuff, sizeof(rdbuff));
    	result += string(rdbuff, (unsigned long) infile.gcount());
    }

    return result;
}
void save_blocks(RaptorQ::Encoder<T_it, T_it> &enc, uint32_t total_size, const string &basename="data");
void save_blocks(RaptorQ::Encoder<T_it, T_it> &enc, uint32_t total_size, const string &basename)
{
	const int MAX_REPAIR = 2;
	int file_count = 0;
	auto common = enc.OTI_Common();
	auto scheme_specific = enc.OTI_Scheme_Specific();
	string symbol_buffer;
	symbol_buffer.reserve(SYMBOL_SIZE);
	for (auto block : enc)
	{
		for (auto sym_itor = block.begin_source(); sym_itor != block.end_source(); ++sym_itor)
		{
			symbol_buffer.clear();
			symbol_buffer.insert(symbol_buffer.begin(), SYMBOL_SIZE, '\0');
			auto symbol_itor = symbol_buffer.begin();
			auto written = (*sym_itor)(symbol_itor, symbol_buffer.end());
			std::stringstream filename;
			filename << basename << file_count++ << ".6330";
			std::ofstream outfile(filename.str());
			outfile.write((char *) &total_size, sizeof(total_size));
			outfile.write((char *) &common, sizeof(common));
			outfile.write((char *) &scheme_specific, sizeof(scheme_specific));
			auto id = (*sym_itor).id();
			outfile.write((char *) &id, sizeof(id));
			outfile.write(symbol_buffer.c_str(), written);
		}
		for (auto rep_itor = block.begin_repair(); rep_itor != block.end_repair(2); ++rep_itor)
		{
			symbol_buffer.clear();
			symbol_buffer.insert(symbol_buffer.begin(), SYMBOL_SIZE, '\0');
			auto symbol_itor = symbol_buffer.begin();
			auto written = (*rep_itor)(symbol_itor, symbol_buffer.end());
			std::stringstream filename;
			filename << basename << file_count++ << ".6330";
			std::ofstream outfile(filename.str());
			outfile.write((char *) &total_size, sizeof(total_size));
			outfile.write((char *) &common, sizeof(common));
			outfile.write((char *) &scheme_specific, sizeof(scheme_specific));
			auto id = (*rep_itor).id();
			outfile.write((char *) &id, sizeof(id));
			outfile.write(symbol_buffer.c_str(), written);

		}
	}
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
    	cout << "Usage: " << argv[0] << " <filename\n";
    	return -1;
    }
	string file_data = read_file(argv[1]);
	cout << argv[1] << " has " << file_data.size() << " bytes\n";

    RaptorQ::Encoder<T_it, T_it> enc ( file_data.begin(),file_data.end(),
    SYMBOL_SIZE , SYMBOL_SIZE , 10000);

    if ((bool) enc)
    {
        cout << "Great Success!\n";
        save_blocks(enc, file_data.size(),"test");
    }
    else
    {
        cout << "Epic failure\n";
    }

    return 0;
}
