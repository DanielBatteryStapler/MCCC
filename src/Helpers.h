#ifndef HELPERS_H_INCLUDED
#define HELPERS_H_INCLUDED

#include <string>
#include <fstream>
#include <sstream>
#include "Definitions.h"

static std::string loadFile(std::string filename){
    std::fstream input(filename);
    
    if(!input){
        throw std::runtime_error("Unable to open file \"" + filename + "\"");
    }
    
    return std::string((std::istreambuf_iterator<char>(input)),(std::istreambuf_iterator<char>()));
}

static std::string toHex(Byte in){
	std::stringstream ss;
	ss << std::hex << static_cast<std::uint32_t>(in);
	std::string out = ss.str();
	while(out.size() < 4){
		out = "0" + out;
	} 
	return out;
}

#endif // HELPERS_H_INCLUDED
