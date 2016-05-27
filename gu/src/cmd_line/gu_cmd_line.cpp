
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


#include <cmd_line/gu_cmd_line.hpp>
#include <sstream>
#include <algorithm>
#include <cassert>

using string_list = std::vector<std::string>;

string_list tokenize(std::string const& cmdline, char separator)
{
    std::stringstream ss(cmdline);
    string_list tokens;
    std::string tok = "";
    while (std::getline(ss, tok, separator))
    {
        tokens.push_back(tok);
    }
    return tokens;
}

gu::cmd_param_list parse(std::string cmdline)
{
    auto error = std::runtime_error{ "invalid command line" };

    // remove spaces at the begining
    cmdline.erase(std::begin(cmdline),
        std::find_if(std::begin(cmdline), std::end(cmdline), [](char a)
    {
        return a != ' ';
    }));

    // remove duplicate spaces
    cmdline.erase(std::unique(std::begin(cmdline), std::end(cmdline), [](char a, char b)
    {
        return a == ' ' && b == ' ';
    }), std::end(cmdline));

    auto sl = tokenize(cmdline, ' ');
    if ((sl.size() & 1) == 1) // pairs are expected
    {
        throw error;
    }

    auto pl = gu::cmd_param_list(sl.size() / 2);
    for (size_t i = 0; i < pl.size(); ++i)
    {
        pl[i].first = sl[i * 2];
        assert(!pl[i].first.empty()); // can't be because of the tokenizer
        if (pl[i].first[0] != '-')
        {
            throw error;
        }
        pl[i].first.erase(0, 1); // remove the '-'

        pl[i].second = sl[i * 2 + 1];
    }

    return pl;
}

gu::cmd_param_list parse(int argc, char** argv)
{
    std::string cmdline = "";
    for (int i = 1; i < argc; ++i)
    {
        cmdline += std::string{ " " } +argv[i];
    }

    return parse(cmdline);
}

std::string find(std::string const& x, gu::cmd_param_list const& pl)
{
    auto iter = std::find_if(std::begin(pl), std::end(pl), [x](gu::cmd_param const& p)
    {
        return p.first == x;
    });

    return iter == std::end(pl) ? "" : iter->second;
}

gu::cmd_line::cmd_line(std::string const& cmdline)
{
    pl = parse(cmdline);
}

gu::cmd_line::cmd_line(int argc, char** argv)
{
    pl = parse(argc, argv);
}

bool gu::cmd_line::get_int(std::string const& param, int& v)
{
    auto data = find(param, pl);
    if (data.empty())
    {
        return false;
    }
    v = std::stoi(data);
    return true;
}

bool gu::cmd_line::get_float(std::string const& param, float& v)
{
    auto data = find(param, pl);
    if (data.empty())
    {
        return false;
    }
    v = std::stof(data);
    return true;
}

bool gu::cmd_line::get_string(std::string const& param, std::string& v)
{
    auto data = find(param, pl);
    if (data.empty())
    {
        return false;
    }
    v = data;
    return true;
}

bool gu::cmd_line::get_bool(std::string const& param, bool& v)
{
    int vi = 0;
    if (!get_int(param, vi))
    {
        return false;
    }
    v = vi != 0;
    return true;
}

bool gu::cmd_line::get_size_t(std::string const& param, std::size_t& v)
{
    auto data = find(param, pl);
    if (data.empty())
    {
        return false;
    }
    v = static_cast<std::size_t>(std::stoull(data));
    return true;
}

bool gu::cmd_line::get_uint64(std::string const& param, std::uint64_t& v)
{
    auto data = find(param, pl);
    if (data.empty())
    {
        return false;
    }
    v = std::stoull(data);
    return true;
}

int gu::cmd_line::get_int(std::string const& param)
{
    int v = 0;
    if (!get_int(param, v))
    {
        throw std::runtime_error{ "invalid command line parameter" };
    }
    return v;
}

float gu::cmd_line::get_float(std::string const& param)
{
    float v = 0;
    if (!get_float(param, v))
    {
        throw std::runtime_error{ "invalid command line parameter" };
    }
    return v;
}

std::string gu::cmd_line::get_string(std::string const& param)
{
    std::string v = "";
    if (!get_string(param, v))
    {
        throw std::runtime_error{ "invalid command line parameter" };
    }
    return v;
}

bool gu::cmd_line::get_bool(std::string const& param)
{
    bool v = 0;
    if (!get_bool(param, v))
    {
        throw std::runtime_error{ "invalid command line parameter" };
    }
    return v;
}

std::size_t gu::cmd_line::get_size_t(std::string const& param)
{
    std::size_t v = 0;
    if (!get_size_t(param, v))
    {
        throw std::runtime_error{ "invalid command line parameter" };
    }
    return v;
}

std::uint64_t gu::cmd_line::get_uint64(std::string const& param)
{
    std::uint64_t v = 0;
    if (!get_uint64(param, v))
    {
        throw std::runtime_error{ "invalid command line parameter" };
    }
    return v;
}
