#include <string>
#include <iostream>
#include <RaptorQ.hpp>
#include <fstream>
#include <sstream>
#include <cstdint>
using std::cout;
using std::string;
using T_it = typename std::string::iterator;

void process_file(char *filename, RaptorQ::Decoder<T_it, T_it> &dec)
{
	std::ifstream infile(filename);
	cout << "Processing: " << filename << "\n";
	RaptorQ::OTI_Common_Data common;
	RaptorQ::OTI_Scheme_Specific_Data scheme_specific;
	uint32_t total_size;
	std::ifstream in(filename);
	in.read((char *) &total_size, sizeof(total_size));
	in.read((char *) &common, sizeof(common));
	in.read((char *) &scheme_specific, sizeof(scheme_specific));
	uint32_t id;
	in.read((char *) &id, sizeof(id));
	char rdbuff[4096];
	string symbol_data;
	while ((bool) in)
	{
		in.read(rdbuff, sizeof(rdbuff));
		symbol_data += string(rdbuff, in.gcount());
	}
	auto data_itor = symbol_data.begin();
	dec.add_symbol(data_itor, symbol_data.end(), id);

}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		cout << "Usage: " << argv[0] << "<output file> <file1> [<file2> ... <fileN>]\n";
		return -1;
	}
	uint32_t total_size;
	RaptorQ::OTI_Common_Data common;
	RaptorQ::OTI_Scheme_Specific_Data scheme_specific;
	std::ifstream in(argv[1]);
	in.read((char *) &total_size, sizeof(total_size));
	in.read((char *) &common, sizeof(common));
	in.read((char *) &scheme_specific, sizeof(scheme_specific));
	RaptorQ::Decoder<T_it, T_it> dec(common, scheme_specific);
	for (int i = 1; i < argc; i++)
	{
		process_file(argv[i], dec);
	}
	string result;
	result.reserve(total_size);
	result.insert(result.begin(), total_size, '\0');

	auto result_itor = result.begin();
	auto decoded = dec.decode(result_itor, result.end());
	if (0 == decoded)
	{
		cout << "Decoding failed\n";
		return -1;
	}
	cout << "Decoding succeeded\n";
	std::ofstream out(argv[1]);
	out.write(result.c_str(),result.size());
	return 0;
}
