
//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


/**
* @file      cmd_line.hpp
* @brief     very basic command line parser
*/

#ifndef GU_CMD_LINE_HPP
#define GU_CMD_LINE_HPP

#include <string>
#include <vector>
#include <utility>

namespace gu
{

    using cmd_param = std::pair<std::string, std::string>;
    using cmd_param_list = std::vector<cmd_param>;

    /**
    * @brief command line parser
    * @note command line format:  -parameter value
    */
    class cmd_line
    {
        cmd_param_list pl{};

    public:

        /**
        * @brief initialize a command line parser
        * @param cmdline command line string
        */
        explicit cmd_line(std::string const& cmdline);

        /**
        * @brief initialize a command line parser
        * @param argc argument count
        * @param argv argument vector
        */
        explicit cmd_line(int argc, char** argv);

        cmd_line() = default;
        cmd_line(cmd_line const&) = default;
        cmd_line(cmd_line&&) = default;
        cmd_line& operator = (cmd_line const&) = default;
        cmd_line& operator = (cmd_line&&) = default;

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @return parameter value
        */
        int get_int(std::string const& param);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @return parameter value
        */
        float get_float(std::string const& param);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @return parameter value
        */
        std::string get_string(std::string const& param);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @return parameter value
        * @note a bool is parsed as an integral value converted to false if 0; true otherwise
        */
        bool get_bool(std::string const& param);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @param return parameter value
        */
        std::size_t get_size_t(std::string const& param);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @param return parameter value
        */
        std::uint64_t get_uint64(std::string const& param);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @param v value. v is unchanged if the parameter does not exist
        * @return true if the parameter exists; false otherwise
        */
        bool get_int(std::string const& param, int& v);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @param v value. v is unchanged if the parameter does not exist
        * @return true if the parameter exists; false otherwise
        */
        bool get_float(std::string const& param, float& v);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @param v value. v is unchanged if the parameter does not exist
        * @return true if the parameter exists; false otherwise
        */
        bool get_string(std::string const& param, std::string& v);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @param v value. v is unchanged if the parameter does not exist
        * @return true if the parameter exists; false otherwise
        */
        bool get_bool(std::string const& param, bool& v);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @param v value. v is unchanged if the parameter does not exist
        * @return true if the parameter exists; false otherwise
        */
        bool get_size_t(std::string const& param, std::size_t& v);

        /**
        * @brief query a parameter value
        * @param parameter the parameter
        * @param v value. v is unchanged if the parameter does not exist
        * @return true if the parameter exists; false otherwise
        */
        bool get_uint64(std::string const& param, std::uint64_t& v);
    };

} // namespace

#endif // GU_CMD_LINE_HPP
