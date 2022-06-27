/*
 * XMLParser.h
 * Copyright (c) 2015, ZHAW
 * All rights reserved.
 *
 *  Created on: 13.07.2015
 *      Author: honegger
 */

#ifndef XML_PARSER_H_
#define XML_PARSER_H_

#include <string>
#include <cstdlib>
#include <stdint.h>

class XMLParser {
    
    public:
        
        static std::string   parse(std::string str, std::string tag);
        static std::string   parse(std::string str, std::string tag, uint32_t number);
        static std::string   parseString(std::string str);
        static std::string   parseString(std::string str, uint32_t number);
        static std::string   parseString(std::string str, std::string tag);
        static std::string   parseString(std::string str, std::string tag, uint32_t number);
        static bool     parseBoolean(std::string str);
        static bool     parseBoolean(std::string str, uint32_t number);
        static bool     parseBoolean(std::string str, std::string tag);
        static bool     parseBoolean(std::string str, std::string tag, uint32_t number);
        static int16_t  parseShort(std::string str);
        static int16_t  parseShort(std::string str, uint32_t number);
        static int16_t  parseShort(std::string str, std::string tag);
        static int16_t  parseShort(std::string str, std::string tag, uint32_t number);
        static int32_t  parseInt(std::string str);
        static int32_t  parseInt(std::string str, uint32_t number);
        static int32_t  parseInt(std::string str, std::string tag);
        static int32_t  parseInt(std::string str, std::string tag, uint32_t number);
        static int64_t  parseLong(std::string str);
        static int64_t  parseLong(std::string str, uint32_t number);
        static int64_t  parseLong(std::string str, std::string tag);
        static int64_t  parseLong(std::string str, std::string tag, uint32_t number);
        static float    parseFloat(std::string str);
        static float    parseFloat(std::string str, uint32_t number);
        static float    parseFloat(std::string str, std::string tag);
        static float    parseFloat(std::string str, std::string tag, uint32_t number);
        static double   parseDouble(std::string str);
        static double   parseDouble(std::string str, uint32_t number);
        static double   parseDouble(std::string str, std::string tag);
        static double   parseDouble(std::string str, std::string tag, uint32_t number);
};

#endif /* XML_PARSER_H_ */
