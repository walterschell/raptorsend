#include <string>
#include <iostream>
#include <RaptorQ.hpp>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
//define OUTPUT_FILES
using std::cout;
using std::string;
using T_it = typename std::string::iterator;
const int SYMBOL_SIZE = 1444;
class __attribute__((packed)) SymbolHeader
{
public:
	uint32_t total_size;
	RaptorQ::OTI_Common_Data common_data; //uint64_t
	RaptorQ::OTI_Scheme_Specific_Data scheme_specific_data; //uint32_t
	uint32_t symbol_id; //uint32_t
	std::string str() const;
};
std::string SymbolHeader::str() const
{
	//TODO: Serialize in network byte order
	return std::string((char *) this, sizeof(SymbolHeader));
}
class UDPSocket
{
public:
	UDPSocket(const std::string &ip, uint16_t port)
	{
		if ((fd = socket(PF_INET, SOCK_DGRAM, 0x11)) < 0)
		{
			perror("socket");
			throw std::exception();
		}
		sockaddr_in da={0};
		da.sin_family = AF_INET;
#ifdef	__APPLE__
		da.sin_len = 4;
#endif /*__APPLE__*/
		inet_pton(AF_INET, ip.c_str(), &(da.sin_addr));
		da.sin_port= htons(port);
		if (connect(fd, (sockaddr *)&da, sizeof(sockaddr_in)) < 0)
		{
			perror("Connect");
			throw std::exception();
		}
	}
	~UDPSocket()
	{
		close(fd);
	}
	void send(const std::string &data)
	{
		::send(fd, data.c_str(), data.size(), 0);
	}
private:
	int fd;
};

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
	UDPSocket socket("127.0.0.1", 9999);
	SymbolHeader symbol_header;
	symbol_header.total_size = total_size;
	symbol_header.common_data = common;
	symbol_header.scheme_specific_data = scheme_specific;


	for (auto block : enc)
	{
		for (auto sym_itor = block.begin_source(); sym_itor != block.end_source(); ++sym_itor)
		{
			symbol_buffer.clear();
			symbol_buffer.insert(symbol_buffer.begin(), SYMBOL_SIZE, '\0');
			auto symbol_itor = symbol_buffer.begin();
			auto written = (*sym_itor)(symbol_itor, symbol_buffer.end());
			auto id = (*sym_itor).id();
			symbol_header.symbol_id = id;
#ifdef OUTPUT_FILES
			std::stringstream filename;
			filename << basename << file_count++ << ".6330";
			std::ofstream outfile(filename.str());
			outfile.write((char *) &total_size, sizeof(total_size));
			outfile.write((char *) &common, sizeof(common));
			outfile.write((char *) &scheme_specific, sizeof(scheme_specific));

			outfile.write((char *) &id, sizeof(id));
			outfile.write(symbol_buffer.c_str(), written);
#endif /*OUTPUT_FILES*/
			socket.send(symbol_header.str() + symbol_buffer);
		}
		for (auto rep_itor = block.begin_repair(); rep_itor != block.end_repair(2); ++rep_itor)
		{
			symbol_buffer.clear();
			symbol_buffer.insert(symbol_buffer.begin(), SYMBOL_SIZE, '\0');
			auto symbol_itor = symbol_buffer.begin();
			auto written = (*rep_itor)(symbol_itor, symbol_buffer.end());
			auto id = (*rep_itor).id();
			symbol_header.symbol_id = id;
#ifdef OUTPUT_FILES
			std::stringstream filename;
			filename << basename << file_count++ << ".6330";
			std::ofstream outfile(filename.str());
			outfile.write((char *) &total_size, sizeof(total_size));
			outfile.write((char *) &common, sizeof(common));
			outfile.write((char *) &scheme_specific, sizeof(scheme_specific));
			outfile.write((char *) &id, sizeof(id));
			outfile.write(symbol_buffer.c_str(), written);
#endif
			socket.send(symbol_header.str() + symbol_buffer);
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
